OBJ=shell cat
CC=gcc -Wall -fno-builtin

all: $(OBJ) 

shell: shell.o libc/libc.a 
	ld -Ttext=40000000 --entry=main -o $@ $^

cat: cat.o libc/libc.a
	ld -Ttext=40000000 --entry=main -o $@ $^

libc/libc.a: 
	make -C libc

.o: .c 
	$(CC) -c $^

clean:
	rm -f $(OBJ) *.o *.a
	make -C libc clean
