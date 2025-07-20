obj-m += acc_mod.o 

 

PWD := $(CURDIR) 

 
run: all
	-sudo rmmod acc_mod
	sudo insmod acc_mod.ko

all: 

	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

 

clean: 

	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean


