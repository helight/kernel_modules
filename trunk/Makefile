FSFILE=file
AUDITFS=auditfs
KERNELPATH=/usr/src/linux-2.6.30.5
#KERNELPATH=/usr/src/linux-$(shell uname -r)

obj-m += $(AUDITFS).o $(FSFILE).o netaudit.o
all:
	 make -C $(KERNELPATH)  M=$(PWD) modules

install:
	insmod $(AUDITFS).ko && mount -t $(AUDITFS) $(AUDITFS) /mnt && insmod $(FSFILE).ko
	ls /mnt

clean:
	make -C $(KERNELPATH)  M=$(PWD) clean 
