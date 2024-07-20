#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/i2c-smbus.h>
#include <linux/kernel.h>
#include <linux/acpi.h>
#include <asm/uaccess.h>

#define ACHANNEL0	0x40
#define ACHANNEL1	0x41
#define ACHANNEL2	0x42
#define ACHANNEL3	0x43

static int major;
static struct i2c_client *pcf8591_client;
static struct class *pcf8591_class;

int pcf8591_open (struct inode *inode, struct file *filp);
ssize_t pcf8591_read (struct file *filp, char __user *buf, size_t len, loff_t *loff);
ssize_t pcf8591_write (struct file *filp, const char __user *buf, size_t len, loff_t *loff);
static int pcf8591_probe (struct i2c_client *client, const struct i2c_device_id *id);
static int pcf8591_remove (struct i2c_client *client);

static struct file_operations pcf8591_fops = {
	.owner = THIS_MODULE,
	.open = pcf8591_open,
	.read = pcf8591_read,
	.write = pcf8591_write,
};

//i2c client
static const struct of_device_id pcf8591_of_match[] = 
{
	{.compatible = "philips,pcf8591"},
	{}
};

static const struct i2c_device_id pcf8591_ids[] = 
{
	{"xxxyyyzzz", (kernel_ulong_t)NULL},
	{},
};

//i2c driver 
static struct i2c_driver i2c_drv = 
{
	.driver = {
		.name = "pcf8591",
		.of_match_table = pcf8591_of_match,
	},
	.probe = pcf8591_probe,
	.remove = pcf8591_remove,
	.id_table = pcf8591_ids,
};

unsigned char pcf8591_readADC (unsigned char ch)
{
	return i2c_smbus_read_byte_data (pcf8591_client, ch);
}


int pcf8591_open (struct inode *inode, struct file *filp)
{
	return 0;
}

ssize_t pcf8591_read (struct file *filp, char __user *buf, size_t len, loff_t *loff)
{
	unsigned long ret = 0;
	unsigned char data = 255;
	if (buf[0] == 0) {
		data = pcf8591_readADC (ACHANNEL0);
	} else if (buf[0] == 1) {
		data = pcf8591_readADC (ACHANNEL1);
	} else if (buf[0] == 2) {
		data = pcf8591_readADC (ACHANNEL2);
	} else if (buf[0] == 3) {
		data = pcf8591_readADC (ACHANNEL3);
	}
	ret = copy_to_user (buf, &data, 1);
	return (size_t)ret;
}


ssize_t pcf8591_write (struct file *filp, const char __user *buf, size_t len, loff_t *loff)
{
	unsigned char data = 255;
	unsigned long ret = 0;
	ret = copy_from_user (&data, buf, 1);
	i2c_smbus_write_byte_data (pcf8591_client, ACHANNEL0, data);
	return ret;
}


static int pcf8591_pad_mux (void)
{
	void __iomem * vaddr;
	struct resource *maddr;
	//申请物理地址, 并将物理地址映射到内核的虚拟地址空间中
	maddr = request_mem_region (0x32b300a8, 16, "null");
	vaddr = ioremap (maddr->start, 8);
	writel (0x44, vaddr);
	writel (0x44, vaddr + 4);
	
	//将mio6控制器配置为ii2控制模式
	vaddr = ioremap (0x28021000, 4);
	writel (0, vaddr);
	return 0;
}

static int pcf8591_probe (struct i2c_client *client, const struct i2c_device_id *id)
{	
	 pcf8591_client = client;
	 if (0 == i2c_check_functionality (client->adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		 return -1;
	 }
	 //a) 注册cdev对象
	 major = register_chrdev (0, "dev_pcf8591", &pcf8591_fops);
	 //b) 完成设备类和设备文件的自动创建 
	 pcf8591_class = class_create (THIS_MODULE, "pcf8591");
	 device_create (pcf8591_class, NULL, MKDEV(major, 0), NULL, "pcf8591");
	 
	 pcf8591_pad_mux();
	 return 0;
}

static int pcf8591_remove (struct i2c_client *client) 
{
	release_mem_region (0x32b300a8, 16);
	device_destroy (pcf8591_class, MKDEV(major, 0));
	class_destroy (pcf8591_class);
	unregister_chrdev (major, "pcf8591");
	return 0;
}

static int i2c_dev_init (void)
{
	int err;
	//将I2C的驱动添加到i2c的core中
	err = i2c_add_driver (&i2c_drv);
	return err;
}

static void i2c_dev_exit (void)
{
	i2c_del_driver (&i2c_drv);
}

module_init (i2c_dev_init);
module_exit (i2c_dev_exit);
MODULE_LICENSE ("GPL");

