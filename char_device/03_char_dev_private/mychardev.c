#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>

#define DEV_NAME "mychardev"

struct mydev {
	char buf[128];
	size_t len;
	struct mutex lock;
	struct cdev cdev;
	dev_t devno;
};

static struct mydev *gdev;
static struct class *my_class;

static int my_open(struct inode *inode, struct file *file) // 这里的 inode file 是什么东西
{/*
file 是每次 open 都会创建一个新的 file 结构体，代表这个打开的文件实例
inode 是代表这个设备文件的 inode 结构体 /dev/xxx ，所有打开这个设备文件的 file 都会指向同一个 inode
*/


	struct mydev *dev;

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

	mutex_lock(&dev->lock);

	if (*ppos >= dev->len) { // 偏移量大于数据长度，说明已经读完了
		ret = 0;
		goto out;
	}

	if (count > dev->len - *ppos)
		count = dev->len - *ppos;

	if (copy_to_user(ubuf, dev->buf + *ppos, count)) {
		ret = -EFAULT;
		goto out;
	}

	*ppos += count;
	ret = count;

out:
	mutex_unlock(&dev->lock);
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
	return ret;
}

static int my_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations my_fops = {
	.owner   = THIS_MODULE,
	.open    = my_open,
	.read    = my_read,
	.write   = my_write,
	.release = my_release,
};

static int __init my_init(void)
{
	int ret;

	gdev = kzalloc(sizeof(*gdev), GFP_KERNEL); // 为什么不直接定义一个全局变量，而是动态分配内存呢
    /*
    在用户态程序里

    如果一个程序运行多次：

    每次运行是一个独立进程
    每个进程通常有自己独立的虚拟地址空间
    所以“全局变量”虽然代码里只有一份定义，但每个进程各有一份副本

    也就是说：

    static int x;

    程序开 3 次，不是 3 个进程共用一个 x，
    而是每个进程各有自己的 x。

    在内核模块里

    驱动模块只加载一次。
    所以：

    static struct mydev gdev;

    这真的是整个驱动全局只有这一份对象。

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