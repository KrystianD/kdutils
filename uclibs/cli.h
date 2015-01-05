#ifndef __CLI_H__
#define __CLI_H__

#include <stdint.h>

typedef void (*cli_handler_t)(char *args);

typedef struct
{
	const char* cmd;
	cli_handler_t handler;
} cli_cmd_t;

extern cli_cmd_t cliCommands[];
extern void cliPutChar(char c);

void cliInit();
void cliProcessChar(char c);

#endif
