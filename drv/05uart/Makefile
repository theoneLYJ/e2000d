objs := uart_steering.o
ARCH = arm64
CROSS_COMPILE = aarch64-none-linux-gnu-

pwm:$(objs)
	$(CROSS_COMPILE)gcc $^ -o uart_steering
	cp uart_steering /home/theonelyj/e2000d/nfsroot

%.o:%.c
	$(CROSS_COMPILE)gcc -Wall -nostdlib -c $< -o $@

clean:
	rm -rf *.o uart_steering

