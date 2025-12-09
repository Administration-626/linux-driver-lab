#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

extern int num;
extern int add(int a, int b);

static int __init hello_init(void)
{
    printk("hello\n");
    int result = add(1, num);
    printk("add(1, num) = %d\n", result);
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
MODULE_DESCRIPTION("import symbol example");
MODULE_VERSION("V1.0");