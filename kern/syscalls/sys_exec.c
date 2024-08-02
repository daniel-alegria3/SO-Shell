#include "types.h"
#include "list.h"
#include "io.h"
#include "lib.h"
#include "file.h"
#include "process.h"

int sys_exec(char *path, char **argv)
{
	char **ap;
	int argc, pid;
	struct file *fp;

	if (!(fp = path_to_file(path))) {
		printk("DEBUG: sys_exec(): %s: command not found\n", path);
		return -1;
	}

	if (!fp->inode)
		fp->inode = ext2_read_inode(fp->disk, fp->inum);

	ap = argv;
	argc = 0;
	while (*ap++) 
		argc++;

	cli;
	pid = load_task(fp->disk, fp->inode, argc, argv);
	sti;

	return pid;
}
