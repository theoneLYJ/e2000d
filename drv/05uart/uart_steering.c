#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
enum uart_port{ 
	COMPORT1 = 1,
	COMPORT2 = 2,
	COMPORT_MAX,
};

enum check_event{ 
	ODD,
	EVEN,
	NONE,
	EVENT_MAX,
};

// uart控制器的属性配置
struct uart_desc {
	enum uart_port port;
	unsigned int baudrate;
	unsigned char nbits;
	unsigned char stopbits;
	enum check_event event;
};

// 定义控制舵机工作的数组下标，为了方便给舵机发送命令
enum steering_cmd {
	VERSION,
	SPEED0,
	SPEED7,
	DEGREE0,
	DEGREE90,
	DEGREE180,
	POS_READ,
	STAT_READ,
	BUTTON_READ,
	LED_ON,
	LED_OFF,
	STEERING_MAX
};

unsigned char steering_control[]={
    0x01, 0x03, 0x00, 0x00, 0x00, 0x01, 0x84, 0x0a,     //读取软件版本号
    0x01, 0x06, 0x00, 0x02, 0x00, 0x00, 0x28, 0x0a,     //设置舵机速度0（最慢）
    0x01, 0x06, 0x00, 0x02, 0x00, 0x07, 0x69, 0xC8,     //设置舵机速度0（最快） 
    0x01, 0x06, 0x00, 0x06, 0x01, 0xf4, 0x69, 0xdc,     //舵机转到0度（位置值500） 
    0x01, 0x06, 0x00, 0x06, 0x05, 0xdc, 0x6b, 0x02,     //舵机转到90度（位置值1500） 
    0x01, 0x06, 0x00, 0x06, 0x09, 0xc4, 0x6e, 0x08,     //舵机转到180度（位置值2500）   
    0x01, 0x03, 0x00, 0x06, 0x00, 0x01, 0x64, 0x0b,     //读舵机的位置 
    0x01, 0x03, 0x00, 0x0a, 0x00, 0x01, 0xa4, 0x08,     //读所有舵机运动状态 
    0x01, 0x03, 0x00, 0x0b, 0x00, 0x01, 0xf5, 0xc8,     //读按钮自上次读取后是否被按下过    
    0x01, 0x06, 0x00, 0x0c, 0x00, 0x01, 0x88, 0x09,     //设置用户LED灯开   
    0x01, 0x06, 0x00, 0x0c, 0x00, 0x00, 0x49, 0xc9,     //设置用户LED灯关   
};

/********************************************
 * uart_open:
 * func: 配置串口
 * param: 要配置的端口号(0-3)
 * return: 成功返回设备文件的文件描述符，错误返回-1
 ********************************************/
static int uart_open (int com)
{
	int fd;
	char dev_uart[128];
	sprintf(dev_uart, "/dev/ttyAMA%d", com);

	fd = open(dev_uart, O_RDWR | O_NOCTTY | O_NDELAY);
	if (0 > fd) {
		perror("open serial port failed!");
		return -1;
	}
	
	if (fcntl(fd, F_SETFL, 0) < 0) {
		perror("fcntl error!");
		return -1;
	}

	if (isatty(STDIN_FILENO) == 0) {
		printf("standard input is not a terminal device\n");
	}

	return fd;
}

/********************************************
 * uart_setting:
 * func: 配置串口的属性信息
 * param: fd 对应的已打开的串口设备的文件描述符
 *				uart uart核心数据结构的变量
 * return: 成功返回设备文件的文件描述符，错误返回-1
 ********************************************/
static int uart_setting(int fd, struct uart_desc uart)
{
	struct termios oldtio, newtio;
	if ( 0 > tcgetattr(fd,	&oldtio)) {
		perror("tcgetattr");
		return -1;
	}

	bzero(&newtio, sizeof(newtio));

	// 硬件流控不选 使能接收端
	newtio.c_cflag |= CLOCAL | CREAD;
	// 数据位清0
	newtio.c_cflag &= ~CSIZE;

	// 设置数据位的个数
	switch (uart.nbits) {
		case 5:
			newtio.c_cflag |= CS5;
			break;
		case 6:
			newtio.c_cflag |= CS6;
			break;
		case 7:
			newtio.c_cflag |= CS7;
			break;
		case 8:
			newtio.c_cflag |= CS8;
			break;
	}

	// 设置奇偶校验模式
	switch (uart.event) {
		case ODD:
			newtio.c_cflag |= PARENB | PARODD;
			newtio.c_cflag |= (INPCK | ISTRIP);
			break;
		case EVEN:
			newtio.c_cflag |= PARENB;
			newtio.c_cflag |= (INPCK | ISTRIP);
			newtio.c_cflag &= ~(PARODD);
			break;
		case NONE:
			newtio.c_cflag &= ~(PARENB);
			break;
		case EVENT_MAX:
			printf("check bit setting error");
			return -1;
	}

	// 设置波特率
	switch (uart.baudrate) {
		case 2400:
			cfsetispeed(&newtio, B2400);
			cfsetospeed(&newtio, B2400);
			break;
		case 9600:
			cfsetispeed(&newtio, B9600);
			cfsetospeed(&newtio, B9600);
			break;
		case 115200:
			cfsetispeed(&newtio, B115200);
			cfsetospeed(&newtio, B115200);
			break;
	}

	// 设置停止位个数
	if (uart.stopbits == 1) {
		newtio.c_cflag &= ~CSTOPB;
	} else if (uart.stopbits == 2) {
		newtio.c_cflag |= CSTOPB;
	}

	// 轮询读串口的数据
	newtio.c_cc[VTIME] = 0;
	newtio.c_cc[VMIN] = 0;

	tcflush(fd, TCIFLUSH);

	if (0 > tcsetattr(fd, TCSANOW, &newtio)) {
		perror("tcsetattr");
		return -1;
	}

	return 0;
}

