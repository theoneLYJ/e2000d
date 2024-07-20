#include "gpio.h"

static struct gpio_desc gpio_val[6] = {0};

// 初始化gpio管脚
void gpio_init(void)
{
	int i, j;
	for (i = 0; i < 6; i++) {
		gpio_val[i].group = i;
		for (j = 0; j < 16; j++) {
			gpio_val[i].num[j] = 496 - 16 * i + j;
		}
	}
}

// 根据组号和组内编号获取数字
unsigned int gpio_get(struct gpio_desc gpios)
{
	return gpio_val[gpios.group].num[gpios.num[0]];
}

/*************************************************************************
func: 将type制定的类型（数据类型）的data数据保存到addr映射的虚拟地址空间中
param:	addr 传入的物理地址
				type 访问内存的类型 字（W）半字（H）字节（B）
				data 要写入虚拟地址空间中的数据
**************************************************************************/
int gpio_mode (char *addr, char *type, char *data)
{
	int fd;
	int type_access = 'w';
	unsigned long write_val;
	off_t target;
	void *map_base, *virt_addr;
	if (0 > (fd = open("/dev/mem", O_ASYNC | O_RDWR))) {
		perror("open /sys/mem error!");
		return -1;
	}

	target = strtoul(addr, 0, 0);
	type_access = tolower(type[0]);
	write_val = strtoul(data, 0, 0);

	map_base = mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, \
			fd, target & ~MAP_MASK);
	if ((void *)-1 == map_base) {
		perror("mmap");
		return -1;
	}

	// 计算映射后的虚拟地址
	virt_addr = map_base + (target & MAP_MASK);
	switch (type_access) {
		case 'b':
			*((unsigned char *)virt_addr) = write_val;
			break;
		case 'h':
			*((unsigned short *)virt_addr) = write_val;
			break;
		case 'w':
			*((unsigned int *)virt_addr) = write_val;
			break;
	}
	if (-1 == munmap(map_base, MAP_SIZE)) {
		perror("munmap error!");
		return -1;
	}

	close(fd);
	return 0;
}

// 根据用户自己的需求配置节点文件下的属性文件
int gpio_write(char *pptr, const char *attr, const char *val)
{
	char file_path[256];
	int fd, len;

	sprintf(file_path, "%s/%s", pptr, attr);
	if (0 > (fd = open(file_path, O_WRONLY)))
	{
		perror("open file_path error!");
		return -1;
	}
	len = strlen(val);
	if (len != write(fd, val, len)) {
		perror("write val error!");
		close(fd);
		return -1;
	}

	close(fd);
	return 0;
}
