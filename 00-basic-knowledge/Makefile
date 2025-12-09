obj-m += export_symbol.o
obj-m += import_symbol.o
obj-m += misc.o

KDIR := /lib/modules/$(shell uname -r)/build

all:
	make -C $(KDIR) M=$(PWD) modules

# 单独编译某个模块，直接指定 .ko 目标
export_symbol:
	make -C $(KDIR) M=$(PWD) export_symbol.ko

import_symbol:
	make -C $(KDIR) M=$(PWD) import_symbol.ko

app: app.c
	gcc -g -o app app.c

misc: app
	make -C $(KDIR) M=$(PWD) misc.ko

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -f app

.PHONY: all export_symbol import_symbol misc clean