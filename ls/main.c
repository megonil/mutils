#include <dirent.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define toerror(...) fprintf(stderr, __VA_ARGS__)
#define toerrorln(fmt, ...) fprintf(stderr, fmt "\n", __VA_ARGS__)

#define error(...)                                                        \
	toerror("error: %s", __VA_ARGS__);                                    \
	exit(1)

#define errorln(...)                                                      \
	toerrorln("error: %s", __VA_ARGS__);                                  \
	exit(1)

#define merror(s)                                                         \
	perror(s);                                                            \
	exit(EXIT_FAILURE)

#define println(fmt, ...) printf(fmt "\n", __VA_ARGS__)

typedef enum
{
	FlagPerms,
	FlagPerms2,
	FlagShowDots,
	FlagShowDotsExclude
} flag;

typedef uint flag_t;
flag_t		 flags = 0;

#define setflag(fl) flags = (1 << fl) | flags
#define getflag(fl) (flags >> (fl)) & 1

int
nodots(const struct dirent* entry)
{
	return entry->d_name[0] != '.'; // return nonzero
}

int
showdotsp(const struct dirent* entry)
{
	if (strcmp(entry->d_name, ".") == 0 ||
		strcmp(entry->d_name, "..") == 0)
	{
		return 0;
	}
	return 1; // show all others
}

void
usage(char* argv[])
{
	toerrorln("Usage %s: [-d] [-D] [-p] [-P] [-h] [path]", argv[0]);
	exit(EXIT_FAILURE);
}

void _Noreturn help()
{
	printf("Help:\n\t-d - show dotfiles\n\t-D - show dotfiles excluding . "
		   "and ..\n\t-p - see permissions of "
		   "entries\n\t-P - see permissions in 0777 form\n\t-h - "
		   "show this message\n");
	exit(EXIT_SUCCESS);
}

void
namelist_print(struct dirent** namelist, int n)
{
	for (int i = 0; i < n; i++)
	{
		printf("%s ", namelist[i]->d_name);
		free(namelist[i]);
	}

	printf("\n");
}

void
pprint(mode_t mode)
{
	printf("%c", S_ISDIR(mode) ? 'd' : '-');

	// User
	printf("%c", (mode & S_IRUSR) ? 'r' : '-');
	printf("%c", (mode & S_IWUSR) ? 'w' : '-');
	printf("%c", (mode & S_IXUSR) ? 'x' : '-');

	// Group
	printf("%c", (mode & S_IRGRP) ? 'r' : '-');
	printf("%c", (mode & S_IWGRP) ? 'w' : '-');
	printf("%c", (mode & S_IXGRP) ? 'x' : '-');

	// Others
	printf("%c", (mode & S_IROTH) ? 'r' : '-');
	printf("%c", (mode & S_IWOTH) ? 'w' : '-');
	printf("%c", (mode & S_IXOTH) ? 'x' : '-');
	putc(' ', stdout); // do not forget the space
}

void
pprint2(mode_t mode)
{
	printf("%o %s ", mode & (S_IRWXU | S_IRWXG | S_IRWXO),
		   S_ISDIR(mode) ? "dir" : "file");
	// printf("%c", S_ISDIR(mode) ? 'd' : '-');
	//
	// // User
	// printf("%c", (mode & S_IRUSR) ? 'r' : '-');
	// printf("%c", (mode & S_IWUSR) ? 'w' : '-');
	// printf("%c", (mode & S_IXUSR) ? 'x' : '-');
	//
	// // Group
	// printf("%c", (mode & S_IRGRP) ? 'r' : '-');
	// printf("%c", (mode & S_IWGRP) ? 'w' : '-');
	// printf("%c", (mode & S_IXGRP) ? 'x' : '-');
	//
	// // Others
	// printf("%c", (mode & S_IROTH) ? 'r' : '-');
	// printf("%c", (mode & S_IWOTH) ? 'w' : '-');
	// printf("%c", (mode & S_IXOTH) ? 'x' : '-');
}

void
fpprint(const char* base, const char* entry_name)
{
	size_t fullpath_len = strlen(base) + strlen(entry_name) + 2;
	char*  fullpath =
		malloc(fullpath_len *
			   sizeof(char)); // 2: 1 for the '/' and 1 for the '\0'
	snprintf(fullpath, fullpath_len, "%s/%s", base, entry_name);

	struct stat buf;
	if (stat(fullpath, &buf) == 0)
	{
		if (getflag(FlagPerms2))
		{
			pprint2(buf.st_mode);
		}
		else
		{
			pprint(buf.st_mode);
		}
	}

	else
	{
		merror("stat");
	}

	free(fullpath);
}

void
namelist_perms_print(struct dirent** namelist, int n)
{
	char cwd[PATH_MAX];
	getcwd(cwd, sizeof(cwd));
	for (int i = 0; i < n; i++)
	{
		fpprint(cwd, namelist[i]->d_name);
		printf("%s\n", namelist[i]->d_name);
		free(namelist[i]);
	}
}

void
mylsscan(const char* dir)
{
	struct dirent** namelist;

	// n is the quantiity of namelist
	int n;
	if (getflag(FlagShowDots))
	{
		n = scandir(
			dir, &namelist, NULL,
			alphasort); // NULL because it will print with dots by default
	}
	else if (getflag(FlagShowDotsExclude))
	{
		n = scandir(dir, &namelist, showdotsp, alphasort);
	}
	else
	{
		n = scandir(dir, &namelist, nodots,
					alphasort); // by default show with no dots
	}

	if (n == -1)
	{
		merror("scandir");
	}

	if (getflag(FlagPerms) || getflag(FlagPerms2))
	{
		namelist_perms_print(namelist, n);
	}
	else
	{
		namelist_print(namelist, n);
	}

	free(namelist);
}

void
scanthis()
{
	mylsscan(".");
}

int
main(int argc, char* argv[])
{
	char c;
	while ((c = getopt(argc, argv, "p::d::D::P::h::")) != -1)
	{
		switch (c)
		{
		case 'p': setflag(FlagPerms); break;
		case 'd': setflag(FlagShowDots); break;
		case 'D': setflag(FlagShowDotsExclude); break;
		case 'P': setflag(FlagPerms2); break;
		case 'h': help();
		case '?': errorln("unknown argument"); return EXIT_SUCCESS;
		}
	}
	// here you can provide some dir
	scanthis();

	return EXIT_SUCCESS;
}
