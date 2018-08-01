/*
Author: Emran Billah (em843554@dal.ca)
	Main.c
	Main program to start execution of Instructions
*/
#include <stdio.h>
#include "Emulator.h"
#include "Loader.h"
#include "cpu.h"
#include "memory.h"


struct PSW_BITS* PSWptr;				/* Initialzing PSW */
extern signed short REG_FILE[];			/* Register File */
unsigned long SYS_CLK;					/* Initializeing System Clock */
extern void InitMemory(void);			/* Initialize Memory components */

/*
	Initializing Primary Memory.
	Array of Bytes or Words.
	Range: 0 to 65535 Bytes, or, 0 to 32768 Words
*/
union MEM_OLAY MEM;

/* Properties of Cache being used (set by the Debugger) */
struct CACHE_PROPERTIES CacheProperties;

/* Initialize 32 Cache Lines */
struct CACHE_LINE CACHE[MAX_CACHE_SIZE];

/* Initialize PSW, Devices and Clock */
void init(void) {
	/* Initializing Devices */
	InitDevices();

	/* Initializing Memory Components */
	InitMemory();

	/* Setting PSW pointer to location of R6 in REG_FILE */
	PSWptr = (struct PSW_BITS *) &REG_FILE[PSW];

	/* Initializing System Clock to 0 */
	SYS_CLK = 0;
}


int main(int argc, char* argv[]) {


	/* Test Variables */
	argc = 2;
	argv[0] = "C:\\Users\\Homepc\\Desktop\\Summer 2018\\Computer Architecture\\Assignment 2\\code\\Emulator\\Debug\\Emulator.exe";
	argv[1] = "C:\\Users\\Homepc\\Desktop\\Summer 2018\\Computer Architecture\\Assignment 2\\Code\\Emulator\\Debug\\test.xme";
	/******************/


	switch (argc) {
	case 1:			/* No file dragged and dropped or opened by program */
		printf("Warning: No input '.xme' file detected!\n");
		break;
	case 2:
		if (XmeLoader(argv[1]) != LOADER_SUCCESS) /* If loader failed to process input file */
			printf("Warning: Failed to load input '.xme' file!\n\n");
		else
			printf("Successfully loaded input '.xme' file!\n\n");
		break;
	}

	/* Initialize PSW, Devices and Clock */
	init();

	/* Run Debugger */
	printf("- What would you like to do? (Type HELP for support)\n");
	InitDebugger();

	//RunMachine();

	CloseDevices();

	printf("\n-------------------\nAll Done\n-------------------\n");
	getchar();

	return 0;
}