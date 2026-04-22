#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

struct mydev_priv {
    struct gpio_desc *led_gpio; // LED GPIO 描述符
};

/*
现在是已经match_table里匹配上了， pdev就是 device是传入参数
*/
static int mydev_probe(struct platform_device *pdev)
{
    struct mydev_priv *priv;

    priv = devm_kzalloc(
        &pdev->dev,
        sizeof(*priv),
        GFP_KERNEL); // 托管内存分配，自动释放，托管到pdev->dev设备上，卸载驱动时会自动释放
    if (!priv)
        return -ENOMEM;

    //  GPIO descriptor API
    priv->led_gpio = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(priv->led_gpio))
        return PTR_ERR(priv->led_gpio);

    platform_set_drvdata(
        pdev,
        priv); // 将私有数据结构指针存储在平台设备的驱动数据中，以便在其他函数中通过
               // platform_get_drvdata() 获取

    dev_info(&pdev->dev, "probe success\n"); // 自动添加设备名称
    return 0;
}

static int mydev_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "remove called\n");

    return 0;
}

static const struct of_device_id mydev_of_match[] = {
    { .compatible =
          "vendor,myled" }, // 这是一个字符串，在设备树中用来匹配设备的
                            // compatible 属性的值
    {}
};
MODULE_DEVICE_TABLE(of, mydev_of_match);

static struct platform_driver mydev_driver = {
    .probe = mydev_probe,
    .remove = mydev_remove,
    .driver = {
        .name = "myled_platform_driver",
        .of_match_table = mydev_of_match,
    },
};

module_platform_driver(
    mydev_driver); // 这是一个宏，定义了模块的初始化和退出函数，分别调用
                   // platform_driver_register() 和 platform_driver_unregister()
                   // 来注册和注销平台驱动

MODULE_LICENSE("GPL");
MODULE_AUTHOR("test");
MODULE_DESCRIPTION("simple platform driver");
