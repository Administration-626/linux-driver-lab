1. 内核模块符号的导出与导入
   - 使用 `EXPORT_SYMBOL` 和 `EXPORT_SYMBOL_GPL` 宏导出符号。
   - 其他模块通过 `extern` 关键字引用导出的符号。
   - 注意加载和卸载模块的顺序，确保依赖关系正确。

2. 打印调用栈 dump_stack()

3. 杂项设备驱动