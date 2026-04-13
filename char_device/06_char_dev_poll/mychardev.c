#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/poll.h>

#define MYDEV_IOC_MAGIC   'M'
#define MYDEV_CLEAR       _IO(MYDEV_IOC_MAGIC, 0)
#define MYDEV_GET_LEN     _IOR(MYDEV_IOC_MAGIC, 1, int)
#define MYDEV_SET_MODE    _IOW(MYDEV_IOC_MAGIC, 2, int)
#define MYDEV_GET_MODE    _IOR(MYDEV_IOC_MAGIC, 3, int)

/*

_IO：只有命令，不带数据
_IOR：用户从驱动读数据
_IOW：用户向驱动写数据
_IOWR：双向

应该也可以不用这些宏吧，直接定义约定
对，完全可以。
#define MY_GET_VAL  0x01
#define MY_SET_VAL  0x02
然后驱动里 switch (cmd) 直接匹配，能跑。

但代价是放弃了内核给你的免费保障：
这四个宏生成的编号里嵌了 size 字段，内核在进入驱动的 ioctl 之前，会用这个 size 做 access_ok() 检查——即验证用户空间指针是否合法、长度是否够。
你自己随便定义一个魔数，这个检查就没了，用户传个野指针进来，驱动如果直接 copy_from_user 而没有自己校验，就是潜在的安全漏洞或 oops。
另外魔数（type 字段）的设计本来就是为了命名空间隔离——防止用户误把 A 驱动的 cmd 发给 B 驱动然后碰巧匹配上。自己定义的话这个保障也没了。
所以结论是：能用，但属于"把安全网自己剪掉"。正式驱动不推荐。
*/



#define DEV_NAME "mychardev"

struct mydev {
	char buf[128];
	size_t len;
	struct mutex lock;
	struct cdev cdev;
	dev_t devno;
	int mode;
	wait_queue_head_t read_queue;
};

static struct mydev *gdev;
static struct class *my_class;

static int my_open(struct inode *inode, struct file *file) // 这里的 inode file 是什么东西
{/*
file 是每次 open 都会创建一个新的 file 结构体，代表这个打开的文件实例
inode 是代表这个设备文件的 inode 结构体 /dev/xxx ，所有打开这个设备文件的 file 都会指向同一个 inode
*/

	// todo 多次open 只有一个dev
	struct mydev *dev;

	// 
	dev = container_of(inode->i_cdev, struct mydev, cdev);
	file->private_data = dev;

	return 0;
}

// 直接用 cat 和 输出重定向来测试读写功能，看看能不能正确地读写数据
static ssize_t my_read(struct file *file, char __user *ubuf,
		       size_t count, loff_t *ppos)
{
	struct mydev *dev = file->private_data;
	ssize_t ret;

	printk("dev->len = %zu, *ppos = %lld\n", dev->len, *ppos);

	if (file->f_flags & O_NONBLOCK) {
		printk("nonblock read\n");
		if (dev->len == 0)
			return -EAGAIN;
	} else {
		if (dev->len == 0) {
			// 这里不能用另一个进程写入，因为每次都创建的不同的 dev
			if(wait_event_interruptible(dev->read_queue, dev->len > 0))
				return -ERESTARTSYS; // 如果在等待过程中被信号打断了，就返回这个错误
		}
	}

	if (*ppos >= dev->len) {
		ret = 0;
		goto out;
	}

	if (count > dev->len - *ppos)
		count = dev->len - *ppos;

	if (copy_to_user(ubuf, dev->buf + *ppos, count)) {
		ret = -EFAULT;
		goto out;
	}

	out:

	return ret;
}

static ssize_t my_write(struct file *file, const char __user *ubuf,
			size_t count, loff_t *ppos)
{
	struct mydev *dev = file->private_data;
	ssize_t ret;

	mutex_lock(&dev->lock);

	if (count > sizeof(dev->buf))
		count = sizeof(dev->buf);

	if (copy_from_user(dev->buf, ubuf, count)) {
		ret = -EFAULT;
		goto out;
	}

	dev->len = count;
	ret = count;

out:
	mutex_unlock(&dev->lock);

	wake_up_interruptible(&dev->read_queue);

	return ret;
}

static int my_release(struct inode *inode, struct file *file)
{
	return 0;
}

