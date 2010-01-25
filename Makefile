obj-m += auditfs.o cr_file.o
kernelpath = /usr/src/linux-2.6.30.5 
#kernelpath = /usr/src/linux-headers-2.6.32
#kernelpath = /lib/modules/2.6.32/build/

all:
#	 make -C /usr/src/linux-$(shell uname -r)  M=$(PWD) modules
	 make -C $(kernelpath)  M=$(PWD) modules

install:
		insmod myfs.ko
		insmod cr_file.ko
		mount -t myfs myfs /mnt
		ls /mnt

clean:
	make -C $(kernelpath)  M=$(PWD) clean 
