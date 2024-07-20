#include <linux/init.h>
#include <linux/module.h>

static int major;
static struct class *led_cls;

// static struct file_operations led_fops = 
// {
// 	.owner = THIS_MODULE,
// 	.open = led_open,
// 	.release = led_close,
// 	.read = led_read,
// 	.write = led_write,
// };

