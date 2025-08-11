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

//debugfs entry
static struct dentry *debugfs_dir;
static struct dentry *debugfs_file;

static char txt_buff[BUF_SIZE];
static ssize_t loopback_mode = 1;
static const char hello_str[] = "hello\n";

static ssize_t acc_read(struct file *file_p, char __user *user_buff, size_t len, loff_t *off)
{
	pr_info("%s - Read is called\n", MODULE_NAME);

	if (!loopback_mode) {
		strncpy(txt_buff, "hello\n", BUF_SIZE);
	}

	ssize_t not_copied, to_copy = strnlen(txt_buff, BUF_SIZE)+1;


	if (to_copy-1 >= BUF_SIZE){
		pr_info("%s - Buffer is not NULL terminated\n", MODULE_NAME);
		return -ECANCELED;
	}

	if (len < to_copy)
		return -ECANCELED;

	if (*off >= to_copy)
		return 0;

	pr_info("length to read: %ld\n", len);
	pr_info("bytes to copy: %ld\n", to_copy);
	pr_info("offset: %lld", *off);
	not_copied = copy_to_user(user_buff, &txt_buff[*off], to_copy);

	if (not_copied)
		pr_warn("%s - not all bytes where copied\n", MODULE_NAME);

	*off += to_copy;
	return to_copy;
}

static ssize_t acc_write(struct file *file_p, const char __user *user_buff, size_t len, loff_t *off)
{
	ssize_t not_copied;
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

static ssize_t debugfs_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
	char kbuf[2] = {0};

	if (count < 1 || count >2)
		return -EINVAL;

	if (copy_from_user(kbuf, buf, 1))
		return -EINVAL;

	if (kbuf[0] == '1')
		loopback_mode = 1;
	else if (kbuf[0] == '0')
		loopback_mode = 0;
	else
		return -EINVAL;

	pr_info("Loopback set to %ld\n", loopback_mode);
	return count;
}

static ssize_t debugfs_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
	return simple_read_from_buffer(buf, count, ppos, hello_str, sizeof(hello_str) - 1);
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
	.write = debugfs_write,
	.read = debugfs_read,
};

static int __init init_acc_mod(void)
{
	ssize_t status;
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
	pr_info("%s - Unregistered\n", MODULE_NAME);
}

module_init(init_acc_mod);
module_exit(exit_acc_mod);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael A. Papachatzakis");
MODULE_DESCRIPTION("Kernel module for the FPGA accelerator");
