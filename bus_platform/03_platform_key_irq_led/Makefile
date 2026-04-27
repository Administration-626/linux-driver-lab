obj-m += myplatform_key_irq_led.o
obj-m += trigger_irq.o

KDIR := /home/tan/orangepi-build/kernel/orange-pi-5.10-rk35xx
ARCH ?= arm64
CROSS_COMPILE ?= aarch64-linux-gnu-
PWD := $(shell pwd)
CC := $(CROSS_COMPILE)gcc

all: modules app

app: app.c
	$(CC) -o $@ $< -g

modules:
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) modules

clean:
	make -C $(KDIR) M=$(PWD) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) clean
	rm -f app

.PHONY: all clean
