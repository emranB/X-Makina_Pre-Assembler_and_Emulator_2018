#include <stdint.h>
#include <stdio.h>
#include "Emulator.h"
#include "Memory.h"
#include "cpu.h"
#include "Instructions.h"

extern struct PSW_BITS* PSWptr;			/* Structure of PSW externed from cpu.c */
extern signed short REG_FILE[];			/* Register file externed from cpu.c */
extern unsigned long SYS_CLK;			/* System Clock */
extern FILE* FOUT_INSTS;				/* File pointer to write results of Instructions */

/*
	Combination of PRPO, DEC and INC bits from Register Direct
	addressing mode, to map address modifier types.
*/
enum REG_DIR_ADDR_MODS {
	UNMODIFIED = 0,	/* PRPO DEC INC = 0 0 0 -> Unmodified */
	PRE_INC	   = 5,	/* PRPO DEC INC = 1 0 1 -> Pre-increment */
	POST_INC   = 1,	/* PRPO DEC INC = 0 0 1 -> Post-increment */
	PRE_DEC	   = 6,	/* PRPO DEC INC = 1 1 0 -> Pre-decrement */
	POST_DEC   = 2	/* PRPO DEC INC = 0 1 0 -> Post-decrement */
};


/* Print results to external file */
void PrintDataResults(char*, char*, signed char, char*, signed char);



/*
	LD: Load contents from memory location at Effective Address, to Dst operand
	Effective Address:	Value of Src register
	prpo:				0 = Post or None, 1 = Pre
	dec:				0 = No Decrement, 1 = Decrement
	inc:				0 = No Increment, 1 = Increment
	Note: For byte operations, only the LO byte is read or written to, and HI byte is left unchanged.
*/
void Process_LD(unsigned char prpo, unsigned char dec, unsigned char inc, unsigned char wb, unsigned int src, unsigned int dst) {
	PrintDataResults("LD", "SRC", src, "DST", dst);

	/* Getting Effective address from Src operand */
	unsigned short EA;
	/* Used for byte operations */
	signed char TEMP_BYTE;
	/* Type of address modifier - PRPO DEC INC */
	enum REG_DIR_ADDR_MODS addr_mod = (prpo << 2) | (dec << 1) | (inc);

	switch (addr_mod) {
	case UNMODIFIED:
		if (wb == WORD) {		/* Word */
			EA = REG_FILE[src];
			MEM_RD(EA, REG_FILE[dst], WORD);
		}
		else {					/* Byte */
			EA = REG_FILE[src];
			MEM_RD(EA, TEMP_BYTE, BYTE);
			REG_FILE[dst] = (REG_FILE[dst] & 0xFF00) | TEMP_BYTE;
		}
		break;
	case PRE_INC:
		if (wb == WORD) {	/* Word */
			REG_FILE[src] += 2;
			EA = REG_FILE[src];
			MEM_RD(EA, REG_FILE[dst], WORD);
		}
		else {				/* Byte */
			REG_FILE[src]++;
			EA = REG_FILE[src];
			MEM_RD(EA, TEMP_BYTE, BYTE);
			REG_FILE[dst] = (REG_FILE[dst] & 0xFF00) | TEMP_BYTE;
		}
		break;
	case POST_INC:
		if (wb == WORD) {	/* Word */
			EA = REG_FILE[src];
			MEM_RD(EA, REG_FILE[dst], WORD);
			REG_FILE[src] += 2;
		}
		else {				/* Byte */
			EA = REG_FILE[src];
			MEM_RD(EA, TEMP_BYTE, BYTE);
			REG_FILE[dst] = (REG_FILE[dst] & 0xFF00) | TEMP_BYTE;
			REG_FILE[src]++;
		}
		break;
	case PRE_DEC:
		if (wb == WORD) {	/* Word */
			REG_FILE[src] -= 2;
			EA = REG_FILE[src];
			MEM_RD(EA, REG_FILE[dst], WORD);
		}
		else {				/* Byte */
			REG_FILE[src]--;
			EA = REG_FILE[src];
			MEM_RD(EA, TEMP_BYTE, BYTE);
			REG_FILE[dst] = (REG_FILE[dst] & 0xFF00) | TEMP_BYTE;
		}
		break;
	case POST_DEC:
		if (wb == 0) {		/* Word */
			EA = REG_FILE[src];
			MEM_RD(EA, REG_FILE[dst], WORD);
			REG_FILE[src] -= 2;
		}
		else {				/* Byte */
			EA = REG_FILE[src];
			MEM_RD(EA, TEMP_BYTE, BYTE);
			REG_FILE[dst] = (REG_FILE[dst] & 0xFF00) | TEMP_BYTE;
			REG_FILE[src]--;
		}
		break;
	}

}



