#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char **argv)
{
	char buf[2];
	int ret, val;
	if (argc != 2 || (strcmp("on", argv[1]) != 0 && strcmp("off", argv[1]) != 0)) {
		printf("usage:\n");
		printf("		on / off\n");
		exit(-1);
	}
	int fd = open ("/dev/led_gpio5_0", O_RDWR);
	if (fd < 0) {
		perror ("open");
		return -1;
	}
	printf ("open successed!\n");

	ret = read(fd, &buf[0], 1);
	if (ret >= 0) {
		printf("read led_gpio5_0 val %d\n", buf[0]);
	} else {
		perror("read");
		exit(-1);
	}

	if (!strcmp("on", argv[1])) {
		val = 1;
	} else {
		val = 0;
	}

	ret = write(fd, &val, 1);
	if (ret == -1) {
		perror("write");
		close(fd);
		exit(-1);
	}

	close(fd);
	return 0;
}


