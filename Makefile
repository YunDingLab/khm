# Makefile for kmod

TARGET = khm

ifneq ($(KERNELRELEASE),)
	EXTRA_CFLAGS += -I$(PWD)/../include/ -I$(PWD)/../internal/
	EXTRA_CFLAGS += -DBASENAME='"${TARGET}"'
	EXTRA_CFLAGS += -Wno-unused-function -Wno-incompatible-pointer-types -Wno-date-time
	EXTRA_LDFLAGS += -T $(PWD)/${TARGET}.lds

	obj-m := ${TARGET}.o

	${TARGET}-y :=   \
		main.o      \
		ctor.o       \
		hook.o		 \
		filter.o
else
	ifneq ($(wildcard /lib/modules/$(shell uname -r)/build/.*),)
		# for ubuntu
		KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	else
		# for centos
		KERNELDIR ?= /usr/src/kernels/$(shell uname -r)
	endif
	PWD := $(shell pwd)

.PHONY: all

all: default

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif
