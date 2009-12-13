obj-m += xauditfs.o
kernelpath=/home/linux/linux-2.6.32
all:
#	 make -C /usr/src/linux-headers-$(shell uname -r)  M=$(PWD) modules
	 make -C $(kernelpath)  M=$(PWD) modules
clean:
#	 make -C /usr/src/linux-headers-$(shell uname -r)  M=$(PWD) clean 
	 make -C $(kernelpath)  M=$(PWD) clean

