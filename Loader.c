#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Loader.h"
#include "memory.h"

char srec[LINE_LEN];	/* Copy of each S-Record to process */
FILE* fp;				/* Pointer to S-Records file */

/*
	Opens input file for reading.
	Analyzes each S-Record, line by line.
	Loads data bytes into memory.
	Verifies CheckSum of each record.
	Extracts starting address for the Emulator.
*/
int XmeLoader(char* fileName) {
	unsigned int SrecType;								/* 1st and 2nd char */
	unsigned int SrecLen;								/* 3rd and 4th char */
	unsigned int SrecAddr, SrecAddrLo, SrecAddrHi;		/* Chars 5 to 8 */
	signed char SrecChkSum;								/* Last two chars */
	signed int SrecCharPair;							/* Pair of chars being read at a time */			

	/* Open S-Records file to read */
	if (!(fp = fopen(fileName, "r"))) 
		return LOADER_BADFILE;

	while (fgets(srec, LINE_LEN, fp) != NULL) {

		if (srec[0] != 'S')
			return LOADER_BADSREC;

		/* Subtract '0' from srec[1] to get the ascii Decimal representaion of char */
		SrecType = srec[1] - '0';
		if (SrecType > 1 && SrecType != 9)
			return LOADER_BADSREC;

		/*
			Scan: 
				3rd and 4th char = Length 
				4th and 5th char = Address High Byte
				6th and 7th char = Address Low Byte
		*/
		sscanf(&srec[2], "%2x%2x%2x", &SrecLen, &SrecAddrHi, &SrecAddrLo);

		/*
			Number of char pairs to read in S1 records.
			Subtract 2 for address bytes, and 1 for checksum byte, from SrecLen
		*/
		unsigned int SrecBytesCount = SrecLen - 3;

		SrecChkSum = SrecLen + SrecAddrHi + SrecAddrLo;
		SrecAddr = (SrecAddrHi << 8) | SrecAddrLo;
		
		unsigned int i = 0, index = 8;
		/*
			S0 Records: Copy 8th to third last char as AssessingFileName
			S9 Records: Copy Starting Addr to R7 (PC); verify CheckSum
			S1 Records: Copy char pairs (starting from SrecAddr) to memory; verify CheckSum
		*/
		switch (SrecType) {
		case 0:
			for (i = 0; i < SrecLen-3; i++) {
				sscanf(&srec[index], "%c", &AssessingFileName[i]);
				index++;
			}
			printf("Input '.asm' File: %s\n", AssessingFileName);
			break;
		case 9:
			REG_FILE[PC] = SrecAddr;
			break;
		default:
			for (i = 0; i < SrecBytesCount; i++) {
				sscanf(&srec[index], "%2x", &SrecCharPair);
				SrecChkSum += SrecCharPair;
				MEM.MEM_BYTE[SrecAddr] = (unsigned char) SrecCharPair;
				SrecAddr++;
				index+=2;
			}
			break;
		}

		/* Read Check Sum char pair */
		sscanf(&srec[index], "%2x", &SrecCharPair);

		/* Check if S-Record is valid by verifying Check Sum */
		SrecChkSum += SrecCharPair;
		if (SrecType != 0 && SrecChkSum != -1)
			return LOADER_CHKSUM_ERR;

	}


	fclose(fp);
	return LOADER_SUCCESS;

}