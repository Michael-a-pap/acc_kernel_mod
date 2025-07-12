#include <linux/module.h>
#include <linux/init.h> 
#include <linux/fs.h>

static int major;

static ssize_t acc_read(struct file *f, char __user *u, size_t l, loff_t *o)
{
	pr_info("FPGA accelerator - Read is called");
	return 0;
}

static struct file_operations fops = {
	.read = acc_read
};

static int __init init_acc_mod(void)
{
	major = register_chrdev(0, "fpga_accelerator", &fops); //0 is for dynamic allocation
	if (major < 0) {
		pr_info("Error registering accelerator device");
		return major;
	}
	pr_info("FPGA accelerator - Major Device Number: %d\n", major); 
	return 0;
} 

static void __exit exit_acc_mod(void) 
{ 
	unregister_chrdev(major, "fpga_accelerator");
	pr_info("FPGA accelerator - Unregistering"); 
} 

module_init(init_acc_mod);
module_exit(exit_acc_mod);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael A. Papachatzakis");
MODULE_DESCRIPTION("Kernel module for the FPGA accelerator");