/*
	ST: Store contents of Src, to memory at Effective Address
	Effective Address:	
		- Pre  -> Dst + Address Modifiers
		- Post -> Src + Address Modifiers
	prpo -> 0 = Post or None, 1 = Pre
	dec ->  0 = No Decrement, 1 = Decrement
	inc	->  0 = No Increment, 1 = Increment
*/
void Process_ST(unsigned char prpo, unsigned char dec, unsigned char inc, unsigned char wb, unsigned int src, unsigned int dst) {
	PrintDataResults("ST", "SRC", src, "DST", dst);

	/* Getting Effective address from Src operand */
	unsigned short EA;
	/* Type of address modifier - PRPO DEC INC */
	enum REG_DIR_ADDR_MODS addr_mod = (prpo << 2) | (dec << 1) | (inc);

	switch (addr_mod) {
	case UNMODIFIED:
		EA = REG_FILE[dst];
		MEM_WR(EA, REG_FILE[src], wb);
		break;
	case PRE_INC:
		/*
			If wb = WORD, Increment Dst by 2
			Else,		  Increment Dst by 1
		*/
		(wb == WORD) ? (REG_FILE[dst] += 2) : (REG_FILE[dst]++);
		EA = REG_FILE[dst];
		MEM_WR(EA, REG_FILE[src], wb);
		break;
	case POST_INC:
		EA = REG_FILE[dst];
		MEM_WR(EA, REG_FILE[src], wb);
		/*
			If wb = WORD, Increment Dst by 2
			Else,		  Increment Dst by 1
		*/
		(wb == WORD) ? (REG_FILE[dst] += 2) : (REG_FILE[dst]++);
		break;
	case PRE_DEC:
		/*
			If wb = WORD, Decrement Dst by 2
			Else,		  Decrement Dst by 1
		*/
		(wb == WORD) ? (REG_FILE[dst] -= 2) : (REG_FILE[dst]--);
		EA = REG_FILE[dst];
		MEM_WR(EA, REG_FILE[src], wb);
		break;
	case POST_DEC:
		EA = REG_FILE[dst];
		MEM_WR(EA, REG_FILE[src], wb);
		/*
			If wb = WORD, Decrement Dst by 2
			Else,		  Decrement Dst by 1
		*/
		(wb == WORD) ? (REG_FILE[dst] -= 2) : (REG_FILE[dst]--);
		break;
	}

}


/*
	LO byte of Dst is replaced with 8-bit data byte of Src.
	HI byte of Dst is left unchanged.
*/
void Process_MOVL(unsigned char data_byte, unsigned int dst) {
	PrintDataResults("MOVL", "DATA", data_byte, "DST", dst);
	unsigned char dst_HI = REG_FILE[dst] & 0xFF00;
	unsigned char dst_LO = data_byte;
	REG_FILE[dst] = (dst_HI << 8) | dst_LO;
}



/*
	LO byte of Dst is replaced with 8-bit data byte of Src.
	HI byte of Dst is replaced with 0's.
*/
void Process_MOVLZ(unsigned char data_byte, unsigned int dst) {
	PrintDataResults("MOVLZ", "DATA", data_byte, "DST", dst);
	unsigned char dst_HI = 0x00;
	unsigned char dst_LO = data_byte;
	REG_FILE[dst] = (dst_HI << 8) | dst_LO;
}



/*
	LO byte of Dst is left unchanged.
	HI byte of Dst is replaced with 8-bit data byte of Src.
*/
void Process_MOVH(unsigned char data_byte, unsigned int dst) {
	PrintDataResults("MOVH", "DATA", data_byte, "DST", dst);
	unsigned char dst_HI = data_byte;
	unsigned char dst_LO = REG_FILE[dst] & 0x00FF;
	REG_FILE[dst] = (dst_HI << 8) | dst_LO;
}



/*
	Load contents of memory at Effective Address to Dst Operand.
	Effective Address = Src Operand + Offset
	Note: The 6-bit offset is sign extended and added to Src Operand 
		  to get Effective Address. 
	Range: +31 to -32 Bytes, or, -16 to +15 Words.

*/
void Process_LDR(unsigned char offset, unsigned char wb, unsigned char src, unsigned char dst) {
	PrintDataResults("LDR", "Offset", offset, "DST", dst);

	signed short total_offset;
	signed short EA;			/* Effective Address */

	/* Extracting Sign bit (bit-6) */
	unsigned signbit = offset & 0x20;

	/* Extending Sign bit*/
	if (signbit == 1)
		total_offset = 0xFFC0 | ((offset) & 0x3F);
	else
		total_offset = 0x003F & ((offset) & 0x3F);

	/* Adding value of Src operand */
	EA = REG_FILE[src] + total_offset;

	MEM_RD(EA, REG_FILE[dst], wb);

}



/*
	Load contents of memory at Effective Address to Dst Operand.
	Effective Address = Dst Operand + Offset
	Note: The 6-bit offset is sign extended and added to Src Operand
	to get Effective Address.
	Range: +31 to -32 Bytes, or, -16 to +15 Words.
*/
void Process_STR(unsigned char offset, unsigned char wb, unsigned char src, unsigned char dst) {
	PrintDataResults("STR", "Offset", offset, "DST", dst);

	signed short total_offset;
	signed short EA;			/* Effective Address */

	/* Extracting Sign bit (bit-6) */
	unsigned signbit = offset & 0x20;

	/* Extending Sign bit*/
	if (signbit == 1)
		total_offset = 0xFFC0 | (offset & 0x3F);
	else
		total_offset = 0x003F & (offset & 0x3F);

	/* Adding value of Src operand */
	EA = REG_FILE[dst] + total_offset;

	MEM_WR(EA, REG_FILE[src], wb);
}



/*
	Print Data Results
*/
void PrintDataResults(char* INST, char* PARAM1_NAME, signed char PARAM1_VAL, char* PARAM2_NAME, signed char PARAM2_VAL) {
	printf("Found **%s**	---		%s(%04x), %s(%04x)\n",
		INST, PARAM1_NAME, PARAM1_VAL & 0xFFFF, PARAM2_NAME, PARAM2_VAL & 0xFFFF);

	fprintf(FOUT_INSTS, "Found **%s**	---		%s(%04x), %s(%04x)\n",
		INST, PARAM1_NAME, PARAM1_VAL & 0xFFFF, PARAM2_NAME, PARAM2_VAL & 0xFFFF);
}