obj-m := acc_mod.o

# Use KERNEL_SRC if passed from environment, otherwise fall back to host
KERNEL_SRC ?= /lib/modules/$(shell uname -r)/build

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD)/build modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD)/build modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD)/build clean
