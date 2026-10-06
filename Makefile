ifneq ($(KERNELRELEASE),)

scull-objs := main.o

obj-m	:= scull.o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build
PWD       := $(shell pwd)

all: modules source load

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) 

source:
	gcc p1.c -o p1
	gcc p2.c -o p2
	gcc mon.c -o mon

load:
	bash scull_load

endif
