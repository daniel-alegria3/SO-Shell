OBJ=shell cat
CC=gcc -Wall -m32 -fno-stack-protector -fno-builtin

all: $(OBJ)

shell: shell.o libc/libc.a
	ld -m elf_i386 -Ttext=40000000 --entry=main -o $@ $^

cat: cat.o libc/libc.a
	ld -m elf_i386 -Ttext=40000000 --entry=main -o $@ $^

libc/libc.a:
	make -C libc

.o: .c
	$(CC) -c $^

clean:
	rm -f $(OBJ) *.o *.a
	make -C libc clean
