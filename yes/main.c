#include <locale.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void
loop(char* buf, size_t n)
{
	while (write(STDOUT_FILENO, buf, n) > 0)
	{
	}

	perror("standard output");
}

int
main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");

	char** operands	   = argv + 1;
	char** operand_llm = argv + argc;
	char*  buf;
	if (operands == operand_llm)
	{
		buf = "y\n";
		loop(buf, sizeof(buf));
		return 1;
	}

	size_t bufalloc = 0;
	char** operandp = operands;
	do
	{
		size_t operand_len = strlen(*operandp);
		bufalloc += operand_len + 1; // 1 for \0
	} while (++operandp < operand_llm);

	if (bufalloc <= BUFSIZ / 2)
	{
		bufalloc = BUFSIZ;
	}
	buf			   = (char*) malloc(bufalloc);
	size_t bufused = 0;
	operandp	   = operands;
	do
	{
		size_t operand_len = strlen(*operandp);
		bufused += operand_len;
		buf[bufused++] = ' ';
	} while (++operandp < operand_llm);

	buf[bufused - 1] = '\n';

	size_t copysize = bufused;
	for (size_t copies = bufalloc / copysize; --copies;)
	{
		memcpy(buf + bufused, buf, copysize);
		bufused += copysize;
	}

	loop(buf, bufused);
	free(buf);
	return 1;
}
