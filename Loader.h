#pragma once
#define LINE_LEN 256					/* Max length of characters in a line */
#define PC		 7							/* Register 7 - PC */

/* 
	Contains the status codes for the xmeLoader 
*/
enum LOADER_STATUS { 
	LOADER_SUCCESS, LOADER_BADFILE, LOADER_BADSREC, LOADER_CHKSUM_ERR 
};	

extern union MEM_OLAY MEM;				/* Emulation of Memory */
extern signed short REG_FILE[];			/* Register File */

char AssessingFileName[LINE_LEN];		/* The S-Record currently being processed */