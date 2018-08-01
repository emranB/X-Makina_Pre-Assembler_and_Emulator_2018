#pragma once

#define SUPPORTED_CMDS_CNT 10	/* Count of supported commands */
#define MAX_CMD_LEN 256			/* Max size of user entered input commands */
#define DELIMITERS " \t\n"		/* Delimiters used to tokenize input commands */

/* List of supported commands */
char SUPPORTED_CMDS[SUPPORTED_CMDS_CNT][MAX_CMD_LEN] = {
	"HELP",
	"SET_REG Rx Val",
	"VIEW_REG Rx",
	"SET_MEM Addr Val",
	"VIEW_MEM Addr",
	"RUN_MACHINE",
	"SET_CLK Val",
	"VIEW_CLK",
	"USE_CACHE [0=DIR_MAP | 1=ASC | 2=HYB] [0=WT | 1=WB]",
	"QUIT"
};
