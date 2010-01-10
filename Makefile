obj-m += auditfs.o cr_file.o
kernelpath = /usr/src/linux-2.6.30.5 

all:
#	 make -C /usr/src/linux-$(shell uname -r)  M=$(PWD) modules
	 make -C $(kernelpath)  M=$(PWD) modules

install:
	if [[ ( -f xuxfs.ko ) && ( -f cr_file.ko ) ]] ; then
		sudo insmod xuxfs.ko
		if [ $? -eq 0 ] ; then
			sudo insmod cr_file.ko
			if [ $? -eq 0 ] ; then
				sudo mount -t xuxfs ssb /mnt
				ls /mnt
			fi
		fi
	fi

	
clean:
#	 make -C /usr/src/linux-$(shell uname -r)  M=$(PWD) clean 
	 make -C $(kernelpath)  M=$(PWD) clean 

