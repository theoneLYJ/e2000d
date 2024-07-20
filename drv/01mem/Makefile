ARCH=arm64
CROSS_COMPILE = aarch64-none-linux-gnu-
obj-m += memory.o
KERNEL_PATH=/home/theonelyj/e2000d/kernel/phytium-linux-kernel

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean

