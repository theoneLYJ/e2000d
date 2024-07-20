#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

char key_val;

void *key_func()
{
	key_val = getchar();
	return NULL;
}

int main(int argc, char *argv[])
{
	int fd;
	char buf[128];
	pthread_t pt_key;
	unsigned char data;
	int nbytes = 0;
	float value;

	if (argc < 2) {
		printf("usage: ./pcf8591_app <channal>\n");
		printf("<channal>: 0 1 3\n");
		printf("  0: brightness\n");
		printf("  1: tempeature\n");
		printf("  3: volatge\n");
		exit(-1);
	}

	fd = open("/dev/pcf8591", O_RDWR);
	if (-1 == fd) {
		perror("open");
		exit(-1);
	}

	// 创建一个
	pthread_create(&pt_key, NULL, key_func, NULL);
	printf("press 'q' to exit...\n");
	usleep(1000000);

	// polling
	while (1) {
		data = (unsigned char)atoi(argv[1]);
		nbytes = read(fd, &data, 1);
		nbytes = write(fd, &data, 1);
		value = (float)data * (3.3 / 255);
		printf("ADC%s = %.3fV\n", argv[1], value);
		usleep(200000);
		if (key_val == 'q') {
			break;
		}
	}
	
	close(fd);
	// 线程的汇合 pthread_detach ---> 线程的分离
	pthread_join(pt_key, NULL);

	return EXIT_SUCCESS;
}
