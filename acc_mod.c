#include <linux/module.h>
#include <linux/init.h> 
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>

#define DEBUGFS_DIR "fpga_debug"
#define DEBUGFS_FILE "my_file"
#define MODULE_NAME "fpga_accelerator"
#define BUF_SIZE 64

//debugfs entrie
static struct dentry *debugfs_dir;
static struct dentry *debugfs_file;

static char txt_buff[BUF_SIZE];

static ssize_t acc_read(struct file *file_p, char __user *user_buff, size_t len, loff_t *off)
{	
	pr_info("%s - Read is called", MODULE_NAME);
	int not_copied, to_copy = strnlen(txt_buff, BUF_SIZE)+1;

	if (to_copy-1 >= BUF_SIZE){
		pr_info("%s - Buffer is not NULL terminated", MODULE_NAME);
		return -ECANCELED;
	}

	if (len < to_copy)
		return -ECANCELED;
	
	if (*off >= to_copy)
		return 0;

	pr_info("length to read: %ld\n", len);
	pr_info("bytes to copy: %d\n", to_copy);
	pr_info("offset: %lld", *off);
	not_copied = copy_to_user(user_buff, &txt_buff[*off], to_copy);

	if (not_copied)
		pr_warn("%s - not all bytes where copied\n", MODULE_NAME);

	*off += to_copy;
	return to_copy;
}

static ssize_t acc_write(struct file *file_p, const char __user *user_buff, size_t len, loff_t *off)
{
	int not_copied;	
	memset(txt_buff, 0xff, sizeof(txt_buff));
	pr_info("%s - Write is called\n", MODULE_NAME);
	
	if (len > sizeof(txt_buff))
		return -EMSGSIZE;

	pr_info("length to write: %ld\n", len);
	pr_info("offset: %lld", *off);
	not_copied = copy_from_user(&txt_buff[*off], user_buff, len);

	if (not_copied){
		pr_warn("%s - not all bytes where copied\n", MODULE_NAME); 
		return -ECANCELED;
	}
	return len;
}

static struct file_operations fops = {
	.read = acc_read,
	.write = acc_write
};

static struct miscdevice acc_misc_device = {
	.name = MODULE_NAME,
	.minor = MISC_DYNAMIC_MINOR,
	.fops = &fops,
};

static struct file_operations debugfs_fops = {
};

static int __init init_acc_mod(void)
{	
	int status; 
	status = misc_register(&acc_misc_device);
	if (status) {
		pr_err("Error registering device");
		return -ENOMEM;
	}

	debugfs_dir = debugfs_create_dir(DEBUGFS_DIR, NULL);
	if(!debugfs_dir) {
		misc_deregister(&acc_misc_device);
		return -ENOMEM;
	}

	debugfs_file = debugfs_create_file(DEBUGFS_FILE, 0666, debugfs_dir, NULL, &debugfs_fops);
	if(!debugfs_file) {
		debugfs_remove_recursive(debugfs_dir);
		misc_deregister(&acc_misc_device);
		return -ENOMEM;
	}
		
	pr_info("%s - Register misc device: \n", MODULE_NAME); 
	return 0;
} 

static void __exit exit_acc_mod(void) 
{ 
	debugfs_remove_recursive(debugfs_dir);
	misc_deregister(&acc_misc_device);
	pr_info("%s - Unregistering", MODULE_NAME); 
} 

module_init(init_acc_mod);
module_exit(exit_acc_mod);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael A. Papachatzakis");
MODULE_DESCRIPTION("Kernel module for the FPGA accelerator");
