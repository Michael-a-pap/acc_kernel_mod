#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>


#define DEBUGFS_DIR "fpga_debug"
#define DEBUGFS_FILE "my_file"
#define MODULE_NAME "fpga_accelerator"
#define COMPATIBLE_STRING "silly_counter"
#define BUF_SIZE 64
#define COUNTER_PHYS_ADDR 0xA0020000
#define COUNTER_REG_SIZE 0x1000

//debugfs entry
static struct dentry *debugfs_dir;
static struct dentry *debugfs_file;
static void __iomem *counter_base;

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

static ssize_t debugfs_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return -EINVAL;
}

static ssize_t debugfs_read(struct file *fp, char __user *user_buffer, size_t count, loff_t *position)
{

	u32 value;
	char buf[32];
	int len;

	if (*position != 0)
		return 0; // only allow one read

	value = ioread32(counter_base); // read from mapped address
	len = snprintf(buf, sizeof(buf), "%u\n", value);

	if (copy_to_user(user_buffer, buf, len))
		return -EFAULT;

	*position = len;
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
	.write = debugfs_write,
	.read = debugfs_read,
};

static int probe_acc_mod(struct platform_device *pdev)
{
	pr_info("Module is probed by an overlay\n");

	counter_base = ioremap(COUNTER_PHYS_ADDR, COUNTER_REG_SIZE);
	if (!counter_base) {
		pr_err(MODULE_NAME ": Failed to map the address\n");
		return -ENOMEM;
	}

	debugfs_dir = debugfs_create_dir(DEBUGFS_DIR, NULL);
	if(!debugfs_dir) {
		return -ENOMEM;
	}

	debugfs_file = debugfs_create_file(DEBUGFS_FILE, 0666, debugfs_dir, NULL, &debugfs_fops);
	if(!debugfs_file) {
		debugfs_remove_recursive(debugfs_dir);
		return -ENOMEM;
	}
	return 0;
}

static int remove_acc_mod(struct platform_device *pdev)
{
	if (counter_base) {
		iounmap(counter_base);
	}

	debugfs_remove_recursive(debugfs_dir);
	pr_info("Overlay is removed\n");
	return 0;
}

static const struct of_device_id acc_mod_of_match[] = {
	{ .compatible = COMPATIBLE_STRING, },
	{}
};
MODULE_DEVICE_TABLE(of, acc_mod_of_match);

static struct platform_driver acc_mod_driver = {
	.probe = probe_acc_mod,
	.remove = remove_acc_mod,
	.driver = {
		.name = MODULE_NAME,
		.of_match_table = acc_mod_of_match,
	},
};
//module_platform_driver(acc_mod_driver); // to be used if no init and exit functions exist/needed

static int __init init_acc_mod(void)
{
	ssize_t status;
	status = misc_register(&acc_misc_device);
	if (status) {
		pr_err("Error registering device");
		return -ENOMEM;
	}

	pr_info("%s - Register misc device: %d\n", MODULE_NAME, acc_misc_device.minor);
	return platform_driver_register(&acc_mod_driver);
}

static void __exit exit_acc_mod(void)
{
	platform_driver_unregister(&acc_mod_driver);
	misc_deregister(&acc_misc_device);
	pr_info("%s - Unregistered\n", MODULE_NAME);
}


module_init(init_acc_mod);
module_exit(exit_acc_mod);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael A. Papachatzakis");
MODULE_DESCRIPTION("Kernel module for the FPGA accelerator");
