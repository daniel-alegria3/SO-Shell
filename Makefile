OBJ=kernel

all: $(OBJ)
	./script.sh
	qemu-system-i386 -hda ./c.img

kernel:
	make -C kern
	make -C userland

clean:
	rm -f $(OBJ) *.o bochs.log
	make -C kern clean
	make -C userland clean
