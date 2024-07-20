ARCH=arm64
CROSS_COMPILE = aarch64-none-linux-gnu-
obj-m += led_dev.o
KERNEL_PATH=/home/theonelyj/e2000d/kernel/phytium-linux-kernel

all:
	make -C $(KERNEL_PATH) M=$(PWD) modules
	cp *.ko app/*app /home/theonelyj/e2000d/nfsroot

clean:
	make -C $(KERNEL_PATH) M=$(PWD) clean

