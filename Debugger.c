#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Emulator.h"
#include "Debugger.h"


/* 
	Set breakpoint in System Clock, during Fetch-Decode-Execute cycle.
	Default: 1000
*/
unsigned long SYS_CLK_BREAKPOINT = 200;

/* Register File */
extern signed short REG_FILE[];


void InitDebugger(void) {

	char cmd[MAX_CMD_LEN];		/* This holds the command entered by user */
	char *token;				/* Tokenize input command */

	/* Read command entered by user */
	while (fgets(cmd, MAX_CMD_LEN, stdin)) {
		fflush(stdout);			/* Flush any buffers */
		token = strtok(cmd, DELIMITERS);

		if (_strcmpi(cmd, "HELP") == 0) { /* Print out all usable commands */
			printf("-------------------------\n");
			printf("SUPPORTED COMMANDS:\n");
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
			printf("Enter: Reg_Num Reg_Value - ");
			int reg_num; unsigned short reg_val;
			scanf("%d %d", &reg_num, &reg_val);
			if (reg_num >= 0 && reg_num <= 7 && reg_val >= 0 && reg_val <= 0xFFFF)
				REG_FILE[reg_num] = reg_val;
			else
				printf("Invalid Values\n");
		}
		else if (_strcmpi(cmd, "VIEW REG") == 0) {
			printf("Enter: Reg_Num - ");
			int reg_num;
			scanf("%d", &reg_num);
			if (reg_num >= 0 && reg_num <= 7)
				printf("R%d = %4x\n", reg_num, REG_FILE[reg_num]);
			else
				printf("Invalid Value\n");
		}
		else if (_strcmpi(cmd, "SET MEM") == 0) {

		}
		else if (_strcmpi(cmd, "RUN MACHINE") == 0) {
			RunMachine();
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