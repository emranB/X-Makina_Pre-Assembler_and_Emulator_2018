#include <stdint.h>
#include <stdio.h>
#include "cpu.h"
#include "Instructions.h"


extern struct PSW_BITS* PSWptr;			/* Program Status Word */
extern signed short REG_FILE[];			/* Register File */
enum RC reg_or_const;					/* Register or Constant */
extern FILE* FOUT_INSTS;

/* Set PSW bits after completing an Arithmetic Inst */
void SetPSW(signed short, signed short, signed short, unsigned wb);

/* Print Results */
void PrintArithResults(char*, unsigned char, unsigned char, unsigned int, unsigned int);

/*
	Look-up table for Register or Constant
	
	  Src	|  Reg	| Const
	-------------------------
	R0 / 0  |	0	|	0	|
	R1 / 1	|	1	|	1	|
	R2 / 2	|	2	|	2	|
	R3 / 3	|	3	|	4	|
	R4 / 4	|	4	|	8	|
	R5 / 5	|	5	|	32	|
	R6 / 6	|	6	|	48	|
	R7 / 7	|	7	|	-1	|

	Usage:
		RC_TBL[src][reg_or_const] = value
	eg.
		RC_TBL[5][REGISTER] = 5
		RC_TBL[5][CONSTANT] = 32
*/
signed int RC_TBL[8][2] = {
	{ 0,  0 },
	{ 1,  1 },
	{ 2,  2 },
	{ 3,  4 },
	{ 4,  8 },
	{ 5, 32 },
	{ 6, 48 },
	{ 7, -1 },
};



/*
	Add value of Src to value of Dst and Save in Dst
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_ADD(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("ADD", rc, wb, src, dst);

	signed dst_pre_op;
	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op  = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]   += src_val;
		sign_bit_post_op = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
	}
	else {													/* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);			/* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);			/* Extracting low byte of Dst */
		sign_bit_pre_op  = BYTE_MSB(dst_LO);				/* Extract bit-7 */
		temp_byte_LO     = LO_BYTE(REG_FILE[src]);			/* Extracting low byte of Src */
		temp_result_byte = dst_LO + temp_byte_LO;			/* Adding low bytes or Src and Dst */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]    = (dst_HI << 8) | temp_result_byte;
		sign_bit_post_op = BYTE_MSB(temp_result_byte);		/* Getting sign bit of result */
	}

	SetPSW(src_val, dst_pre_op, REG_FILE[dst], wb);
}



/*
	Add value of Src, value of Dst, and carry bit in PSW, and Save in Dst
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_ADDC(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("ADDC", rc, wb, src, dst);

	signed short dst_pre_op;
	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op	 = WORD_MSB(REG_FILE[dst]);		/* Extract bit-15 (MSB) */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]   += src_val + PSWptr->carry;
		PSWptr->carry	 = 0;							/* Reset carry bit after borrowing */
		sign_bit_post_op = WORD_MSB(REG_FILE[dst]);		/* Extract bit-15 (MSB) */
	}
	else {															/* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);					/* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);					/* Extracting low byte of Dst */
		sign_bit_pre_op	 = BYTE_MSB(dst_LO);						/* Extract bit-7 */
		temp_byte_LO	 = LO_BYTE(REG_FILE[src]);					/* Extracting low byte of Src */
		temp_result_byte = dst_LO + temp_byte_LO + PSWptr->carry;	/* Adding low bytes or Src and Dst */
		PSWptr->carry	 = 0;										/* Reset carry bit after borrowing */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]	 = (dst_HI << 8) | temp_result_byte;
		sign_bit_post_op = BYTE_MSB(temp_result_byte);				/* Getting sign bit of result */
	}

	SetPSW(src_val, dst_pre_op, REG_FILE[dst], wb);
}



