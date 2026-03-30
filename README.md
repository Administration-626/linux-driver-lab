# linux-driver-lab

面向 `Linux 驱动 / 嵌入式 Linux` 学习的个人训练仓库。

这个仓库不是作品集，主要目标是把知识点拆成一条能持续推进的主线，避免学习过程中越学越散。

## 学习主线

当前建议按下面的顺序推进：

1. `thread_practice/`
   先把 `mutex / semaphore / condvar / ring buffer / thread pool / memory pool` 练扎实。
2. `kernel_module/`
   再进入内核模块基础，建立对驱动运行环境的感觉。
3. `char_device/`
   主攻字符设备，把 `read/write/ioctl/blocking/poll` 这些基础机制串起来。
4. `driver_mechanism/`
   继续补 `wait queue / kfifo / timer / workqueue` 等驱动通用机制。
5. `bus_platform/`
   再进入 `platform driver / device tree / UART / I2C / SPI`。
6. `zynq_topic/`
   最后补 `ZYNQ / AMP / A核-R核 / 裸机协作`。

## 当前目录说明

### `thread_practice/`

并发与同步练习区。

- `02_semaphore_prodcons.c`
  使用 `semaphore + mutex` 实现生产者消费者。
- `03_condvar_prodcons.c`
  使用 `condvar + mutex` 实现条件等待和唤醒。
- `04_lockfree_queue.c`
  当前更接近“环形队列 + 条件等待”的实验，后面可以再演进成真正的无锁队列。

### `kernel_module/`

内核模块基础练习区。

- `03_export_import_symbol/`
  目前已经有符号导出和导入实验。

### `char_device/`

字符设备练习主线。

- `01_misc_char_basic/`
  从 `misc device` 开始，先打通用户态和内核态交互。
- `02_standard_char_device/`
  后续放标准字符设备注册与更完整的驱动实现。

### `notes/`

放学习总结、知识点记录和复盘笔记。

### `roadmap/`

放阶段计划和下一步任务，不放代码。

## 当前建议

下一阶段先只做这三件事：

1. 完善 `thread_practice/`
   补一个 `01_mutex_counter.c`，并把现有实验各自写清楚“解决什么问题”。
2. 跑通 `kernel_module/03_export_import_symbol/`
   熟悉模块编译、加载、卸载、日志观察。
3. 开始写 `char_device/02_standard_char_device/my_dev.c`
   从标准字符设备注册开始，不急着一口气上 `ioctl/poll`。

## 学习原则

- 每个主题都保留最小可运行 demo。
- 每个主题都写一句“这个知识点解决什么问题”。
- 先学会跑通，再学会解释，最后再做增强版。
