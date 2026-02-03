#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define cannotget()                                                       \
	fprintf(stderr, "error: cannot get pwd/cwd");                         \
	exit(1);

#define printexit(...)                                                    \
	printf(__VA_ARGS__);                                                  \
	exit(0)

void _Noreturn print_cwd()
{
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) == NULL)
	{
		cannotget()
	}

	printexit("%s\n", cwd);
}

void _Noreturn print_pwd()
{
	char* pwd = getenv("PWD");
	if (pwd == NULL)
	{
		cannotget();
	}

	printexit("%s\n", pwd);
}

void _Noreturn help()
{
	printf("Help:\n\t-c - use cwd\n\t-p - use pwd env variable\n\t-h - "
		   "print this message\n");
	exit(0);
}

int
main(int argc, char* argv[])
{
	char c;
	while ((c = getopt(argc, argv, "c::p::h::")) != -1)
	{
		switch (c)
		{
		case 'c': print_cwd();
		case 'p': print_pwd();
		case 'h': help();
		}
	}

	print_pwd(); // default
	return 0;
}
