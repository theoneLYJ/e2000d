objs := key.o
ARCH = arm64
CROSS_COMPILE = aarch64-none-linux-gnu-

key:$(objs)
	$(CROSS_COMPILE)gcc $^ -o key
	cp key /home/theonelyj/e2000d/nfsroot

%.o:%.c
	$(CROSS_COMPILE)gcc -Wall -nostdlib -c $< -o $@

clean:
	rm -rf *.o key

