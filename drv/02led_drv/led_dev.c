#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/acpi.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>

MODULE_LICENSE ("GPL");

#define IO_PAD_B		0x032b3015c		//blue led
#define GPIO_DDR_B	0x028039004		//blue led
#define GPIO_DR_B		0x028039000		//blue led

#define IO_PAD_R		0x032b30050		//red led
#define GPIO_DDR_R	0x028034004 	//red led
#define GPIO_DR_R		0x028034000 	//red led

#define IO_PAD_G		0x032b30060		//red led
#define GPIO_DDR_G	0x028035004 	//red led
#define GPIO_DR_G		0x028035000 	//red led

static int major;
static struct class *led_cls;
void __iomem *vaddr;

int led_open (struct inode *inode, struct file *filp)
{

	vaddr = ioremap(IO_PAD_B, 4);
	writel(0x0246, vaddr);

	vaddr = ioremap(GPIO_DDR_B, 4);
	writel(readl(vaddr) | 0x00000001, vaddr);

	// printk (KERN_ALERT "%s - %d - %s\n", __FILE__, __LINE__, __func__);
	return 0;
}

int led_close (struct inode *inode, struct file *filp)
{
	// printk (KERN_ALERT "%s - %d - %s\n", __FILE__, __LINE__, __func__);
	return 0;
}

ssize_t led_read (struct file *filp, char __user *buf, size_t count, loff_t *ppos)
{
	// printk (KERN_ALERT "%s - %d - %s", __FILE__, __LINE__, __func__);

	int ret = 0;
	char val = 255;

	vaddr = ioremap(GPIO_DR_B, 4);
	val = (readl(vaddr) & 0x00000001) & 0x01;
	ret = copy_to_user (buf, &val, count);

	return (ssize_t) ((ret == 0) ? count : -1);
}

ssize_t led_write (struct file *filp, const char __user *buf, size_t count, loff_t *ppos)
{
	// printk (KERN_ALERT "%s - %d - %s", __FILE__, __LINE__, __func__);

	int ret = 0;
	char val = 255;

	ret = copy_from_user(&val, buf, count);

	vaddr = ioremap(GPIO_DR_B, 0x04);
	if (val & 0x1) {
		writel(readl(vaddr) | 0x00000001, vaddr);
	} else {
		writel(readl(vaddr) & 0xfffffffe, vaddr);
	}
	return (ssize_t) ((ret == 0) ? count : -1);
}

static struct file_operations led_fops = 
{
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_close,
	.read = led_read,
	.write = led_write,
};

int __init led_drv_init (void)
{
	major = register_chrdev(0, "led_dev", &led_fops);
	led_cls = class_create (THIS_MODULE, "led"); 
	device_create(led_cls, NULL, MKDEV(major, 0), NULL, "led_gpio5_0");

	return 0;
}
void __exit led_drv_exit (void)
{
	// 接觸映射
	iounmap(vaddr);

	//注销设备文件
	device_destroy (led_cls, MKDEV(major, 0));
	//注销设备类
	class_destroy (led_cls);
	//注销设备号
	unregister_chrdev(major, "led_dev");

	return ;
}
module_init (led_drv_init);
module_exit (led_drv_exit);