/*
	Subtract value of Src from Dst and Save in Dst
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_SUB(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("SUB", rc, wb, src, dst);

	signed short dst_pre_op;
	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op	 = WORD_MSB(REG_FILE[dst]);		/* Extract bit-15 (MSB) */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]   -= src_val;
		sign_bit_post_op = WORD_MSB(REG_FILE[dst]);		/* Extract bit-15 (MSB) */
	}
	else {												/* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);		/* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);		/* Extracting low byte of Dst */
		sign_bit_pre_op	 = BYTE_MSB(dst_LO);			/* Extract bit-7 */
		temp_byte_LO	 = LO_BYTE(REG_FILE[src]);		/* Extracting low byte of Src */
		temp_result_byte = dst_LO - temp_byte_LO;		/* Subtracting low byte of Src to Dst */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]	 = (dst_HI << 8) | temp_result_byte;
		sign_bit_post_op = BYTE_MSB(temp_result_byte);	/* Getting sign bit of result */
	}

	SetPSW(src_val, dst_pre_op, REG_FILE[dst], wb);
}



/*
	Subtract value of Src from Dst, add PSW carry bit, and Save in Dst
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_SUBC(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("SUBC", rc, wb, src, dst);

	signed short dst_pre_op;
	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op  = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]   -= src_val + PSWptr->carry;
		PSWptr->carry    = 0;								/* Reset carry bit after borrowing */
		sign_bit_post_op = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
	}
	else {															/* Byte */
		dst_HI           = HI_BYTE(REG_FILE[dst]);					/* Extracting High byte of Dst */
		dst_LO           = LO_BYTE(REG_FILE[dst]);					/* Extracting low byte of Dst */
		sign_bit_pre_op  = BYTE_MSB(dst_LO);						/* Extract bit-7 */
		temp_byte_LO     = LO_BYTE(REG_FILE[src]);					/* Extracting low byte of Src */
		temp_result_byte = dst_LO - temp_byte_LO + PSWptr->carry;	/* Subtracting low byte of Src to Dst */
		PSWptr->carry    = 0;										/* Reset carry bit after borrowing */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]    = (dst_HI << 8) | temp_result_byte;
		sign_bit_post_op = BYTE_MSB(temp_result_byte);				/* Getting sign bit of result */
	}

	/* Set bits in PSW */
	SetPSW(src_val, dst_pre_op, REG_FILE[dst], wb);

}




void Process_DADD(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("DADD", rc, wb, src, dst);

}



/*
	Subtract value of Src from Dst and compare result
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_CMP(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("CMP", rc, wb, src, dst);

	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed short temp_result_word;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op	  = WORD_MSB(REG_FILE[dst]);			  /* Extract bit-15 (MSB) */
		temp_result_word  = REG_FILE[dst] - src_val;
		sign_bit_post_op  = WORD_MSB(temp_result_word);			  /* Extract bit-15 (MSB) */
	}
	else {														  /* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);				  /* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);				  /* Extracting low byte of Dst */
		sign_bit_pre_op  = BYTE_MSB(dst_LO);					  /* Extract bit-7 */
		temp_byte_LO	 = LO_BYTE(REG_FILE[src]);				  /* Extracting low byte of Src */
		temp_result_word = (signed short)(dst_LO - temp_byte_LO); /* Subtracting low byte of Src to Dst */
		sign_bit_post_op = BYTE_MSB(temp_result_word);			  /* Getting sign bit of result */
	}

	/* Set bits in PSW */
	SetPSW(src_val, REG_FILE[dst], temp_result_word, wb);
}



/*
	XOR Src and Dst and save in Dst
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_XOR(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("XOR", rc, wb, src, dst);

	signed short dst_pre_op;
	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op  = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]    = REG_FILE[dst] ^ src_val;
		sign_bit_post_op = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
	}
	else {													/* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);			/* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);			/* Extracting low byte of Dst */
		sign_bit_pre_op  = BYTE_MSB(dst_LO);				/* Extract bit-7 */
		temp_byte_LO	 = LO_BYTE(REG_FILE[src]);			/* Extracting low byte of Src */
		temp_result_byte = dst_LO ^ temp_byte_LO;			/* Subtracting low byte of Src to Dst */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]	 = (dst_HI << 8) | temp_result_byte;
		sign_bit_post_op = BYTE_MSB(temp_result_byte);		/* Getting sign bit of result */
	}

	/* Set bits in PSW */
	SetPSW(src_val, dst_pre_op, REG_FILE[dst], wb);
}



