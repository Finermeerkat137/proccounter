obj-m += proccounter.o

CFLAGS += "-Wall"

ifeq ($(APILOCATION),)
APILOCATION := $(shell uname -r)
endif

all:
	make -C /lib/modules/$(APILOCATION)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(APILOCATION)/build M=$(PWD) clean

module: all
	mv proccounter.ko /tmp
	make clean
	xz /tmp/proccounter.ko
	mv /tmp/proccounter.ko.xz .

install: module
	sudo insmod proccounter.ko.xz
