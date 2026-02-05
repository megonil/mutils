#include <linux/limits.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define println(fmt, ...) printf(fmt "\n" __VA_ARGS__)
#define println_exit(exit_code, fmt, ...)                                 \
	println(fmt, __VA_ARGS__);                                            \
	exit(exit_code)

#define mode777() S_IRWXU | S_IRWXG | S_IRWXO

typedef enum
{
	FlagParents
} Flags;

typedef int flag_t;

flag_t flags = 0;
#define setflag(fl) flags |= (1 << fl)
#define getflag(fl) flags & (1 << fl)

void _Noreturn usage(const char* program)
{
	println_exit(1, "Usage: %s [OPTION]... DIR...", , program);
}

void _Noreturn help()
{
	println_exit(1, "Help:\n\t-p - no error if existing, make parent "
					"directories as needed");
}

char initial_cwd[PATH_MAX];

int
main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");
	if (getcwd(initial_cwd, sizeof(initial_cwd)) == NULL)
	{
		println_exit(1, "error: cannot get cwd");
	}

	char c;
	while ((c = getopt(argc, argv, "p::h::")) != -1)
	{
		switch (c)
		{
		case 'p': setflag(FlagParents); break;
		case 'h': help();
		}
	}

	if (optind >= argc)
	{
		usage(argv[0]);
	}

	for (int i = optind; i < argc; ++i, chdir(initial_cwd))
	{
		char* dirname = argv[i];
		if (memchr(dirname, '/',
				   strlen(dirname))) // check if dirname contains /
		{
			if (getflag(FlagParents))
			{
				if (mkdir(dirname, mode777()) != 0) // 0 means success
				{
					perror("mkdir");
				}; // with 777 mode
			}
			else
			{
				for (char* dir = strtok(dirname, "/"); dir != NULL;
					 dir	   = strtok(NULL, "/"))
				{
					mkdir(dir, mode777());
					chdir(dir); // hack
				}
			}
		}
		else
		{
			mkdir(dirname, mode777());
		}
	}

	return 0;
}