/*
	DST = DST & SRC
	 - AND Src and Dst and save in Dst
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_AND(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("AND", rc, wb, src, dst);

	signed short dst_pre_op;
	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op  = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]    = REG_FILE[dst] & src_val;
		sign_bit_post_op = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
	}
	else {													/* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);			/* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);			/* Extracting low byte of Dst */
		sign_bit_pre_op  = BYTE_MSB(dst_LO);				/* Extract bit-7 */
		temp_byte_LO     = LO_BYTE(src_val);			/* Extracting low byte of Src */
		temp_result_byte = dst_LO & temp_byte_LO;			/* Subtracting low byte of Src to Dst */
		dst_pre_op		 = REG_FILE[dst];
		REG_FILE[dst]    = (dst_HI << 8) | temp_result_byte;
		sign_bit_post_op = BYTE_MSB(temp_result_byte);		/* Getting sign bit of result */
	}

	/* Set bits in PSW */
	SetPSW(src_val, dst_pre_op, REG_FILE[dst], wb);
}



/*
	DST & SRC
	- AND Src and Dst and compare result
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_BIT(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("BIT", rc, wb, src, dst);

	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed short temp_result_word;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op  = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
		temp_result_word = REG_FILE[dst] & src_val;
		sign_bit_post_op = WORD_MSB(temp_result_word);		/* Extract bit-15 (MSB) */

		SetPSW(temp_result_word, sign_bit_pre_op, sign_bit_post_op, wb);
	}
	else {													/* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);			/* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);			/* Extracting low byte of Dst */
		sign_bit_pre_op  = BYTE_MSB(dst_LO);				/* Extract bit-7 */
		temp_byte_LO     = LO_BYTE(REG_FILE[src]);			/* Extracting low byte of Src */
		temp_result_byte = dst_LO & temp_byte_LO;			/* Subtracting low byte of Src to Dst */
		sign_bit_post_op = BYTE_MSB(temp_result_byte);		/* Getting sign bit of result */

		SetPSW(temp_result_byte, sign_bit_pre_op, sign_bit_post_op, wb);
	}
}



/*
	DST = DST & ~SRC
	 - AND Src and Dst and compare result
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_BIC(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("BIC", rc, wb, src, dst);

	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op  = WORD_MSB(REG_FILE[dst]);		/* Extract bit-15 (MSB) */
		REG_FILE[dst]    = REG_FILE[dst] & (~src_val);
		sign_bit_post_op = WORD_MSB(REG_FILE[dst]);		/* Extract bit-15 (MSB) */

		SetPSW(REG_FILE[dst], sign_bit_pre_op, sign_bit_post_op, wb);
	}
	else {												/* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);		/* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);		/* Extracting low byte of Dst */
		sign_bit_pre_op  = BYTE_MSB(dst_LO);			/* Extract bit-7 */
		temp_byte_LO     = LO_BYTE(REG_FILE[src]);		/* Extracting low byte of Src */
		temp_result_byte = dst_LO & (~temp_byte_LO);	/* Subtracting low byte of Src to Dst */
		REG_FILE[dst]    = (dst_HI << 8) | temp_result_byte;
		sign_bit_post_op = BYTE_MSB(temp_result_byte);	/* Getting sign bit of result */

		SetPSW(temp_result_byte, sign_bit_pre_op, sign_bit_post_op, wb);
	}
}



/*
	DST = DST | SRC
	 - AND Src and Dst and compare result
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_BIS(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("BIS", rc, wb, src, dst);

	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		sign_bit_pre_op  = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */
		REG_FILE[dst]    = REG_FILE[dst] | src_val;
		sign_bit_post_op = WORD_MSB(REG_FILE[dst]);			/* Extract bit-15 (MSB) */

		SetPSW(REG_FILE[dst], sign_bit_pre_op, sign_bit_post_op, wb);
	}
	else {													/* Byte */
		dst_HI			 = HI_BYTE(REG_FILE[dst]);			/* Extracting High byte of Dst */
		dst_LO			 = LO_BYTE(REG_FILE[dst]);			/* Extracting low byte of Dst */
		sign_bit_pre_op  = BYTE_MSB(dst_LO);				/* Extract bit-7 */
		temp_byte_LO     = LO_BYTE(REG_FILE[src]);			/* Extracting low byte of Src */
		temp_result_byte = dst_LO & temp_byte_LO;			/* Subtracting low byte of Src to Dst */
		REG_FILE[dst]    = (dst_HI << 8) | temp_result_byte;
		sign_bit_post_op = BYTE_MSB(temp_result_byte);		/* Getting sign bit of result */

		SetPSW(temp_result_byte, sign_bit_pre_op, sign_bit_post_op, wb);
	}
}



