#include "cli.h"

#include <string.h>

static char lineBuffer[100];
static int lineIdx = 0;

static void print(const char* s);
static int isspace(char c);
static char* trim(char* str);

void cliInit()
{
}
void cliProcessChar(char c)
{
	lineBuffer[lineIdx] = c;

	if (c == 127) // backspace
	{
		if (lineIdx)
		{
			cliPutChar(0x08);
			cliPutChar(' ');
			cliPutChar(0x08);
			lineIdx--;
		}
	}
	else if (c == '\n' || c == '\r')
	{
		lineBuffer[lineIdx] = 0;
		char* s = trim(lineBuffer);

		lineIdx = 0;

		char *sp = strchr(s, ' ');
		if (sp)
		{
			*sp = 0;
		}

		cliPutChar('\r');
		cliPutChar('\n');
		int i;
		for (i = 0; cliCommands[i].cmd; i++)
		{
			if (strcmp(s, cliCommands[i].cmd) == 0)
			{
				cliCommands[i].handler(sp + 1);
				s = 0;
				break;
			}
		}
		if (s && strlen(s))
		{
			print("invalid command\r\n");
		}
		print("> ");
	}
	else
	{
		cliPutChar(c);
		lineIdx++;
		if (lineIdx == sizeof(lineBuffer))
			lineIdx = 0;
	}
}

void print(const char* s)
{
	while (*s)
		cliPutChar(*s++);
}

int isspace(char c)
{
	return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
char* trim(char* str)
{
  char* end;

  while (isspace(*str)) str++;

  if (*str == 0)
    return str;

  end = str + strlen(str) - 1;
  while (end > str && isspace(*end)) end--;

  *(end + 1) = 0;

  return str;
}
