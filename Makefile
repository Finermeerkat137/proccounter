obj-m += proccounter.o

CFLAGS += "-Wall"

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

module: all
	mv proccounter.ko /tmp
	make clean
	xz /tmp/proccounter.ko
	mv /tmp/proccounter.ko.xz .

install: module
	sudo insmod proccounter.ko.xz
