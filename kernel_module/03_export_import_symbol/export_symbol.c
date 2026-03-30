#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/ptrace.h>

int num = 10;
EXPORT_SYMBOL(num);

int add(int a, int b);
int add(int a, int b)
{
    return a + b;
}
EXPORT_SYMBOL(add);

static int __init hello_init(void)
{
    printk("hello\n");
    dump_stack();
    return 0;
}

static void __exit hello_exit(void)
{
    printk("goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("tan");
MODULE_DESCRIPTION("export symbol example");
MODULE_VERSION("V1.0");