long my_ioctl (struct file *file, unsigned int cmd, unsigned long arg)
{
	struct mydev *dev = file->private_data;
	int val;

	switch (cmd) {
	case MYDEV_CLEAR:
		mutex_lock(&dev->lock);
		memset(dev->buf, 0, sizeof(dev->buf));
		dev->len = 0;
		mutex_unlock(&dev->lock);
		return 0;

	case MYDEV_GET_LEN:
		mutex_lock(&dev->lock);
		val = dev->len;
		mutex_unlock(&dev->lock);

		if (copy_to_user((int __user *)arg, &val, sizeof(val)))
			return -EFAULT;
		return 0;

	case MYDEV_SET_MODE:
		if (copy_from_user(&val, (int __user *)arg, sizeof(val)))
			return -EFAULT;

		if (val < 0 || val > 3)
			return -EINVAL;

		mutex_lock(&dev->lock);
		dev->mode = val;
		mutex_unlock(&dev->lock);
		return 0;

	case MYDEV_GET_MODE:
		mutex_lock(&dev->lock);
		val = dev->mode;
		mutex_unlock(&dev->lock);

		if (copy_to_user((int __user *)arg, &val, sizeof(val)))
			return -EFAULT;
		return 0;

	default:
		return -ENOTTY;
	}
}

__poll_t my_poll(struct file *file, struct poll_table_struct *wait)
{
	__poll_t mask = 0;

	struct mydev *dev = file->private_data;
	poll_wait(file, &dev->read_queue, wait);
	


	return mask;
}

static const struct file_operations my_fops = {
	.owner   = THIS_MODULE,
	.open    = my_open,
	.read    = my_read,
	.write   = my_write,
	.release = my_release,
	.unlocked_ioctl = my_ioctl,
	.poll = my_poll,
};

static int __init my_init(void)
{
	int ret;

	/*
	多个设备时，创建多个 mydev 结构体，每个设备一个

	for (i = 0; i < MYDEV_COUNT; i++) {
		struct mydev *dev = &devs[i];

		dev->devno = MKDEV(MAJOR(base_devno), MINOR(base_devno) + i);

		mutex_init(&dev->lock);
		init_waitqueue_head(&dev->read_queue);
		dev->len = 0;

		cdev_init(&dev->cdev, &my_fops);
		dev->cdev.owner = THIS_MODULE;

		ret = cdev_add(&dev->cdev, dev->devno, 1);
		if (ret)
			goto err;
	}
	*/
	gdev = kzalloc(sizeof(*gdev), GFP_KERNEL); // 
    /*
	为什么不直接定义一个全局变量，而是动态分配内存呢

    在用户态程序里
    如果一个程序运行多次：每次运行是一个独立进程 每个进程通常有自己独立的虚拟地址空间
    所以“全局变量”虽然代码里只有一份定义，但每个进程各有一份副本
    也就是说：static int x; 程序开 3 次，不是 3 个进程共用一个 x，而是每个进程各有自己的 x。
    
	在内核模块里
    驱动模块只加载一次。
    所以：static struct mydev gdev; 这真的是整个驱动全局只有这一份对象。
    不是“每次 open 一份”，也不是“每个进程一份”
    */

    /* 核心问题就是内核里只有这一份对象 */
	if (!gdev)
		return -ENOMEM;

	mutex_init(&gdev->lock);

	ret = alloc_chrdev_region(&gdev->devno, 0, 1, DEV_NAME);
	if (ret)
		goto err_alloc_region;

	cdev_init(&gdev->cdev, &my_fops);
	gdev->cdev.owner = THIS_MODULE;

	ret = cdev_add(&gdev->cdev, gdev->devno, 1);
	if (ret)
		goto err_cdev_add;

	my_class = class_create(THIS_MODULE, DEV_NAME);
	if (IS_ERR(my_class)) {
		ret = PTR_ERR(my_class);
		goto err_class_create;
	}

	if (IS_ERR(device_create(my_class, NULL, gdev->devno, NULL, DEV_NAME))) {
		ret = -EINVAL;
		goto err_device_create;
	}

	pr_info("mychardev init ok: major=%d minor=%d\n",
		MAJOR(gdev->devno), MINOR(gdev->devno));

	init_waitqueue_head(&gdev->read_queue); // 初始化等待队列头
		
	return 0;

err_device_create:
	class_destroy(my_class);
err_class_create:
	cdev_del(&gdev->cdev);
err_cdev_add:
	unregister_chrdev_region(gdev->devno, 1);
err_alloc_region:
	kfree(gdev);
	return ret;
}

static void __exit my_exit(void)
{
	device_destroy(my_class, gdev->devno);
	class_destroy(my_class);
	cdev_del(&gdev->cdev);
	unregister_chrdev_region(gdev->devno, 1);
	kfree(gdev);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ChatGPT");
MODULE_DESCRIPTION("simple char device with private_data");