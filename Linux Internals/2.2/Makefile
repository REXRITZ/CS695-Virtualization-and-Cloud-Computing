KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

obj-m += bloom_driver.o

all:
	make -C $(KDIR) M=$(PWD) modules
	gcc control_station.c -o control_station
	gcc soldier.c -o soldier

clean:
	make -C $(KDIR) M=$(PWD) clean
	rm -r soldier
	rm -r control_station