obj-m += intercept.o
intercept-objs := main.o hooks.o stream.o ioctl.o list_manager.o

ccflags-y += -I$(PWD)/../include

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean