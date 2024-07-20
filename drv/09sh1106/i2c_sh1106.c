#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/i2c-smbus.h>
#include <linux/kernel.h>
#include <linux/acpi.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#define SH1106_CMD	0x00
#define SH1106_DATA 0x40

static struct i2c_client *sh1106_client;
static int major;
static struct class *sh1106_class;

static int oled_write_byte(unsigned char reg, unsigned char para, int len)
{
	unsigned char data[2];
	struct i2c_msg msg;
	int ret;

	data[0] = reg;                  //寄存器
	data[1] = para;                 //参数
																	//
	msg.addr = sh1106_client->addr;        //ap3216c地址, 设备树中的地址
	msg.flags = 0;                  //标记为写
	msg.buf = data;                 //要写入的数据缓冲区
	msg.len = len + 1;              //要写入的数据长度
	ret = i2c_transfer(sh1106_client->adapter, &msg, 1); //用于发送的client
	return ret;
}

static void oled_init(void)
{
	int i;
	unsigned char data[] = {
		0xAE, 0x00, 0x10, 0x40, 0xB0, 0x81, 0xFF, 0xA1, 0xA6,
		0xA8, 0x3F, 0xC8, 0xD3, 0x00, 0xD5, 0x80, 0xD8, 0x05,
		0xD9, 0xF1, 0xDA, 0x12, 0xDB, 0x30, 0x8D, 0x14, 0xAF
	};
	for (i = 0; i < sizeof(data) / sizeof(unsigned char); i++) {
		oled_write_byte(SH1106_CMD, data[i], 1);
	}
	return ;
}

int sh1106_open(struct inode *inode, struct file *filp)
{
	oled_init();
	return 0;
}

ssize_t sh1106_write(struct file *filp, const char __user *buf, size_t len, loff_t *loff)
{
	int ret = 0;
	int n = 0;
	int i, j;
	unsigned char data[1024];
	ret = copy_from_user(&data, buf, len);
	for (i = 0; i < 8; i++) {
		oled_write_byte(SH1106_CMD, 0xb0 + i, 1);
		oled_write_byte(SH1106_CMD, 0x00, 1);
		oled_write_byte(SH1106_CMD, 0x10, 1);

		for (j = 0; j < 128; j++) {
			oled_write_byte(SH1106_DATA, data[n], 1);
			n++;
		}
	}
	return ret;
}

static struct file_operations sh1106_fops = {
	.owner = THIS_MODULE,
	.open = sh1106_open,
	.write = sh1106_write,
};


static int sh1106_pad_mux(void)
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

static int sh1106_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	printk(KERN_ERR "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	sh1106_client = client;

	/* register_chrdev */
	major = register_chrdev(0, "dev_sh1106", &sh1106_fops);
	/* class_create */
	sh1106_class = class_create(THIS_MODULE, "sh1106");
	/* device_create */
	device_create(sh1106_class, NULL, MKDEV(major, 0), NULL, "sh1106");

	sh1106_pad_mux();
	return 0;
}

static int sh1106_remove(struct i2c_client *client)
{
	printk(KERN_ERR "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	device_destroy (sh1106_class, MKDEV(major, 0));
	class_destroy (sh1106_class);
	unregister_chrdev (major, "sh1106");
	return 0;
}

static const struct of_device_id sh1106_of_match_table[] = {
	{.compatible = "SinoWealth,sh1106"},
	{}
};

static const struct i2c_device_id sh1106_ids[] = {
	{"xxxyyy", (kernel_ulong_t)NULL},
	{}
};

static struct i2c_driver sh1106_drv = {
	.driver = {
		.name = "sh1106",
		.of_match_table = sh1106_of_match_table,
	},
	.probe = sh1106_probe,
	.remove = sh1106_remove,
	.id_table = sh1106_ids,
};

static int __init sh1106_init(void)
{
	int ret = 0;
	ret = i2c_add_driver(&sh1106_drv);
	return ret;
}

static void __exit sh1106_exit(void)
{
	i2c_del_driver(&sh1106_drv);
}

module_init(sh1106_init);
module_exit(sh1106_exit);

MODULE_LICENSE("GPL");
