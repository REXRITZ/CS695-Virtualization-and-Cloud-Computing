KDIR := /lib/modules/$(shell uname -r)/build
PWD	:= $(shell pwd)

obj-m += lkm1.o
obj-m += lkm2.o
obj-m += lkm3.o
obj-m += lkm4.o
obj-m += lkm5.o

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc test1.c -o test1
	gcc test2.c -o test2

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -r test1
	rm -r test2