/********************************************
 * steering_ctrl:
 * func: 将命令写入到打开的串口设备文件描述符
 * param: fd 对应的已打开的串口设备的文件描述符
 *				buf 存放命令的数组
 *				cmd 访问的数组下标
 * return: 成功返回回填参数的个数，错误返回-1
 ********************************************/
static int steering_ctrl(int fd, char *outdata, enum steering_cmd cmd)
{
	int ret;
	switch (cmd) {
		case VERSION:
			ret = write(fd, &steering_control[0*8], 8);
			break;
		case SPEED0:
			ret = write(fd, &steering_control[1*8], 8);
			break;
		case SPEED7:
			ret = write(fd, &steering_control[2*8], 8);
			break;
		case DEGREE0:
			ret = write(fd, &steering_control[3*8], 8);
			break;
		case DEGREE90:
			ret = write(fd, &steering_control[4*8], 8);
			break;
		case DEGREE180:
			ret = write(fd, &steering_control[5*8], 8);
			break;
		case POS_READ:
			ret = write(fd, &steering_control[6*8], 8);
			break;
		case STAT_READ:
			ret = write(fd, &steering_control[7*8], 8);
			break;
		case BUTTON_READ:
			ret = write(fd, &steering_control[8*8], 8);
			break;
		case LED_ON:
			ret = write(fd, &steering_control[9*8], 8);
			break;
		case LED_OFF:
			ret = write(fd, &steering_control[10*8], 8);
			break;
		case STEERING_MAX:
			break;
	}
	if (ret == 8) {
		usleep(5000);
		ret = read(fd, outdata, 8);
		if (ret != 8) {
			printf("read failed\n");
		}

	}
	return ret;
}


int main(int argc, char *argv[])
{
	int fd;
	struct uart_desc uart;
	char buf[8];
	// user useage： ./uart_steering port <angle>
	if (argc < 3 || atoi(argv[1]) != COMPORT2) {
		printf("useage： ./uart_steering 2 <angle:%s>\n", argv[2]);
		exit(-1);
	}

	// 打开串口
	uart.port = COMPORT2;
	if (0 > (fd = uart_open(uart.port))) {
		perror("uart_open error!");
	}

	// 配置uart2的参数信息
	uart.baudrate = 115200;
	uart.nbits = 8;
	uart.stopbits = 1;
	uart.event = NONE;

	// 将配置好的串口参数设置到设备中
	int ret = 0;
	if (0 > (ret = uart_setting(fd, uart))) {
		perror("uart_setting error");
		exit(-1);
	}

	printf("uart_setting success!\n");
	
	// 读取舵机控制器的软件版本号
	steering_ctrl(fd, buf, VERSION);
	printf("version: V%d.%d\n", buf[4], buf[5]);
	printf("please press the key3...\n");

	ret = 0;
	while (ret == 0) {
		// 读取舵机的运动状态,buf[4] << 8 | buf[5]不为0,证明舵机可用
		steering_ctrl(fd, buf, BUTTON_READ);
		ret = buf[4] << 8 | buf[5];
		usleep(100000);
	}
	printf("steering is active...\n");

	steering_ctrl(fd, buf, LED_ON);
	usleep(100000);

	steering_ctrl(fd, buf, SPEED0);
	usleep(100000);
	// 根据用户输入的旋转角度信息,让舵机进行运动
	if (strcmp(argv[2], "90") == 0) {
		steering_ctrl(fd, buf, DEGREE90);
	} else if (strcmp(argv[2], "180") == 0) {
		steering_ctrl(fd, buf, DEGREE180);
	} else {
		steering_ctrl(fd, buf, DEGREE0);
	}
	usleep(100000);

	// 等待舵机运动完成并读取运动状态
	ret = 0xfffc;
	while (ret != 0xfffc) {
		read(fd, buf, 0);
		ret = buf[4] << 8 | buf[5];
		usleep(100000);
	}

	ret = 0xffff;
	while (ret != 0) {
		steering_ctrl(fd, buf, STAT_READ);
		ret = buf[4] << 8 | buf[5];
		usleep(100000);
	}

	// 舵机复位
	steering_ctrl(fd, buf, DEGREE0);
	// 等待舵机运动完成并读取运动状态
	ret = 0xfffc;
	while (ret != 0xfffc) {
		read(fd, buf, 0);
		ret = buf[4] << 8 | buf[5];
		usleep(100000);
	}

	ret = 0xffff;
	while (ret != 0) {
		steering_ctrl(fd, buf, STAT_READ);
		ret = buf[4] << 8 | buf[5];
		usleep(100000);
	}

	steering_ctrl(fd, buf, LED_OFF);
	usleep(100000);
	return EXIT_SUCCESS;
}


















