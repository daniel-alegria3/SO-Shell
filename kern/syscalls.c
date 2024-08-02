#include "types.h"
#include "list.h"
#include "lib.h"
#include "io.h"
#include "process.h"
#include "kmalloc.h"
#include "mm.h"
#include "schedule.h"
#include "file.h"
#include "syscalls.h"
#include "console.h"

void do_syscalls(int sys_num)
{
	u32 *stack_ptr;

	/* 
	 * Stocke dans stack_ptr le pointeur vers les registres sauvegardes.
	 * Les arguments sont transmis dans : ebx, ecx, edx, esi edi
	 * Le code de retour sera transmis dans %eax : stack_ptr[14]
	 */
	asm("mov %%ebp, %0": "=m"(stack_ptr):);

	/* print console */
	if (sys_num == 1) {
		char *u_str;
		int i;

		asm("mov %%ebx, %0": "=m"(u_str):);
		for (i = 0; i < 100000; i++);	/* temporisation */

		cli;
		printk(u_str);
		sti;
	}

	else if (sys_num == 2) {
		int status;

		cli;
		asm("mov %%ebx, %0": "=m"(status):);
		sys_exit(status);
	}

	else if (sys_num == 3) {
		char *path;

		cli;
		asm("mov %%ebx, %0": "=m"(path):);
		stack_ptr[14] = sys_open(path);
	}

	/* file read */
	else if (sys_num == 4) {
		char *u_buf;	/* buffer d'entree utilisateur */
		u32 fd;
		u32 bufsize;
		struct open_file *of;

		asm("	mov %%ebx, %0;	\
			mov %%ecx, %1;	\
			mov %%edx, %2": "=m"(fd), "=m"(u_buf), "=m"(bufsize):);

		//// printk("DEBUG: sys_read(): reading %d bytes on fd %d\n", bufsize, fd);

		of = current->fd;
		while (fd) {
			of = of->next;
			if (of == 0) {
				printk ("ERROR: sys_read(): invalid file descriptor\n");
				stack_ptr[14] = -1;
				return;
			}
			fd--;
		}

		if ((of->ptr + bufsize) > of->file->inode->i_size)
			bufsize = of->file->inode->i_size - of->ptr;

		memcpy(u_buf, (char *) (of->file->mmap + of->ptr), bufsize);
		of->ptr += bufsize;

		stack_ptr[14] = bufsize;
	}

	/* file close */
	else if (sys_num == 5) {
		u32 fd;
		struct open_file *of;

		asm("mov %%ebx, %0": "=m"(fd):);

		//// printk("DEBUG: sys_close(): process[%d] closing fd %d\n", current->pid, fd);

		of = current->fd;
		while (fd) {
			of = of->next;
			if (of == 0) {
				printk("ERROR: sys_close(): invalid file descriptor\n");
				return;
			}
			fd--;
		}

		kfree(of->file->mmap);
		of->file->mmap = 0;
		of->file = 0;
		of->ptr = 0;
	}

	else if (sys_num == 6) {
		char *u_buf;	/* buffer d'entree utilisateur */

		asm("mov %%ebx, %0": "=m"(u_buf):);

		stack_ptr[14] = sys_console_read(u_buf);
	}

	/* chdir */
	else if (sys_num == 7) {
		char *path;
		struct file *fp;

		asm("mov %%ebx, %0": "=m"(path):);

		if (!(fp = path_to_file(path))) {
			printk("can't chdir to %s\n", path);
			return;
		}

		if (!fp->inode)
			fp->inode = ext2_read_inode(fp->disk, fp->inum);

		if (!is_directory(fp)) {
			printk("%s is not a directory\n", path);
			return;
		}

		current->pwd = fp;
		//// printk("DEBUG: sys_chdir() to %s\n", current->pwd->name);
	}


	/* pwd */
	else if (sys_num == 8) {
		char *u_buf;
		int sz;
		struct file *fp;

		asm("mov %%ebx, %0": "=m"(u_buf):);

		if (current->pwd == f_root) {
			u_buf[0] = '/';
			u_buf[1] = 0;
			return;
		}

		fp = current->pwd;
		sz = strlen(fp->name) + 1;
		while (fp->parent != f_root) {
			fp = fp->parent;
			sz += (strlen(fp->name) + 1);
		}

		fp = current->pwd;
		u_buf[sz] = 0;

		while (sz > 0) {
			memcpy(u_buf + sz - strlen(fp->name), fp->name,
			       strlen(fp->name));
			sz -= (strlen(fp->name) + 1);
			u_buf[sz] = '/';
			fp = fp->parent;
		}

		//// printk("DEBUG: sys_pwd(): %s\n", current->pwd->name);
	}

	else if (sys_num == 9) {
		char *path;
		char **argv;

		asm("	mov %%ebx, %0	\n \
			mov %%ecx, %1"
			: "=m"(path), "=m"(argv) :);

		stack_ptr[14] = sys_exec(path, argv);
	}

	else if (sys_num == 10) {
		int  size;

		asm("mov %%ebx, %0": "=m"(size):);
		stack_ptr[14] = (u32) sys_sbrk(size);
	}

	/* debug */
	else if (sys_num == 100) {
		u32 *pa;
		asm("mov %%ebp, %0": "=m"(pa):);
		printk("eax: %p ecx: %p edx: %p ebx: %p\n", pa[12], pa[11], pa[10], pa[9]);
		printk("ds: %p esi: %p edi: %p\n", pa[4], pa[6], pa[5]);
		printk("ss: %p ebp: %p esp: %p\n", pa[17], pa[7], pa[16]);
		printk("cs: %p eip: %p\n", pa[14], pa[13]);
	}

	else
		printk("unknown syscall %d\n", sys_num);

	return;
}
