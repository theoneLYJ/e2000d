#include <linux/init.h>
#include <linux/module.h>
#include <linux/spi/spi.h>

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

static int major = 0;
static struct class *mpu6500_class = NULL;

static const struct of_device_id mpu6500_dt_ids[] = {
	{ .compatible = "InvenSense,mpu6500", },
	{},
};

struct mpu6500_data {
	dev_t dev;
	spinlock_t spi_lock; // 自旋🔒
	struct spi_device *spi_device;
	struct list_head device_entry;

	// miso mosi
	struct mutex	buff_lock;
	unsigned char *tx_buffer;
	unsigned char *rx_buffer;
	unsigned int	speed_hz;
};

static int mpu6500_probe(struct spi_device *spi)
{
	void __iomem * vaddr;
	struct resource *maddr;
	struct mpu6500_data *spidev;

	printk(KERN_ERR "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);

	// 申请io内存
	maddr = reguest_mem_region(0x32b300e8, 32, " ");
	vaddr = ioremap(maddr->start, 16);

	// 将gpio管脚复用spi功能
	writel(0x40, vaddr);
	writel(0x40, vaddr + 4);
	writel(0x40, vaddr + 8);
	writel(0x40, vaddr + 12);

	// 初始化spi的配置
	spidev = kzalloc(sizeof(*spidev), GFP_KERNEL);
	if (!spidev) {
		return -ENOMEM;
	}
	spidev->spi = spi;

	spin_lock_init(&spidev->spi_lock);
	mutx_init(&spidev->buff_lock);
	INIT_LIST_HEAD(&spidev->device_entry);
	mutex_lock(&device_list_lock);
	
	return 0;
}

static int mpu6500_remove(struct spi_device *spi)
{
	printk(KERN_ERR "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
	return 0;
}

static struct spi_driver mpu6500_driver = {
	.driver = {
		.name = "mpu6500",
		.of_match_table = of_match_ptr(mpu6500_dt_ids),
	},
	.probe = mpu6500_probe,
	.remove = mpu6500_remove,
};

static int __init mpu6500_init(void)
{
	int status;
	// 注册设备
	major = register_chrdev(0, "mpu6500_dev", &mpu6500_fops);
	// 设备类的自动创建
	mpu6500_class =  class_create(THIS_MODULE, "mpu6500");
	// 设备文件的自动创建
	// device_create(mpu6500_class, );
	
	status = spi_register_driver(&mpu6500_driver);
	return status;
}

static void __exit mpu6500_exit(void)
{
	spi_unregister_driver(&mpu6500_driver);
	// 注销设备类
	class_destroy(mpu6500_class);
	// 注销设备
	unregister_chrdev(major, "mpu6500_dev");
	return ;
}

module_init(mpu6500_init);
module_exit(mpu6500_exit);

MODULE_LICENSE("GPL");

