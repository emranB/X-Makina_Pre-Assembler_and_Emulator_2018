#pragma once

#define FALSE 0
#define TRUE 1
#define R6 6							/* Register 6 - PSW */

extern struct PSW_BITS* PSWptr;			/* Structure of PSW externed from cpu.c, to be initialized */
extern signed short REG_FILE[];			/* Register file externed from cpu.c, to be initialized */
extern unsigned long SYS_CLK;
extern union MEM_OLAY MEM;				/* Emulation of Memory */

extern void InitDebugger(void);			/* Transfer control to Debugger */
extern int XmeLoader(char *);			/* Loader function to load .xme file */
extern void RunMachine(void);			/* Initiate Emulation for CPU */
