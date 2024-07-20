ARCH=arm64
CROSS_COMPILE = aarch64-none-linux-gnu-
obj-m += i2c_sh1106.o
KERNEL_PATH=/home/theonelyj/e2000d/kernel/phytium-linux-kernel

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules
	cp *.ko /home/theonelyj/e2000d/nfsroot

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean

