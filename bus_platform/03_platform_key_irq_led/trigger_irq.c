#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

struct mydev_priv {
    struct gpio_desc *led_gpio; // LED GPIO 描述符
    dev_t dev_num; // 设备号
    struct cdev led_chr_dev; // 字符设备结构体
    struct class *led_class; // 设备类
    struct device *led_dev; // 设备节点
};

int mydev_open(struct inode *inode, struct file *file)
{
    struct mydev_priv *priv;

    priv = container_of(inode->i_cdev, struct mydev_priv, led_chr_dev);
    file->private_data = priv;

    pr_info("chrdev_mytrigger: open called\n");
    return 0;
}

ssize_t mydev_read(
    struct file *file,
    char __user *buf,
    size_t count,
    loff_t *ppos)
{
    pr_info("chrdev_mytrigger: read called\n");
    return 0;
}

ssize_t mydev_write(
    struct file *file,
    const char __user *buf,
    size_t count,
    loff_t *ppos)
{
    struct mydev_priv *priv;
    char write_data;
    int not_copied;

    pr_info("chrdev_mytrigger: write called\n");

    if (count < sizeof(write_data))
        return -EINVAL;

    priv = file->private_data;
    if (!priv)
        return -ENODEV;

    not_copied = copy_from_user(&write_data, buf, sizeof(write_data));
    if (not_copied)
        return -EFAULT;

    if (write_data == '1') {
        gpiod_set_value(priv->led_gpio, 1); // 点亮 LED
    } else if (write_data == '0') {
        gpiod_set_value(priv->led_gpio, 0); // 熄灭 LED
    }
    return 0;
}

struct file_operations mydev_fops = {
    .owner = THIS_MODULE,
    .open = mydev_open,
    .read = mydev_read,
    .write = mydev_write,
};

/*
现在是已经match_table里匹配上了， pdev就是 device是传入参数
*/
static int mydev_probe(struct platform_device *pdev)
{
    struct mydev_priv *priv;
    int ret;

    dev_info(&pdev->dev, "entry probe\n");
    priv = devm_kzalloc(
        &pdev->dev,
        sizeof(*priv),
        GFP_KERNEL); // 托管内存分配，自动释放，托管到pdev->dev设备上，卸载驱动时会自动释放
    if (!priv)
        return -ENOMEM;

    //  GPIO descriptor API
    priv->led_gpio = devm_gpiod_get(&pdev->dev, "trigger", GPIOD_OUT_LOW);
    if (IS_ERR(priv->led_gpio))
        return dev_err_probe(
            &pdev->dev, PTR_ERR(priv->led_gpio), "failed to get led gpio\n");

    platform_set_drvdata(
        pdev,
        priv); // 将私有数据结构指针存储在平台设备的驱动数据中，以便在其他函数中通过
               // platform_get_drvdata() 获取

    dev_info(&pdev->dev, "probe success\n"); // 自动添加设备名称

    /* 字符设备驱动 */
    /* 1. 分配字符设备号，在/proc/devices中显示chrdev_mytrigger */
    dev_info(&pdev->dev, "register chr dev\n");
    ret = alloc_chrdev_region(&priv->dev_num, 0, 1, "chrdev_mytrigger");
    if (ret)
        goto err_alloc_region;

    /* 2. 初始化字符设备，将字符设备与文件操作关联 */
    cdev_init(&priv->led_chr_dev, &mydev_fops);
    priv->led_chr_dev.owner = THIS_MODULE;

    /* 3. 添加字符设备，将字符设备和设备号关联 */
    ret = cdev_add(&priv->led_chr_dev, priv->dev_num, 1);
    if (ret)
        goto err_cdev_add;

    /* 4. 创建设备类，创建 /sys/class/chrdev_mytrigger 目录 */
    priv->led_class = class_create(THIS_MODULE, "chrdev_mytrigger");
    if (IS_ERR(priv->led_class)) {
        ret = PTR_ERR(priv->led_class);
        goto err_class_create;
    }

    /* 5. 创建设备，创建 /dev/chrdev_mytrigger
     * 设备节点和/sys/class/chrdev_mytrigger 的具体某个设备 */
    priv->led_dev = device_create(
        priv->led_class,
        NULL,
        priv->dev_num,
        NULL,
        "chrdev_mytrigger"); // 设备名称
    if (IS_ERR(priv->led_dev)) {
        ret = PTR_ERR(priv->led_dev);
        goto err_device_create;
    }

    return 0;

err_device_create:
    class_destroy(priv->led_class);
err_class_create:
    cdev_del(&priv->led_chr_dev);
err_cdev_add:
    unregister_chrdev_region(priv->dev_num, 1);
err_alloc_region:
    return ret;
}

static int mydev_remove(struct platform_device *pdev)
{
    struct mydev_priv *priv = platform_get_drvdata(pdev);
    device_destroy(priv->led_class, priv->dev_num);
    class_destroy(priv->led_class);
    cdev_del(&priv->led_chr_dev);
    unregister_chrdev_region(priv->dev_num, 1);
    dev_info(&pdev->dev, "remove called\n");

    return 0;
}

static const struct of_device_id mydev_of_match[] = {
    { .compatible =
          "vendor,mytrigger" }, // 这是一个字符串，在设备树中用来匹配设备的
                              // compatible 属性的值
    {}
};
MODULE_DEVICE_TABLE(of, mydev_of_match);

static struct platform_driver mydev_driver = {
    .probe = mydev_probe,
    .remove = mydev_remove,
    .driver = {
        .name = "myled_platform_trigger", //  在/sys/bus/platform/drivers/
        .of_match_table = mydev_of_match,
    },
};

static int __init mydev_init(void)
{
    pr_info("myplatform_gpio: module init, register platform driver, trigger\n");
    return platform_driver_register(&mydev_driver);
}

static void __exit mydev_exit(void)
{
    pr_info("myplatform_gpio: module exit, unregister platform driver\n");
    platform_driver_unregister(&mydev_driver);
}

module_init(mydev_init);
module_exit(mydev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("simple platform driver");
