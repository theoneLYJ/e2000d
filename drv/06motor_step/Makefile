objs := motor_step.o
ARCH = arm64
CROSS_COMPILE = aarch64-none-linux-gnu-

pwm:$(objs)
	$(CROSS_COMPILE)gcc $^ -o motor_step
	cp motor_step /home/theonelyj/e2000d/nfsroot

%.o:%.c
	$(CROSS_COMPILE)gcc -Wall -nostdlib -c $< -o $@

clean:
	rm -rf *.o motor_step

