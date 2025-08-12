# Use KERNEL_SRC if passed from environment, otherwise fall back to host
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

obj-m := acc_mod.o 

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
