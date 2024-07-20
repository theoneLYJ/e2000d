#ifndef __GPIO_H__
#define __GPIO_H__

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

#define	WRITE_TYPE_W											"W"
#define	WRITE_TYPE_H											"H"
#define	WRITE_TYPE_B											"B"

struct gpio_desc {
	unsigned int group; // 描述gpio的组号
	unsigned int num[16];   // 描述gpio的组内编号
};


void gpio_init(void);
unsigned int gpio_get(struct gpio_desc gpios);
int gpio_mode (char *addr, char *type, char *data);
int gpio_write(char *pptr, const char *attr, const char *val);

#endif // !__GPIO_H__
