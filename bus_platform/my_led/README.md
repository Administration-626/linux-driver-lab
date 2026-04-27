# 一、platform 驱动和字符设备驱动的关系
Platform 驱动负责硬件的"发现和接入"，字符设备驱动负责提供"用户空间接口"，两者通常是上下层协作关系，共同完成一个完整设备驱动的构建

| 层级 | 类型                  | 核心职责                                                                  |
| -- | ------------------- | --------------------------------------------------------------------- |
| 底层 | **Platform Driver** | 匹配设备树/DTS 描述的硬件节点，获取资源（内存映射、IRQ、时钟等），完成硬件初始化                          |
| 上层 | **字符设备驱动**          | 注册字符设备（`cdev`），实现 `file_operations`（open/read/write/ioctl），提供用户空间访问接口 |

**为什么需要分层？**

解耦：Platform 机制解决"硬件在哪里、如何配置"的问题；字符设备解决"用户如何访问"的问题

复用：同一硬件可能暴露多个接口（如一个 UART 可以是字符设备，也可通过 tty 层），Platform 部分可保持不变

标准化：Platform 总线统一处理设备生命周期（电源管理、热插拔），字符设备层专注 I/O 语义

## 这么看在platform驱动看不到和字符驱动的关联
这里 platform driver 和 char dev 没有通过内核对象建立强绑定关系，只是代码上共享了同一份全局资源 led_res，所以在 platform 驱动的 probe 函数中获取了硬件资源并保存到全局变量后，字符设备驱动的 write 函数就可以直接使用这个全局变量来操作硬件寄存器,实际使用时当然可以灵活变通
```
platform driver 匹配设备树
        ↓
led_probe()
        ↓
of_iomap() 获取寄存器虚拟地址
        ↓
保存到全局变量 led_res.va_DR / led_res.va_DDR
        ↓
注册 char dev
        ↓
用户 write /dev/led_test
        ↓
led_chr_dev_write()
        ↓
使用 led_res.va_DR 操作硬件寄存器

```

```
在 rk3588-orangepi-5-plus.dts添加一个顶层节点
		myled {
			compatible = "vendor,myled";
			led-gpios = <&gpio1 RK_PA4 GPIO_ACTIVE_HIGH>;
			status = "okay";
		};


编译了 dtbs 之后，把设备树文件复制到 /boot/dtbs/ 目录下，并重启系统，/sys/firmware/devicetree/base/gpio-leds/myled@0 就会出现，说明设备树节点被正确解析了
```

## 要区分三个名字：

driver.name = "myled_platform_driver"
作用：平台驱动名
体现：/sys/bus/platform/drivers/myled_platform_driver/

of_match_table = { .compatible = "vendor,myled" }
作用：设备树匹配依据
体现：不是直接作为 sysfs 目录名，而是用于匹配设备树里的 compatible

device_create(..., "chrdev_myled")
作用：字符设备名
体现：/dev/chrdev_myled 和 /sys/class/chrdev_myled/...