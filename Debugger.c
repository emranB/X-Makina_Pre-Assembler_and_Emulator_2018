/*
	Debugger Implementation - Developer Tools
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Emulator.h"
#include "Debugger.h"
#include "memory.h"
#include "cpu.h"


/* 
	Set breakpoint in System Clock, during Fetch-Decode-Execute cycle.
	Default: 1000
*/
unsigned long SYS_CLK_BREAKPOINT = 500;

/* Register File */
extern signed short REG_FILE[];

/* Memory */
extern union MEM_OLAY MEM;

/* Properties of Cache being used (set by the Debugger) */
extern struct CACHE_PROPERTIES CacheProperties;

/* Properties of Cache being used (set by the Debugger) */
struct CACHE_LINE CACHE[MAX_CACHE_SIZE];

/* Starting address from S9 Record */
extern unsigned short StartingAddr;

extern void init(void);


void InitDebugger(void) {

	char cmd[MAX_CMD_LEN];		/* This holds the command entered by user */
	char *tok1 = (char *)malloc(MAX_CMD_LEN);
	char *tok2 = (char *)malloc(MAX_CMD_LEN);
	char *tok3 = (char *)malloc(MAX_CMD_LEN);

	/* Read command entered by user */
	while (fgets(cmd, MAX_CMD_LEN, stdin) != NULL) {

		tok1 = strtok(cmd, DELIMITERS);

		if (_strcmpi(tok1, "HELP") == 0) { /* Print out all usable commands */
			printf("-------------------------\n");
			printf("SUPPORTED COMMANDS:\n");
			printf("-------------------------\n\n");
			int i = 0;
			for (i = 0; i < SUPPORTED_CMDS_CNT; i++) {
				printf("%d:	%s\n", i, SUPPORTED_CMDS[i]);
			}
			printf("-------------------------\n\n");
		}
		else if (_strcmpi(tok1, "VIEW_REG_FILE") == 0) {
			int i;
			printf("-------------------------\n");
			for (i = 0; i < 8; i++) {
				printf("REG(%d) = %04x = %d\n", i, REG_FILE[i] & 0xFFFF, REG_FILE[i] & 0xFFFF);
			}
			printf("-------------------------\n\n");
		}
		else if (_strcmpi(tok1, "VIEW_CLK") == 0) {
			printf("-------------------------\n");
			printf("SYS_CLK = %d\n", SYS_CLK_BREAKPOINT);
			printf("-------------------------\n\n");
		}
		else if (_strcmpi(tok1, "RUN") == 0) {
			//init();
			//REG_FILE[PC] = StartingAddr;
			RunMachine();
		}
		else if (_strcmpi(tok1, "DISCARD_CACHE") == 0) {
			CacheProperties.ACTIVE = FALSE;
		}
		else if (_strcmpi(tok1, "SHOW_CACHE_PROPS") == 0) {
			printf("-------------------------\n");
			printf("**Cache Properties:**\n\n");
			printf("Active		= %s\n", CacheProperties.ACTIVE ? "Active" : "Inactive");
			printf("Org Type	= %d\n", CacheProperties.ORG_TYPE);
			printf("Replace Policy	= %d\n", CacheProperties.REPL_POL);
			printf("-------------------------\n\n");
		}
		else if (_strcmpi(tok1, "QUIT") == 0) {
			return 0;
		}
		else {
			if (tok2 = strtok(NULL, DELIMITERS)) {
				if (_strcmpi(tok1, "SET_CLK") == 0) {
					SYS_CLK_BREAKPOINT = atoi(tok2);
				}
				else {
					if (tok3 = strtok(NULL, DELIMITERS)) {
						if (_strcmpi(tok1, "SET_REG") == 0) {
							REG_FILE[atoi(tok2)] = tok3;
						}
						else if (_strcmpi(tok1, "SET_MEM") == 0) {
							MEM.MEM_BYTE[atoi(tok2)] = tok3;
						}
						else if (_strcmpi(tok1, "VIEW_MEM") == 0) {
							int i; int j; tok2 = atoi(tok2); tok3 = atoi(tok3);
							int page_size = 8;
							int segments = ((tok3 - tok2) % page_size) ? (((tok3 - tok2) / page_size) + 1) : ((tok3 - tok2) / page_size);
							printf("------------------------------------------------------------------\n");
							printf("\n**Printing Mem contents starting at:**\n\n");
							for (j = 0; j < (segments); j++) {
								for (i = (tok2 + page_size*j); i <= (tok2 + page_size*j + page_size); i++) {
									printf("%04x	", i & 0xFFFF);
								}
								for (i = (tok2 + page_size*j); i <= (tok2 + page_size*j + page_size); i++) {
									printf("%02x	", MEM.MEM_BYTE[i] & 0xFF);
								}
								printf("------------------------------------------------------------------\n");
							}
						}
						else if (_strcmpi(tok1, "USE_CACHE") == 0) {
							CacheProperties.ACTIVE   = TRUE;
							CacheProperties.ORG_TYPE = atoi(tok2);
							CacheProperties.REPL_POL = atoi(tok3);
						}
						else if (_strcmpi(tok1, "VIEW_CACHE_MEM") == 0) {
							printf("------------------------------------------------------------------\n");
							printf("**Printing Cache mem contents from %d**\n", atoi(tok2));
							printf("\n");

							int i; int j; int tok_2 = atoi(tok2); int tok_3 = atoi(tok3);
							int page_size = 8;
							int segments = ((tok_3 - tok_2) % page_size) ? (((tok_3 - tok_2) / page_size) + 1) : ((tok_3 - tok_2) / page_size);
							for (j = 0; j < (segments); j++) {
								for (i = (tok_2 + page_size*j); i <= (tok_2 + page_size*j + page_size); i++) {
									printf("%d	", i & 0xFFFF);
								}
								printf("\n");
								for (i = (tok_2 + page_size*j); i <= (tok_2 + page_size*j + page_size); i++) {
									printf("%04x	", CACHE[i].Address & 0xFFFF);
								}
								printf("\n");
								for (i = (tok_2 + page_size*j); i <= (tok_2 + page_size*j + page_size); i++) {
									printf("%04x	", CACHE[i].Data & 0xFFFF);
								}
								printf("\n");
								for (i = (tok_2 + page_size*j); i <= (tok_2 + page_size*j + page_size); i++) {
									printf("DB %d	", CACHE[i].DirtyBit & 0x0001); /* Dirty Bit */
								}
								printf("\n");
								printf("------------------------------------------------------------------\n");
							}
						}
					}
				}
			}
		}

		fflush(stdout);			/* Flush all buffers */
		//free(tok1);
		//free(tok2);
		//free(tok3);

		printf("\n\n");
		printf("--> What would you like to do? (Type HELP for support)\n");
		printf("\n\n");

	}
}