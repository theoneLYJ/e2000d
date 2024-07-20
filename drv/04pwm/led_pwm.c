#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "led_pwm.h"


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

static int pwm_config(const char *attr, const char *val, int node)
{
	int fd, len;
	char file_path[256];

	sprintf(file_path, "/sys/class/pwm/pwmchip%d/pwm%d/%s", node, node, attr);
	if (0 > (fd = open(file_path, O_WRONLY))) {
		perror("open file_path error!");
		return -1;
	}

	len = strlen(val);
	if (len != write(fd, val, len)) {
		perror("write val error!");
		close(fd);
	}
	close(fd);
	return 0;
}

int pwm_init(void)
{
	static char pwm0_path[128];
	static char pwm2_path[128];
	// 使能pwm控制器
	gpio_mode(PWM_ENABLE_REG, WRITE_TYPE_H, PWM_ENABLE_VAL);
	// 配置gpio管脚模式
	gpio_mode(GPIO_PAD_MODE_PWM0_CHANNEL0_REG, WRITE_TYPE_H, GPIO_PAD_MODE_PWM0_CHANNEL0_VAL);
	gpio_mode(GPIO_PAD_MODE_PWM2_CHANNEL0_REG, WRITE_TYPE_H, GPIO_PAD_MODE_PWM2_CHANNEL0_VAL);

	sprintf(pwm0_path, "/sys/class/pwm/pwmchip%s/pwm%s", PWM0_CONTROLLER, PWM0_CHANNEL);
	sprintf(pwm2_path, "/sys/class/pwm/pwmchip%s/pwm%s", PWM2_CONTROLLER, PWM2_CHANNEL);
	if (access(pwm0_path,F_OK)) {
		int fd, len;
		if (0 > (fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY))) {
			perror("open");
			return -1;
		}
		len = strlen(PWM0_CHANNEL);
		if (len != write(fd, PWM0_CHANNEL, len)) {
			perror("write pwm1 export error");
			close(fd);
			return -1;
		}
		close(fd);
	}
	if (access(pwm2_path,F_OK)) {
		int fd, len;
		if (0 > (fd = open("/sys/class/pwm/pwmchip2/export", O_WRONLY))) {
			perror("open");
			return -1;
		}
		len = strlen(PWM0_CHANNEL);
		if (len != write(fd, PWM0_CHANNEL, len)) {
			perror("write pwm2 export error");
			close(fd);
			return -1;
		}
		close(fd);
	}

	pwm_config("period", "100000", 0);
	pwm_config("period", "100000", 2);
	pwm_config("duty_cycle", "0", 0);
	pwm_config("duty_cycle", "0", 2);
	pwm_config("enable", "1", 0);
	pwm_config("enable", "1", 2);

	return 0;
}

int pwm_open(int pwmx, int step)
{
	unsigned long duty1 = atol(PWM_DUTY1);
	unsigned long duty2 = atol(PWM_DUTY2);
	unsigned long duty = duty1;
	int increase = 1;
	char dutys[128];
	step = ((int)(duty2 - duty1) / 1000) * step;

	while (1) {
		if (increase == 1) {
			duty += step;
		} else if (increase == 0) {
			duty -= step;
		}

		memset(dutys, 0, sizeof(dutys) / sizeof(char));
		sprintf(dutys, "%ld", duty);

		if (pwmx == PWM0) {
			if ( 0 > pwm_config("duty_cycle", PWM_DUTY2, 2)) {
				fprintf(stderr, "config pwm2 duty_cycle error");
				exit(-1);
			}
			if ( 0 > pwm_config("duty_cycle", dutys, 0)) {
				fprintf(stderr, "config pwm0 duty_cycle error");
				exit(-1);
			}
		} else if (pwmx == PWM2) {
			if ( 0 > pwm_config("duty_cycle", PWM_DUTY2, 0)) {
				fprintf(stderr, "config pwm0 duty_cycle error");
				exit(-1);
			}
			if ( 0 > pwm_config("duty_cycle", dutys, 2)) {
				fprintf(stderr, "config pwm2 duty_cycle error");
				exit(-1);
			}
		} else if (pwmx == PWM0_2) {
			if ( 0 > pwm_config("duty_cycle", dutys, 0)) {
				fprintf(stderr, "config pwm0 duty_cycle error");
				exit(-1);
			}
			if ( 0 > pwm_config("duty_cycle", dutys, 2)) {
				fprintf(stderr, "config pwm2 duty_cycle error");
				exit(-1);
			}
		}

		if (duty >= duty2) {
			increase = 0;
		}
		if (duty <= duty1) {
			increase = 1;
		}
		usleep(2000);
	}
}

