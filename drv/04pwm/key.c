#include <signal.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <wait.h>
#include "led_pwm.h"

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

// 根据用户自己的需求配置节点文件下的属性文件
static int gpio_write(char *pptr, char *attr, char *val)
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

int main(int argc, char *argv[])
{
	int len;
	struct gpio_desc gpios;
	char gpio_num[6];
	char pptr4_1[128];
	char pptr4_3[128];
	char buff[256];
	int gpio4_1_val_fd;
	int gpio4_3_val_fd;
	struct pollfd fds[2];
	int pwmx, step;

	// 初始化gpio管脚编号
	gpio_init();
	pwm_init();

	// 根据用户输入的组号和管脚编号决定往 /sys/class/gpio/export
	gpios.group = 4;
	gpios.num[0] = 1;

	sprintf(gpio_num, "%d", gpio_get(gpios));
	sprintf(pptr4_1, "/sys/class/gpio/gpio%s", gpio_num);

	if (access(pptr4_1,  F_OK)) {
		int fd;
		if (0 > (fd = open("/sys/class/gpio/export", O_WRONLY))) {
			perror("open gpio/export");
			exit(-1);
		}

		len = sizeof(gpio_num);
		if (len != write(fd, gpio_num, len)) {
			perror("write error");
			close(fd);
			exit(-1);
		}
	}

	gpios.group = 4;
	gpios.num[0] = 3;

	sprintf(gpio_num, "%d", gpio_get(gpios));
	sprintf(pptr4_3, "/sys/class/gpio/gpio%s", gpio_num);

	if (access(pptr4_3,  F_OK)) {
		int fd;
		if (0 > (fd = open("/sys/class/gpio/export", O_WRONLY))) {
			perror("open gpio/export");
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
	if (-1 == gpio_write(pptr4_1, "edge", "falling")) {
		printf("config gpio edge attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_write(pptr4_1, "direction", "in")) {
		printf("config gpio direction attribute error!\n");
		exit(-1);
	}

	memset(buff, 0, sizeof(buff) / sizeof(char));
	sprintf(buff, "%s/value", pptr4_1);

	// 当中断发生，唤醒poll进程他
	gpio4_1_val_fd = open(buff, O_RDONLY);
	if (-1 == gpio4_1_val_fd) {
		perror("open value file error");
		exit(-1);
	}

	if (-1 == gpio_write(pptr4_3, "edge", "falling")) {
		printf("config gpio edge attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_write(pptr4_3, "direction", "in")) {
		printf("config gpio direction attribute error!\n");
		exit(-1);
	}

	memset(buff, 0, sizeof(buff) / sizeof(char));
	sprintf(buff, "%s/value", pptr4_3);

	gpio4_3_val_fd = open(buff, O_RDONLY);
	if (-1 == gpio4_3_val_fd) {
		perror("open value file error");
		exit(-1);
	}

	fds[0].fd = gpio4_1_val_fd;
	fds[0].events = POLLPRI;
	fds[1].fd = gpio4_3_val_fd;
	fds[1].events = POLLPRI;
	memset(buff, 0, sizeof(buff) / sizeof(char));
	int ret = read(gpio4_1_val_fd, buff, 10);
	if (-1 == ret) {
		perror("read value file error!");
	}

	memset(buff, 0, sizeof(buff) / sizeof(char));
	ret = read(gpio4_3_val_fd, buff, 10);
	if (-1 == ret) {
		perror("read value file error!");
	}

	printf("please press the key1 or key2... \n");
	pwmx = 0;
	step = 1;

	int pid = fork();
	if (pid < 0) {
		perror("fork");
	}
	if (pid == 0) {
		pwm_open(pwmx, step);
	}
	while (1) {
		ret = poll(fds, 2, -1);
		if (ret == -1) {
			perror("poll error!");
			exit(-1);
		}
		if (fds[0].revents & POLLPRI) {
			ret = lseek(gpio4_1_val_fd, 0, SEEK_SET);
			if (ret == -1) {
				perror("lseek error");
			}

			memset(buff, 0, sizeof(buff) / sizeof(char));
			ret = read(gpio4_1_val_fd, buff, 10);
			if (-1 == ret) {
				perror("read value file error!");
			}
			printf("key interrupt is occured!\n");

			if (step == 1) {
				step = 10;
			} else {
				step = 1;
			}
			
			kill(pid, SIGTERM);
			wait(NULL);
			pid = fork();
			if (pid < 0) {
				perror("fork");
			}
			if (pid == 0) {
				pwm_open(pwmx, step);
			}

		}
		if (fds[1].revents & POLLPRI) {
			ret = lseek(gpio4_3_val_fd, 0, SEEK_SET);
			if (ret == -1) {
				perror("lseek error");
			}

			memset(buff, 0, sizeof(buff) / sizeof(char));
			ret = read(gpio4_3_val_fd, buff, 10);
			if (-1 == ret) {
				perror("read value file error!");
			}
			printf("key interrupt is occured!\n");

			pwmx = (pwmx + 1) % 3;

			kill(pid, SIGTERM);
			wait(NULL);
			pid = fork();
			if (pid < 0) {
				perror("fork");
			}
			if (pid == 0) {
				pwm_open(pwmx, step);
			}

		}
	}


	return EXIT_SUCCESS;
}
