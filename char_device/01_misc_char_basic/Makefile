obj-m += misc.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all: misc app

misc:
	make -C $(KDIR) M=$(PWD) misc.ko

app: app.c
	gcc -g -o app app.c

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f app

.PHONY: all misc app clean
