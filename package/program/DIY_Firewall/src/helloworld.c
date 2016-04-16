#include <linux/init.h>  
#include <linux/module.h>  
#include <linux/kernel.h>  


static int __init hello_init(void)  
{  
    printk("hello world.\n"); 
	printk("hello world.\n");
	printk("hello world.\n");
	printk("hello world.\n");
    return 0;  
}  


static void __exit hello_exit(void)  
{  
    printk("exit hello world.\n");  
}  

module_init(hello_init);  
module_exit(hello_exit);  
MODULE_LICENSE("GPL");  
MODULE_AUTHOR("hxw");  


