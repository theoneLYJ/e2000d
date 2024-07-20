objs := ultrasonic.o gpio.o
ARCH = arm64
CROSS_COMPILE = aarch64-none-linux-gnu-

pwm:$(objs)
	$(CROSS_COMPILE)gcc $^ -o ultrasonic
	cp ultrasonic /home/theonelyj/e2000d/nfsroot

%.o:%.c
	$(CROSS_COMPILE)gcc -Wall -nostdlib -c $< -o $@

clean:
	rm -rf *.o ultrasonic