/*
	DST = SRC
	 - Set Dst to the value of Src
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_MOV(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("MOV", rc, wb, src, dst);

	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		REG_FILE[dst] = src_val;
	}
	else {													/* Byte */
		dst_HI = HI_BYTE(REG_FILE[dst]);					/* Extracting High byte of Dst */
		temp_byte_LO = LO_BYTE(src_val);					/* Extracting low byte of Src */
		REG_FILE[dst] = (dst_HI << 8) | temp_byte_LO;
	}
}



/*
	Swap Src and Dst
	 - Set Dst to the value of Src
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_SWAP(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("SWAP", rc, wb, src, dst);

	unsigned sign_bit_pre_op, sign_bit_post_op;
	signed short temp_word;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		temp_word     = REG_FILE[dst];
		REG_FILE[dst] = src_val;
		REG_FILE[src] = temp_word;
	}
	else {													/* Byte */
		dst_LO		  = LO_BYTE(REG_FILE[dst]);				/* Extracting High byte of Dst */
		temp_byte_LO  = LO_BYTE(src_val);					/* Extracting low byte of Src */
		REG_FILE[src] = ((HI_BYTE(REG_FILE[src])) << 8) | dst_LO;
		REG_FILE[dst] = (REG_FILE[dst] & 0xFF00) | temp_byte_LO;
	}
}



/*
	DST.MSB -> ... DST.LSB -> C
		- Shift each bit right by one place
		- DST.LSB gets Carry bit
		- DST.MSB reatins sign bit
	IF BYTE operation:
	Perform the same operation for Low bytes of Src and Dst
*/
void Process_SRA(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("SRA", rc, wb, src, dst);

	unsigned MSB, LSB;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		MSB = WORD_MSB(REG_FILE[dst]);			/* New MSB */
		LSB = WORD_LSB(REG_FILE[dst]);			/* Get bit-0 (LSB) */
		/*
			IF MSB = 0 
				~(MSB << 15) = 1111 1111 1111 1111
			IF MSB = 1 
				~(MSB << 15) = 0111 1111 1111 1111
		*/
		REG_FILE[dst] = (REG_FILE[dst] >> 1) & (~(MSB << 15));
		PSWptr->carry = LSB;
	}
	else {											/* Byte */
		dst_HI		  = HI_BYTE(REG_FILE[dst]);		/* Get High byte of Dst */
		dst_LO		  = LO_BYTE(REG_FILE[dst]);		/* Get low byte of Dst */
		MSB           = WORD_MSB(REG_FILE[dst]);	/* New MSB */
		LSB           = BYTE_LSB(dst_LO);			/* Get bit-0 (LSB) */
		temp_byte_LO  = (dst_LO >> 1) & (~(MSB << 7));
		REG_FILE[dst] = (dst_HI << 8) | temp_byte_LO;
		PSWptr->carry = LSB;
	}
}



/*
	C->DST.MSB -> ... DST.LSB->C
		- Shift each bit right by one place
		- DST.MSB is set to Carry bit
		- DST.LSB gets Carry bit
	IF BYTE operation:
		Perform the same operation for Low bytes of Src and Dst
*/
void Process_RRC(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("RRC", rc, wb, src, dst);

	unsigned MSB, LSB;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	signed short src_val = RC_TBL[src][rc];
	if (rc == REGISTER)
		src_val = REG_FILE[src_val];

	if (wb == WORD) {
		MSB = PSWptr->carry;					/* New MSB */
		LSB = WORD_LSB(REG_FILE[dst]);			/* Extract bit-0 (LSB) */
		/*
			IF MSB = 0
				~(MSB << 15) = 1111 1111 1111 1111
			IF MSB = 1
				~(MSB << 15) = 0111 1111 1111 1111
		*/
		REG_FILE[dst] = (REG_FILE[dst] >> 1) & (~(MSB << 15));
		PSWptr->carry = LSB;
	}
	else {											/* Byte */
		dst_HI		  = HI_BYTE(REG_FILE[dst]);		/* Get High byte of Dst */
		dst_LO		  = LO_BYTE(REG_FILE[dst]);		/* Get low byte of Dst */
		MSB			  = PSWptr->carry;				/* New MSB */
		LSB			  = BYTE_LSB(dst_LO);			/* Get bit-0 (LSB) */
		temp_byte_LO = (dst_LO >> 1) & (~(MSB << 7));
		REG_FILE[dst] = (dst_HI << 8) | temp_byte_LO;
		PSWptr->carry = LSB;
	}
}



