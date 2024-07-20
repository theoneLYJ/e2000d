#include <bits/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <sys/poll.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include "gpio.h"

int main(int argc, char *argv[])
{
	int len;
	struct gpio_desc gpios;
	char gpio_num[6];
	char pptr4_4[128];
	char pptr4_5[128];
	char buff[256];
	int gpio4_5_val_fd;
	struct pollfd fds[1];
	struct timespec start, end;
	float distance;

	// 初始化gpio管脚编号
	gpio_init();

	// 根据用户输入的组号和管脚编号决定往 /sys/class/gpio/export
	gpios.group = 4;
	gpios.num[0] = 4;
	sprintf(gpio_num, "%d", gpio_get(gpios));
	sprintf(pptr4_4, "/sys/class/gpio/gpio%s", gpio_num);

	if (access(pptr4_4,  F_OK)) {
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
	gpios.num[0] = 5;

	sprintf(gpio_num, "%d", gpio_get(gpios));
	sprintf(pptr4_5, "/sys/class/gpio/gpio%s", gpio_num);

	if (access(pptr4_5,  F_OK)) {
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
	if (-1 == gpio_write(pptr4_4, "edge", "none")) {
		printf("config gpio edge attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_write(pptr4_4, "direction", "out")) {
		printf("config gpio direction attribute error!\n");
		exit(-1);
	}
	gpio_write(pptr4_4, "value", "0");

	if (-1 == gpio_write(pptr4_5, "edge", "both")) {
		printf("config gpio edge attribute error!\n");
		exit(-1);
	}
	if (-1 == gpio_write(pptr4_5, "direction", "in")) {
		printf("config gpio direction attribute error!\n");
		exit(-1);
	}

	memset(buff, 0, sizeof(buff) / sizeof(char));
	sprintf(buff, "%s/value", pptr4_5);

	// 当中断发生，唤醒poll进程他
	gpio4_5_val_fd = open(buff, O_RDONLY);
	if (-1 == gpio4_5_val_fd) {
		perror("open value file error");
		exit(-1);
	}

	fds[0].fd = gpio4_5_val_fd;
	fds[0].events = POLLPRI;
	memset(buff, 0, sizeof(buff) / sizeof(char));
	int ret = read(gpio4_5_val_fd, buff, 10);
	if (-1 == ret) {
		perror("read value file error!");
	}

	while (1) {
		usleep(4);
		gpio_write(pptr4_4, "value", "1");
		clock_gettime(CLOCK_MONOTONIC, &start);
		while ((end.tv_nsec - start.tv_nsec) / 1000 <= 10) {
			for (int i = 0; i < 5; i++) {
				clock_gettime(CLOCK_MONOTONIC, &end);
			}
		}
		gpio_write(pptr4_4, "value", "0");

		ret = poll(fds, 1, -1);
		if (ret == -1) {
			perror("poll error!");
			exit(-1);
		}
		if (fds[0].revents & POLLPRI) {
			ret = lseek(gpio4_5_val_fd, 0, SEEK_SET);
			if (ret == -1) {
				perror("lseek error");
			}
			bzero(buff, sizeof(buff) / sizeof(char));
			ret = read(gpio4_5_val_fd, buff, 1);
			if (ret == -1) {
				perror("read error!");
			}
			if (atoi(buff) == 1) {
				clock_gettime(CLOCK_MONOTONIC, &start);
			} else if (atoi(buff) == 0) {
				clock_gettime(CLOCK_MONOTONIC, &end);
				distance = (float)(end.tv_nsec - start.tv_nsec) / 1000 / 29.4 / 2;
				if (distance > 0) {
					printf("distance: %.2f\n", distance);
				}
			}
		}
		usleep(10000);
	}


	return EXIT_SUCCESS;
}
