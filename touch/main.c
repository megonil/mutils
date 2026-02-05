#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utime.h>

#define error(fmt, ...)                                                   \
	fprintf(stderr, "error: " fmt, __VA_ARGS__);                          \
	exit(1);

#define printexit(...)                                                    \
	printf(__VA_ARGS__);                                                  \
	exit(0);

typedef enum
{
	FlagDoNotCreateFiles,
} Flags;

typedef int flag_t;

flag_t flags = 0;

#define getflag(fl) flags & (1 << fl)
#define setflag(fl) flags |= (1 << fl)

int
file_exists(char* filename)
{
	struct stat buf;
	return stat(filename, &buf) == 0;
}

void _Noreturn help()
{
	printexit("Help:\n\t-c - do not create any files\n\t-h - show this "
			  "message\n");
}

void _Noreturn usage(const char* program)
{
	printexit("Usage: %s [OPTION]... FILE...\n", program);
}

void
touch(const char* filename)
{
	utime(filename, NULL); // NULL means set current time
}

int
main(int argc, char* argv[])
{
	char c;
	while ((c = getopt(argc, argv, "c::h::")) != -1)
	{
		switch (c)
		{
		case 'c': setflag(FlagDoNotCreateFiles); break;
		case 'h': help();
		}
	}

	if (optind >= argc)
	{
		usage(argv[0]);
	}

	for (int i = optind; i < argc; i++)
	{
		if (file_exists(argv[i]))
		{
			touch(argv[i]);
		}
		else if (getflag(FlagDoNotCreateFiles))
		{
			error("file %s doest not exists\n", argv[i]);
		}
		else
		{
			FILE* file =
				fopen(argv[i], "a"); // "a" means create if not exists(and
									 // some other things)
			if (file == NULL)
			{
				perror("could not create file:");
				// error("cannot create file %s", argv[i]);
			}
		}
	}
	return 0;
}
