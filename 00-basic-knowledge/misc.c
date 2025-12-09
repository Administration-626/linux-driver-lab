/*
 * misc 设备驱动程序
*/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/miscdevice.h>

static int misc_open(struct inode *inode, struct file *file)
{
    printk("misc open\n");
    return 0;
}

static int misc_release(struct inode *inode, struct file *file)
{
    printk("misc release\n");
    return 0;
}

static ssize_t misc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    printk("misc read\n");
    return 0;
}

static ssize_t misc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk("misc write\n");
    return count;
}

static struct file_operations misc_fops = {
    .owner = THIS_MODULE,
    .open = misc_open,
    .release = misc_release,
    .read = misc_read,
    .write = misc_write,
};

static struct miscdevice misc_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "my_misc_device", // 和设备节点 /dev/my_misc_device 对应
    .fops = &misc_fops,
};

static int __init misc_init(void)
{
    int ret;

    ret = misc_register(&misc_dev);
    if (ret) {
        printk("failed to register misc device\n");
        return ret;
    }

    printk("misc device registered with minor %d\n", misc_dev.minor);
    return 0;
}

static void __exit misc_exit(void)
{
    misc_deregister(&misc_dev);
    printk("misc device unregistered\n");
}

module_init(misc_init);
module_exit(misc_exit);
MODULE_LICENSE("GPL");