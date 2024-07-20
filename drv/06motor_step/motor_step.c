#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

#define MAP_SIZE 4094UL
#define MAP_MASK (MAP_SIZE - 1)

#define GPIO_PAD_MODE_GPIO4_11_REG				"0x32b30148"
#define GPIO_PAD_MODE_GPIO5_12_REG				"0x32b3018c"
#define GPIO_PAD_MODE_GPIO5_13_REG				"0x32b30190"
#define GPIO_PAD_MODE_GPIO5_14_REG				"0x32b30194"

#define GPIO_PAD_MODE_GPIO4_11_VAL				"0x246"
#define GPIO_PAD_MODE_GPIO5_12_VAL				"0x046"
#define GPIO_PAD_MODE_GPIO5_13_VAL				"0x046"
#define GPIO_PAD_MODE_GPIO5_14_VAL				"0x046"

#define	WRITE_TYPE_W											"W"
#define	WRITE_TYPE_H											"H"
#define	WRITE_TYPE_B											"B"

struct gpio_desc {
	unsigned int group; // 描述gpio的组号
	unsigned int num[16];   // 描述gpio的组内编号
};

static struct gpio_desc gpio_val[6] = {0};

// 初始化gpio管脚
static void gpio_init(void)
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
static unsigned int gpio_get(struct gpio_desc gpios)
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
static int gpio_config(char *pptr, const char *attr, const char *val)
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

static int motor_step_init()
{
	int ret;
	ret = gpio_mode(GPIO_PAD_MODE_GPIO4_11_REG, WRITE_TYPE_H, GPIO_PAD_MODE_GPIO4_11_VAL);
	if (ret != 0) {
		fprintf(stderr, "config gpio4_11 mod error!");
	}
	ret = gpio_mode(GPIO_PAD_MODE_GPIO5_12_REG, WRITE_TYPE_H, GPIO_PAD_MODE_GPIO5_12_VAL);
	if (ret != 0) {
		fprintf(stderr, "config gpio5_12 mod error!");
	}
	ret = gpio_mode(GPIO_PAD_MODE_GPIO5_13_REG, WRITE_TYPE_H, GPIO_PAD_MODE_GPIO5_13_VAL);
	if (ret != 0) {
		fprintf(stderr, "config gpio5_13 mod error!");
	}
	ret = gpio_mode(GPIO_PAD_MODE_GPIO5_14_REG, WRITE_TYPE_H, GPIO_PAD_MODE_GPIO5_14_VAL);
	if (ret != 0) {
		fprintf(stderr, "config gpio5_14 mod error!");
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	struct gpio_desc gpios;
	int len;
	char gpio_num[6];
	char gpio4_11_pptr[128];
	char gpio5_12_pptr[128];
	char gpio5_13_pptr[128];
	char gpio5_14_pptr[128];
	char buff[256];

	ret = motor_step_init();
	if (ret) {
		fprintf(stderr, "motor step init error!");
	}

	gpio_init();

	gpios.group = 4;
	gpios.num[0] = 11;

	sprintf(gpio_num, "%d", gpio_get(gpios));
	sprintf(gpio4_11_pptr, "/sys/class/gpio/gpio%s", gpio_num);

	if (access(gpio4_11_pptr,  F_OK)) {
		int fd;
		if (0 > (fd = open("/sys/class/gpio/export", O_WRONLY))) {
			perror("open");
			exit(-1);
		}

		len = sizeof(gpio_num);
		if (len != write(fd, gpio_num, len)) {
			perror("write error");
			close(fd);
			exit(-1);
		}
	}

	gpios.group = 5;
	gpios.num[0] = 12;

	sprintf(gpio_num, "%d", gpio_get(gpios));
	sprintf(gpio5_12_pptr, "/sys/class/gpio/gpio%s", gpio_num);

	if (access(gpio5_12_pptr,  F_OK)) {
		int fd;
		if (0 > (fd = open("/sys/class/gpio/export", O_WRONLY))) {
			perror("open");
			exit(-1);
		}

		len = sizeof(gpio_num);
		if (len != write(fd, gpio_num, len)) {
			perror("write error");
			close(fd);
			exit(-1);
		}
	}

	gpios.num[0] = 13;

	sprintf(gpio_num, "%d", gpio_get(gpios));
	sprintf(gpio5_13_pptr, "/sys/class/gpio/gpio%s", gpio_num);

	if (access(gpio5_13_pptr,  F_OK)) {
		int fd;
		if (0 > (fd = open("/sys/class/gpio/export", O_WRONLY))) {
			perror("open");
			exit(-1);
		}

		len = sizeof(gpio_num);
		if (len != write(fd, gpio_num, len)) {
			perror("write error");
			close(fd);
			exit(-1);
		}
	}

	gpios.num[0] = 14;

	sprintf(gpio_num, "%d", gpio_get(gpios));
	sprintf(gpio5_14_pptr, "/sys/class/gpio/gpio%s", gpio_num);

	if (access(gpio5_14_pptr,  F_OK)) {
		int fd;
		if (0 > (fd = open("/sys/class/gpio/export", O_WRONLY))) {
			perror("open");
			exit(-1);
		}

		len = sizeof(gpio_num);
		if (len != write(fd, gpio_num, len)) {
			perror("write error");
			close(fd);
			exit(-1);
		}
	}

	// 配置节点文件下的属性文件 edge 和 direction
	if (-1 == gpio_config(gpio4_11_pptr, "edge", "none")) {
		printf("config gpio edge attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_config(gpio4_11_pptr, "direction", "out")) {
		printf("config gpio direction attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_config(gpio5_12_pptr, "edge", "none")) {
		printf("config gpio edge attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_config(gpio5_12_pptr, "direction", "out")) {
		printf("config gpio direction attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_config(gpio5_13_pptr, "edge", "none")) {
		printf("config gpio edge attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_config(gpio5_13_pptr, "direction", "out")) {
		printf("config gpio direction attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_config(gpio5_14_pptr, "edge", "none")) {
		printf("config gpio edge attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_config(gpio5_14_pptr, "direction", "out")) {
		printf("config gpio direction attribute error!\n");
		exit(-1);
	}

	printf("config gpio success!\n");

	while (1) {
		gpio_config(gpio4_11_pptr, "value", "1");
		gpio_config(gpio5_12_pptr, "value", "0");
		gpio_config(gpio5_13_pptr, "value", "0");
		gpio_config(gpio5_14_pptr, "value", "0");
		usleep(20000);
		gpio_config(gpio4_11_pptr, "value", "0");
		gpio_config(gpio5_12_pptr, "value", "1");
		gpio_config(gpio5_13_pptr, "value", "0");
		gpio_config(gpio5_14_pptr, "value", "0");
		usleep(20000);
		gpio_config(gpio4_11_pptr, "value", "0");
		gpio_config(gpio5_12_pptr, "value", "0");
		gpio_config(gpio5_13_pptr, "value", "1");
		gpio_config(gpio5_14_pptr, "value", "0");
		usleep(20000);
		gpio_config(gpio4_11_pptr, "value", "0");
		gpio_config(gpio5_12_pptr, "value", "0");
		gpio_config(gpio5_13_pptr, "value", "0");
		gpio_config(gpio5_14_pptr, "value", "1");
		usleep(20000);
	}

	return EXIT_SUCCESS;
}
