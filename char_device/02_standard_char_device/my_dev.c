/*
 * 字符设备驱动
明白字符设备是什么，用户态怎么通过 /dev/xxx 访问它
能写出基本的 open / read / write / release
理解 copy_to_user / copy_from_user
会做设备注册与注销
理解 cdev、设备号、class_create、device_create
能写最基础的 ioctl
知道阻塞/非阻塞读写是什么
知道 poll、等待队列是干什么的
能自己编译、加载、卸载、看 dmesg 排错
*/

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>

static struct class *cls;
static struct device *my_dev_device;
static struct cdev mydev;
static dev_t char_devid;

static char my_dev_data[100] = "Hello from my_dev!";

static int my_dev_open(struct inode *inode, struct file *file)
{
    printk("my_dev open\n");
    return 0;
}

static int my_dev_release(struct inode *inode, struct file *filp)
{
    printk("my_dev release\n");
    return 0;
}

static ssize_t my_dev_read(
    struct file *filp,
    char __user *ubuf,
    size_t count,
    loff_t *offp)
{
    int len;

    printk("my_dev read\n");

    len = strlen(my_dev_data) + 1;
    if (count < len)
        return -EINVAL;

    if (copy_to_user(ubuf, my_dev_data, len))
        return -EFAULT;

    return len;
}

static ssize_t my_dev_write(
    struct file *filp,
    const char __user *ubuf,
    size_t count,
    loff_t *offp)
{
    printk("my_dev write\n");

    if (copy_from_user(my_dev_data, ubuf, count))
        return -EFAULT;

    return count;
}

static struct file_operations my_dev_fops = {
    .owner = THIS_MODULE,
    .open = my_dev_open,
    .release = my_dev_release,
    .read = my_dev_read,
    .write = my_dev_write,
};

static int __init my_dev_init(void)
{
    dev_t dev_id;
    int ret;

    printk("my_dev init\n");

    ret = alloc_chrdev_region(&dev_id, 0, 1, "my_dev");
    if (ret) {
        printk("alloc_chrdev_region failed: %d\n", ret);
        return ret;
    }
    char_devid = dev_id;

    printk("major: %d\n", MAJOR(dev_id));
    printk("minor: %d\n", MINOR(dev_id));

    mydev.owner = THIS_MODULE;

    cdev_init(&mydev, &my_dev_fops);
    ret = cdev_add(
        &mydev, dev_id, 1); // 添加字符设备，参数：cdev结构体，设备号，设备数量
    if (ret) {
        printk("cdev_add failed: %d\n", ret);
        unregister_chrdev_region(dev_id, 1);
        return ret;
    }

    cls = class_create(THIS_MODULE, "my_dev");
    if (IS_ERR(cls)) {
        ret = PTR_ERR(cls);
        printk("class_create failed: %d\n", ret);
        cdev_del(&mydev);
        unregister_chrdev_region(dev_id, 1);
        return ret;
    }

    my_dev_device = device_create(cls, NULL, dev_id, NULL, "my_dev");
    if (IS_ERR(my_dev_device)) {
        ret = PTR_ERR(my_dev_device);
        printk("device_create failed: %d\n", ret);
        class_destroy(cls);
        cdev_del(&mydev);
        unregister_chrdev_region(dev_id, 1);
        return ret;
    }

    return 0;
}

static void __exit my_dev_exit(void)
{
    printk("my_dev exit\n");
    device_destroy(cls, char_devid); // 注意先销毁设备再销毁类
    cdev_del(&mydev);
    class_destroy(cls);
    unregister_chrdev_region(char_devid, 1); // 有注册就有销毁嘛
}

module_init(my_dev_init);
module_exit(my_dev_exit);
MODULE_LICENSE("GPL");
