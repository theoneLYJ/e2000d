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

#endif // !__GPIO_H__
