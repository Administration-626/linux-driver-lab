# 依赖的设备树节点

- `myled`：用于匹配设备树中的 LED 设备节点
- `mykey`：用于匹配设备树中的按键设备节点
- `mytrigger`：用于匹配设备树中的触发器设备节点
# 设备树节点示例

```dts
	my_key_irq_led {
		compatible = "vendor,mykey-irq-led";
		led-gpios = <&gpio1 RK_PA4 GPIO_ACTIVE_HIGH>;
		key-gpios = <&gpio1 RK_PA7 GPIO_ACTIVE_LOW>;
		interrupt-parent = <&gpio1>;
		interrupts = <RK_PA7 IRQ_TYPE_EDGE_RISING>;
		status = "okay";
	};


	mytrigger {
		compatible = "vendor,mytrigger";
		trigger-gpios = <&gpio1 RK_PB0 GPIO_ACTIVE_HIGH>;
		status = "okay";
	};
```

```shell
# -L 选项表示跟随符号链接，-type d 表示只查找目录，-name "*my_key*" 表示匹配名称包含 "my_key" 的节点
find -L /proc/device-tree -type d -name "*my_key*"
find -L /proc/device-tree -type d -name "*mytrigger*"
```


结论：可以“匹配两个设备树节点”，但要先分清是哪一种需求。

你这里 [myplatform_key_irq_led.c](/home/tan/linux-driver-lab/bus_platform/03_platform_key_irq_led/myplatform_key_irq_led.c:169) 现在的写法，只表示这个驱动能绑定 `compatible = "vendor,myled"` 的节点。  
**`of_match_table` 只是定义“这个驱动支持哪些节点类型”**，不是让一次 `probe()` 同时收到两个节点。

**更合理的两种理解：**

1. **一个驱动分别绑定两个设备树节点**
2. **一个驱动在一次 `probe()` 里同时使用两个节点的数据**

这两种做法完全不同。

---

**一、如果你的意思是：一个驱动支持两个节点类型**

可以直接在 `of_match_table` 里写两个 `compatible`：

```c
static const struct of_device_id mydev_of_match[] = {
    { .compatible = "vendor,myled" },
    { .compatible = "vendor,mykey" },
    { }
};
```

或者设备树里放两个同类节点，都写同一个 `compatible`：

```dts
myled0 {
    compatible = "vendor,myled";
    led-gpios = <...>;
};

myled1 {
    compatible = "vendor,myled";
    led-gpios = <...>;
};
```

**效果：**
- 每个匹配到的节点，内核都会创建一个 `platform_device`
- 驱动的 `probe()` 会被调用多次
- 每次 `probe()` 的 `pdev->dev.of_node` 都只对应当前这一个节点

**关键点：**
你不能理解成“一个驱动一次匹配到了两个节点”。  
准确说法是：**同一个驱动实例逻辑，可以分别绑定多个节点，每个节点各进一次 `probe()`。**

---

**二、如果你的意思是：这个驱动既想拿 LED 节点，又想拿 KEY 节点**

那**不能只靠 `of_match_table`**。

原因是：
- `of_match_table` 只决定“当前 `pdev` 对应哪个节点”
- 进入 `probe()` 后，`pdev->dev` 只代表当前这个节点
- 你现在代码里写的是：

```c
priv->led_gpio = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
```

这要求**当前被绑定的节点**里必须有 `led-gpios`。  
如果你把 `vendor,mykey` 也加进匹配表，但 `mykey` 节点里没有 `led-gpios`，这里就会失败。

---

**三、如果一个功能确实依赖 LED 和 KEY 两类资源，正确建模通常有两种**

### 方案 1：合成一个设备树节点
这是更推荐的方式。把这个驱动需要的资源都放进一个节点里：

```dts
my_key_irq_led {
    compatible = "vendor,mykey-irq-led";
    led-gpios = <&gpio1 RK_PA4 GPIO_ACTIVE_HIGH>;
    key-gpios = <&gpio1 RK_PA7 GPIO_ACTIVE_LOW>;
    interrupt-parent = <&gpio1>;
    interrupts = <RK_PA7 IRQ_TYPE_EDGE_RISING>;
    status = "okay";
};
```

驱动只匹配这一个节点：

```c
static const struct of_device_id mydev_of_match[] = {
    { .compatible = "vendor,mykey-irq-led" },
    { }
};
```

然后在一个 `probe()` 里同时取 LED、KEY、IRQ 资源。  
**这是设备模型最清晰的做法。**

### 方案 2：当前节点通过 phandle 引用另一个节点
如果你确实要保留两个独立节点，可以让一个主节点引用另一个节点：

```dts
myctrl {
    compatible = "vendor,myctrl";
    led-handle = <&myled0>;
    key-handle = <&mykey0>;
};

myled0: myled {
    compatible = "vendor,myled";
    led-gpios = <...>;
};

mykey0: mykey {
    compatible = "vendor,mykey";
    key-gpios = <...>;
};
```

然后在驱动里通过 `of_parse_phandle()` 找到另一个节点。  
但这比“单一功能节点”更复杂，不适合你这个入门实验场景。

---

**四、结合你这个文件，最关键的问题**

你这个文件名叫 `key_irq_led`，但当前实现实际上只绑定了 `vendor,myled`，并且只拿了 `led-gpios`。  
所以：

- **如果你只是想让同一个驱动支持 `myled` 和 `mykey` 两种节点**  
  可以加两个 `compatible`，但还要在 `probe()` 里根据节点类型分别处理，否则会因为资源名不同而失败。

- **如果你想做“按键中断控制 LED”的一个完整设备**  
  最合理的不是“匹配两个独立节点”，而是**新建一个组合节点**，让这个驱动只匹配这个组合节点。

---

**五、如果你坚持一个驱动支持两种 `compatible`，最少要这样区分**

```c
static const struct of_device_id mydev_of_match[] = {
    { .compatible = "vendor,myled", .data = (void *)1 },
    { .compatible = "vendor,mykey", .data = (void *)2 },
    { }
};
```

在 `probe()` 里判断当前匹配到的是哪一种：

```c
const struct of_device_id *match;

match = of_match_device(mydev_of_match, &pdev->dev);
if (!match)
    return -ENODEV;

if ((uintptr_t)match->data == 1) {
    // 处理 led 节点
} else if ((uintptr_t)match->data == 2) {
    // 处理 key 节点
}
```

但这仍然是“**分别绑定两个节点**”，不是“一个 `probe()` 同时拿两个节点”。

---

**一句话总结：**

- **想让驱动能绑定两个节点类型**：在 `of_match_table` 里加两项
- **想让一个功能同时使用 LED 和 KEY 资源**：不要靠“匹配两个节点”，应改成**一个组合设备树节点**，这是更对的建模方式。
