#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Emulator.h"
#include "Debugger.h"

void InitDebugger(void) {

	char cmd[MAX_CMD_LEN];		/* This holds the command entered by user */
	char *token;				/* Tokenize input command */

	/* Read command entered by user */
	while (fgets(cmd, MAX_CMD_LEN, stdin)) {
		fflush(stdout);			/* Flush any buffers */
		token = strtok(cmd, DELIMITERS);

		if (_strcmpi(cmd, "HELP") == 0) { /* Print out all usable commands */
			printf("-------------------------\n");
			printf("SUPPOERTED COMMANDS:\n");
			printf("-------------------------\n");
			int i = 0;
			for (i = 0; i < SUPPORTED_CMDS_CNT; i++) {
				printf("%d:	%s\n", i, SUPPORTED_CMDS[i]);
			}
			printf("-------------------------\n");
		}
		else if (_strcmpi(cmd, "VIEW MEM") == 0) {

		}
		else if (_strcmpi(cmd, "SET MEM") == 0) {

		}
		else if (_strcmpi(cmd, "SET REG") == 0) {

		}
		else if (_strcmpi(cmd, "VIEW REG") == 0) {

		}
		else if (_strcmpi(cmd, "SET MEM") == 0) {

		}
		else if (_strcmpi(cmd, "RUN MACHINE") == 0) {

		}
		else if (_strcmpi(cmd, "QUIT") == 0) {
			return 0;
		}
		else {
			printf("COMMAND NOT SUPPORTED!\n");
		}

		printf("\n");
	}
}