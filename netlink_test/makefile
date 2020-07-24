obj-m += netlink_kernel.o
#generate the path
CURRENT_PATH:=$(shell pwd)
#the current kernel version number
LINUX_KERNEL:=$(shell uname -r)
#the absolute path
#LINUX_KERNEL_PATH:=/usr/src/linux-headers-$(LINUX_KERNEL)
LINUX_KERNEL_PATH:=/usr/src/linux-headers-4.19.0-9-amd64/
#complie object
all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules
#clean
clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean