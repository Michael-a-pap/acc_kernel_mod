#include <linux/module.h>
#include <linux/init.h> 


static int __init init_acc_mod(void)
{ 
    pr_info("Hello world!\n"); 
    return 0;
} 

static void __exit exit_acc_mod(void) 
{ 
    pr_info("Goodbye world!\n"); 
} 

module_init(init_acc_mod);
module_exit(exit_acc_mod);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael A. Papachatzakis");
MODULE_DESCRIPTION("Kernel module for the FPGA accelerator");
