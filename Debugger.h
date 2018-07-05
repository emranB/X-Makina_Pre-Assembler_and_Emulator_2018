#pragma once

#define SUPPORTED_CMDS_CNT 8	/* Count of supported commands */
#define MAX_CMD_LEN 256			/* Max size of user entered input commands */
#define DELIMITERS " \t\n"		/* Delimiters used to tokenize input commands */

/* List of supported commands */
char SUPPORTED_CMDS[SUPPORTED_CMDS_CNT][MAX_CMD_LEN] = {
	"HELP",
	"VIEW MEM",
	"SET MEM",
	"SET REG",
	"VIEW REG",
	"SET MEM",
	"RUN MACHINE",
	"QUIT"
};
