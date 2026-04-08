obj-m += misc.o

KDIR := /home/tan/orangepi-build/kernel/orange-pi-5.10-rk35xx
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-linux-gnu-
PWD := $(shell pwd)
CC := $(CROSS_COMPILE)gcc

all: misc app

misc:
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

app: app.c
	$(CC) -g -o $@ $<

clean:
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -f app

.PHONY: all misc app clean
