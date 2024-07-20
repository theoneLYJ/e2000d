objs := led_pwm.o key.o
ARCH = arm64
CROSS_COMPILE = aarch64-none-linux-gnu-

pwm:$(objs)
	$(CROSS_COMPILE)gcc $^ -o led_app
	cp led_app /home/theonelyj/e2000d/nfsroot

%.o:%.c
	$(CROSS_COMPILE)gcc -Wall -nostdlib -c $< -o $@

clean:
	rm -rf *.o led_app