/*
	SWAP HI byte with LO, in DST
	Word oeprations allowed only (byte ignored)
*/
void Process_SWPB(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("SWPB", rc, wb, src, dst);

	unsigned MSB, LSB;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	MSB = HI_BYTE(REG_FILE[dst]);			/* Get High Byte */
	LSB = LO_BYTE(REG_FILE[dst]);			/* Get Low Byte */

	REG_FILE[dst] = (MSB >> 8) | (LSB << 8);
}



/*
	Sign extend LO byte to Word, in Dst
	Word oeprations allowed only (byte ignored)
*/
void Process_SXT(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {

	/* Print Results to screen and external file */
	PrintArithResults("SXT", rc, wb, src, dst);

	unsigned MSBit, LSByte;
	signed char dst_HI, dst_LO, temp_result_byte, temp_byte_LO;

	LSByte = LO_BYTE(REG_FILE[dst]);			/* Get Low Byte */
	MSBit = BYTE_MSB(LSByte);			/* Get bit-7 (MSB) */

	if (MSBit == 1)
		REG_FILE[dst] = LSByte & 0xFFFF;
	else
		REG_FILE[dst] = LSByte & 0x00FF;
}



void none(unsigned char rc, unsigned char wb, unsigned int src, unsigned int dst) {
	/* Print Results to screen and external file */
	PrintArithResults("NOP", rc, wb, src, dst);
}



/*
	Set PSW bits
	@params result of operation, sign bit before operartion, sign bit after operation
	@return void
*/
void SetPSW(signed short src, signed short dst_pre_op, signed short dst_post_op, unsigned wb) {

	unsigned dst_sign_bit_pre_op, dst_sign_bit_post_op;
	signed char dst_HI, dst_LO, temp_byte_HI, temp_byte_LO;

	src			= (wb == WORD) ? (src)		   : ((signed char)src);
	dst_pre_op  = (wb == WORD) ? (dst_pre_op)  : ((signed char)dst_pre_op);
	dst_post_op = (wb == WORD) ? (dst_post_op) : ((signed char)dst_post_op);

	dst_sign_bit_pre_op  = (wb == WORD) ? WORD_MSB(dst_pre_op)  : BYTE_MSB(dst_pre_op);
	dst_sign_bit_post_op = (wb == WORD) ? WORD_MSB(dst_post_op) : BYTE_MSB(dst_post_op);

	/* Check for negative result */
	PSWptr->negative = (dst_post_op < 0) ? 1 : 0;

	/* Check for overflow 
	   SET
		IF two negatives result in a positive
		OR two positives result in a negative
	*/
	if ((src < 0 && dst_pre_op < 0 && dst_post_op >0) || (src >= 0 && dst_pre_op >= 0 && dst_post_op <= 0))
		PSWptr->overflow = 1;
	else
		PSWptr->overflow = 0;

	/* Check for carry */
	PSWptr->carry = (dst_sign_bit_pre_op == 1 && dst_sign_bit_post_op == 0) ? 1 : 0;

	/* Check for zero result */
	PSWptr->zero = (dst_post_op == 0) ? 1 : 0;
}



/*
	Print Results
*/
void PrintArithResults(char* INST, unsigned char RC, unsigned char WB, unsigned int SRC, unsigned int DST) {
	printf("Found **%s**	---		RC(%d), WB(%d), SRC(%d), DST(%d)\n",
		INST, RC, WB, SRC, DST);

	fprintf(FOUT_INSTS, "Found **%s**	---		RC(%d), WB(%d), SRC(%d), DST(%d)\n",
		INST, RC, WB, SRC, DST);
}