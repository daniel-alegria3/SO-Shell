OBJ=libc.a
CC=gcc -Wall -m32 -fno-stack-protector -fno-builtin

all: $(OBJ)

libc.a: libc.o syscalls.o malloc.o
	ar rcs $@ $^

.o: .c
	$(CC) -c $^

clean:
	rm -f $(OBJ) *.o *.a
