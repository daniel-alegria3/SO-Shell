OBJ=kernel
IMG=c.img

all: $(IMG)

$(IMG): $(OBJ) script.sh
	./script.sh

kernel:
	make -C kern
	make -C userland

clean:
	rm -f $(OBJ) *.o bochs.log
	make -C kern clean
	make -C userland clean

run: all
	qemu-system-i386 -hda $(IMG)

.PHONY: all clean run
