#include "cli.h"

#include <string.h>

static char cli_lineBuffer[100];
static int cli_lineIdx = 0;

void cli_print(char *s);
int myisspace(char c);
char *trim(char *str);

void cliInit()
{
}
void cliProcessChar(char c)
{
	cli_lineBuffer[cli_lineIdx] = c;

	if (c == 127) // backspace
	{
		if (cli_lineIdx)
		{
			cliPutChar(0x08);
			cliPutChar(' ');
			cliPutChar(0x08);
			cli_lineIdx--;
		}
	}
	else if (c == '\n' || c == '\r')
	{
		cli_lineBuffer[cli_lineIdx] = 0;
		char *s = trim(cli_lineBuffer);

		cli_lineIdx = 0;

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
			cli_print("invalid command\r\n");
		}
		cli_print("> ");
	}
	else
	{
		cliPutChar(c);
		cli_lineIdx++;
		if (cli_lineIdx == 100)
			cli_lineIdx = 0;
	}
}

// priv
void cli_print(char *s)
{
	while (*s)
		cliPutChar(*s++);
}

int myisspace(char c)
{
	return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
char *trim(char *str)
{
  char *end;

  while (myisspace(*str)) str++;

  if (*str == 0)
    return str;

  end = str + strlen(str) - 1;
  while (end > str && myisspace(*end)) end--;

  *(end + 1) = 0;

  return str;
}
