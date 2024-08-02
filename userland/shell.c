#include "libc/libc.h"
#include "libc/syscalls.h"
#include "libc/malloc.h"
#include "dir.h"


#define PROMPT	"minishell> "


int main(void)
{
	struct directory_entry *dir;
	char buf[512], out[512]; 
	char *beg_p, *end_p, **av;
	int fd, count, ac;
	int i;

	while(1) {
		console_write(PROMPT);

		for(i=0;i<512;i++)
			buf[i]=0;
		console_read(buf);

		beg_p = buf;
		while (*beg_p == ' ' || *beg_p == '\t')		/* enleve les blancs */
			beg_p++;

		/* exit */
		if (strncmp("exit", beg_p, 4) == 0) {
			exit(0);
		}

		/* ls */
		else if (strncmp("ls", beg_p, 2) == 0) {
			fd = open(".");
			count = read(fd, buf, sizeof(buf));
			dir = (struct directory_entry*) buf;

			while(count>0 && dir->inode) {
				memcpy(out, &dir->name, dir->name_len);
				out[dir->name_len] = 0;
				
				console_write(out);
				console_write("\n");

				count -= dir->rec_len;
				dir = (struct directory_entry*) ((char*) dir + dir->rec_len);
			}
			close(fd);
		}

		/* cd */
		else if (strncmp("cd", beg_p, 2) == 0) {
			beg_p += 2;
			while (*beg_p == ' ' || *beg_p == '\t') 
				beg_p++;

			end_p = beg_p;
	       		while (*end_p && *end_p != '\n' && *end_p != ' ' && *end_p != '\t') 
				end_p++;
			*end_p = 0;

			chdir(beg_p);
		}

		/* pwd */
		else if (strncmp("pwd", beg_p, 3) == 0) {
			pwd(buf);
			console_write(buf);
			console_write("\n");
		}

		/* help */
		else if (strncmp("help", beg_p, 4) == 0) {
			console_write("usage:\n\tcd\n\texit\n\tls\n\tpwd\n");
		}

		/* exec command */
		else {
			/* Comptage du nombre de parametres */
			ac = 1;
			end_p = beg_p;
       			while (*end_p && *end_p != '\n') {
		       		while (*end_p && *end_p != '\n' && *end_p != ' ' && *end_p != '\t') 
					end_p++;
				ac++;
				while (*end_p == ' ' || *end_p == '\t') 
					end_p++;
			}

			if (ac>1) {
				av = (char**) malloc(sizeof(char*) * ac);

				beg_p = end_p = beg_p;
				for(i=0 ; i<(ac-1) ; i++) {
					while (*end_p == ' ' || *end_p == '\t')
						end_p++;
					beg_p = end_p;
	
       					while (*end_p && *end_p != '\n' && *end_p != ' ' && *end_p != '\t')
       						end_p++;
		
       					av[i] = (char*) malloc(end_p - beg_p + 1);
       					strncpy(av[i], beg_p, end_p - beg_p);
			       		av[i][end_p - beg_p] = 0;
				}
				av[i] = (char*) 0;
	
				exec(av[0], av);

				for(i=0 ; i<(ac-1) ; i++) 
					free(av[i]);
				free((char*) av);
			}
		}
	}

	exit(0);

	return 0;
}

