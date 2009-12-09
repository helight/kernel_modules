obj-m += xuxfs.o
all:
	 make -C /usr/src/linux-$(shell uname -r)  M=$(PWD) modules
clean:
	 make -C /usr/src/linux-$(shell uname -r)  M=$(PWD) clean 

