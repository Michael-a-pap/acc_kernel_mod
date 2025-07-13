#include <linux/module.h>
#include <linux/init.h> 
#include <linux/fs.h>

static int major;
static char txt_buff[64];


static ssize_t acc_read(struct file *file_p, char __user *user_buff, size_t len, loff_t *off)
{
	int not_copied, delta, to_copy = (len + *off) < sizeof(txt_buff) ? len : (sizeof(txt_buff) - *off);
	pr_info("FPGA accelerator - Read is called");

	if (*off >= sizeof(txt_buff))
		return 0;
	
	not_copied = copy_to_user(user_buff, &txt_buff[*off], to_copy);
	delta = to_copy - not_copied;

	if (not_copied)
		pr_warn("FPGA accelerator - Only %d bytes where copied\n", delta);

	*off += delta;
	return delta;
}

static ssize_t acc_write(struct file *file_p, const char __user *user_buff, size_t len, loff_t *off)
{
	int not_copied, delta, to_copy = (len + *off) < sizeof(txt_buff) ? len : (sizeof(txt_buff) - *off);
	pr_info("FPGA accelerator - Write is called");

	if (*off >= sizeof(txt_buff))
		return 0;
	
	not_copied = copy_from_user(&txt_buff[*off],user_buff, to_copy);
	delta = to_copy - not_copied;

	if (not_copied)
		pr_warn("FPGA accelerator - Only %d bytes where copied\n", delta);

	*off += delta;	
	return delta;
}



static struct file_operations fops = {
	.read = acc_read,
	.write = acc_write
	
};

static int __init init_acc_mod(void)
{
	major = register_chrdev(0, "fpga_accelerator", &fops); //0 is for dynamic allocation
	if (major < 0) {
		pr_err("Error registering accelerator device");
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
