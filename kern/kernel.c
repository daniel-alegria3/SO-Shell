#include "types.h"
#include "list.h"
#include "lib.h"
#include "gdt.h"
#include "screen.h"
#include "io.h"
#include "idt.h"
#include "mm.h"
#include "process.h"
#include "boot.h"
#include "disk.h"
#include "kmalloc.h"
#include "ext2.h"
#include "elf.h"
#include "file.h"

void ok_msg(void)
{
	kX = 40;
	kattr = 0x0A;
	printk("OK\n");
	kattr = 0x07;
}

void kmain(struct multiboot_info *mbi)
{
	printk("Pepin is booting...\n");
	printk("RAM detected : %uk (lower), %uk (upper)\n", mbi->low_mem, mbi->high_mem);

	cli;

	/* Initialisation de la GDT et des segments */
	printk("Loading GDT");
	init_gdt();
	asm("	movw $0x18, %%ax \n \
		movw %%ax, %%ss \n \
		movl %0, %%esp"::"i" (KERN_STACK));
	ok_msg();

	printk("Loading IDT");
	init_idt();
	ok_msg();

	printk("Configure PIC");
	init_pic();
	ok_msg();

	printk("Loading Task Register");
	asm("	movw $0x38, %ax; ltr %ax");
	ok_msg();

	printk("Enabling paging");
	init_mm(mbi->high_mem);
	ok_msg();

	hide_hw_cursor();

	{
		struct partition *p1;
		struct disk *hd;
		struct file *fp;
		struct terminal tty1;

		/* 
		 * Lecture des informations relatives a la premiere partition
		 */
		p1 = (struct partition *) kmalloc(sizeof(struct partition));
		disk_read(0, 0x01BE, (char *) p1, 16);
		printk("Partition found on block: %d, size: %d blocks, bootable: %x\n",
		     p1->s_lba, p1->size, p1->bootable);

		/*
		 * On alloue une structure pour stocker les informations
		 * relative au filesystem
		 */
		hd = ext2_get_disk_info(0, p1);

		/* 
		 * Montage du disque sur la racine 
		 */
		printk("Mount root partition (ext2fs)");
		f_root = init_root(hd);
		ok_msg();

		/* Initialise le terminal */
		tty1.pread = tty1.pwrite = 0;
		current_term = &tty1;

		/* 
		 * Initialise le thread kernel 
		 */
		current = &p_list[0];
		current->pid = 0;
		current->state = 1;
		current->regs.cr3 = (u32) pd0;
		current->console = 0;
		current->pwd = f_root;

		/* Lancement du shell */
		fp = path_to_file("/bin/sh");
		fp->inode = ext2_read_inode(hd, fp->inum);
		load_task(hd, fp->inode, 0, 0);

		kattr = 0x47;
		printk("Interrupts are enable. System is ready !\n\n");
		kattr = 0x07;

		sti;

		while (1) {
			if (n_proc == 0) {
				cli;
				load_task(hd, fp->inode, 0, 0);
				sti;
			}
		}
	}

}
