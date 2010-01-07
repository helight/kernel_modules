<<<<<<< .mine
obj-m += auditfs.o cr_file.o
kernelpath = /usr/src/linux-2.6.30.5 

=======
obj-m += xauditfs.o
kernelpath=/home/linux/linux-2.6.32
>>>>>>> .r7
all:
<<<<<<< .mine
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

	
=======
#	 make -C /usr/src/linux-headers-$(shell uname -r)  M=$(PWD) modules
	 make -C $(kernelpath)  M=$(PWD) modules
>>>>>>> .r7
clean:
<<<<<<< .mine
#	 make -C /usr/src/linux-$(shell uname -r)  M=$(PWD) clean 
	 make -C $(kernelpath)  M=$(PWD) clean 
=======
#	 make -C /usr/src/linux-headers-$(shell uname -r)  M=$(PWD) clean 
	 make -C $(kernelpath)  M=$(PWD) clean
>>>>>>> .r7

