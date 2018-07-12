#pragma once

#define FALSE		0
#define TRUE		1
#define PSW			6					/* Register 6 - PSW */

extern union MEM_OLAY MEM;				/* Emulation of Memory */

extern void InitDevices(void);			/* Initialize devices */
extern void CloseDevices(void);			/* Close Devices */
extern void InitDebugger(void);			/* Transfer control to Debugger */
extern int XmeLoader(char *);			/* Loader function to load .xme file */
extern void RunMachine(void);			/* Initiate Emulation for CPU */
