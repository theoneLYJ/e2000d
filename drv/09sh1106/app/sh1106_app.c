#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "boot_ani.h"

/*
 * sh1106_app /dev/sh1106 r 10
 * sh1106_app /dev/sh1106 w 10
 */

int main(int argc, char *argv[])
{
	int fd;
	int buf[2];
	int addr;

	fd = open("/dev/sh1106", O_RDWR);
	if (fd < 0) {
		perror("open error!");
	}

	for (int i = 0; i < 58; i++) {
		write(fd, openHarmony_All[i], 1024);
	}

	return EXIT_SUCCESS;
}
