obj-m := lab5fs_mod.o
lab5fs_mod-objs := lab5fs.o lab5fs_inode.o lab5fs_super.o
all: module mkfs

mkfs:
	gcc lab5mkfs.c -o lab5mkfs

module:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm lab5mkfs
