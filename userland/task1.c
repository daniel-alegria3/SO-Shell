#include "libc/libc.h"
#include "libc/syscalls.h"
#include "libc/malloc.h"


int main(void)
{
	char **av;

	av = (char**) malloc(sizeof(char*) * 3);
	av[0] = (char*) malloc(4);
	strcpy(av[0], "cat");

	av[1] = (char*) malloc(8);
	strcpy(av[1], "foo.txt");
	av[2] = (char*) 0;
	exec(av[0], av);

	free(av[1]);
	av[1] = (char*) malloc(13);
	strcpy(av[1], "/tmp/bar.txt");
	exec(av[0], av);

	exit(0);

	return 0;
}

