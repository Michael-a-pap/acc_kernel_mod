#include <linux/module.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/miscdevice.h>

#define MODULE_NAME_LONG "FPGA accelerator"
#define MODULE_NAME_SHORT "fpga_acc"
#define BUF_SIZE 64

static char txt_buff[64];
static size_t buff_len = 0; //Buffer tracking

static ssize_t acc_read(struct file *file_p, char __user *user_buff, size_t len, loff_t *off)
{	

	int not_copied, delta, to_copy = buff_len;

	pr_info("%s - Read is called",MODULE_NAME_LONG);
	pr_info("length to read: %ld\n",len);
	pr_info("bytes to copy: %d\n",to_copy);
	pr_info("offset: %lld",*off);

	not_copied = copy_to_user(user_buff, &txt_buff[*off], to_copy);
	delta = to_copy - not_copied;
	pr_info("not copied: %d",not_copied);
	pr_info("delta: %d",delta);
	buff_len = 0;

	if (not_copied)
		pr_warn("%s - Only %d bytes where copied\n",MODULE_NAME_LONG, delta);

	return delta;
}

static ssize_t acc_write(struct file *file_p, const char __user *user_buff, size_t len, loff_t *off)
{	
	if (len > sizeof(txt_buff))
		return -ENOSPC;

	int not_copied, delta, to_copy = len;
	*off = 0;	
	pr_info("%s - Write is called\n",MODULE_NAME_LONG);
	pr_info("length to write: %ld\n",len);
	pr_info("bytes to copy: %d\n",to_copy);
	pr_info("offset: %lld",*off);
	not_copied = copy_from_user(&txt_buff[*off], user_buff, to_copy);
	delta = to_copy - not_copied;

	if (not_copied)
		pr_warn("%s - %d bytes where copied\n",MODULE_NAME_LONG, delta);
	
	pr_info("delta: %d",delta);
	
	//How many bytes where written so read() will know
	buff_len = len; 

	return delta;
}

static struct file_operations fops = {
	.read = acc_read,
	.write = acc_write
};

static struct miscdevice acc_misc_device = {
	.name = MODULE_NAME_SHORT,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static int __init init_acc_mod(void)
{	
	int status; 
	status = misc_register(&acc_misc_device);
	if (status) {
		pr_err("Error registering device");
		return -status;
	}
	pr_info("%s - Register misc device: \n",MODULE_NAME_LONG); 
	return 0;
} 

static void __exit exit_acc_mod(void) 
{ 
	misc_deregister(&acc_misc_device);
	pr_info("%s - Unregistering",MODULE_NAME_LONG); 
} 

module_init(init_acc_mod);
module_exit(exit_acc_mod);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael A. Papachatzakis");
MODULE_DESCRIPTION("Kernel module for the FPGA accelerator");
