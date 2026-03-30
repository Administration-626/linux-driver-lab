# 当前学习计划

## 第一阶段：thread_practice

目标：把并发基础练明白，而不是只会调用 API。

### 已有内容

- `02_semaphore_prodcons.c`
- `03_condvar_prodcons.c`
- `04_lockfree_queue.c`

### 下一步

1. 新增 `01_mutex_counter.c`
2. 把 `04_lockfree_queue.c` 重新定义成“环形队列实验”
3. 增加 `05_thread_pool.c`
4. 增加 `06_memory_pool.c`

## 第二阶段：kernel_module

目标：理解内核模块的基本编译、加载、卸载和模块间依赖。

### 已有内容

- `03_export_import_symbol/`

### 下一步

1. 增加 `01_hello/`
2. 增加 `02_module_param/`
3. 增加 `04_misc_device/` 或继续复用 `char_device/01_misc_char_basic/`

## 第三阶段：char_device

目标：建立字符设备的完整主线。

### 已有内容

- `01_misc_char_basic/`
- `02_standard_char_device/`

### 下一步

1. 完成标准字符设备注册
2. 补 `open/read/write/release`
3. 再加 `ioctl`
4. 最后再补 `blocking read + poll`
