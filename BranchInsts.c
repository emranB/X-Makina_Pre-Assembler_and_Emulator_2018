/*
	Function Handlers for Branch Instructions
	- Convert offset to 16-bit value
	- Keep sign bit through sign extension
	- Make PC jump to new offset location
*/
#include <stdint.h>
#include <stdio.h>
#include "cpu.h"
#include "Instructions.h"


extern struct PSW_BITS* PSWptr;			/* Program Status Word */
extern signed short REG_FILE[];			/* Register File */
extern FILE* FOUT_INSTS;				/* Write Instructions to external file */

/*
	Sign extend a 10-bit offset to a 16-bit value
		@params 10-bit offset
		@return 16-bit offset
*/
signed short TenBitFullOffset(signed short offset);

/* Print Resulsts */
void PrintBranchResults(char*, signed short);


/*
	LR <- PC
	PC = PC + offset
	13-bit offset it shifted left by 1, then sign extended, to get full offset.
	Branch with link to subroutine.
	Store return address in LR.
*/
void Process_BL(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BL", offset);

	unsigned offset_MSB; 

	offset	   = (offset << 1);	/* Shift left by 1, to ensure even value */
	offset_MSB = (offset & 0x2000) >> 13;		/* Extract bit-13 */

	if (offset_MSB == 1)
		offset |= 0xC000;
	else
		offset &= 0x3FFF;

	REG_FILE[LR]  = REG_FILE[PC];
	REG_FILE[PC] += offset;
}



/*
	PC <- PSW.Z = 1 ? PC+label : PC
	Branch to label if zero flag is set
*/
void Process_BEQ(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BEQ", offset);

	offset = TenBitFullOffset(offset);
	REG_FILE[PC] = (PSWptr->zero == 1) ? (REG_FILE[PC] + offset) : REG_FILE[PC];
}



/*
	PC <- PSW.Z = 0 ? PC+label : PC
	Branch to label if zero flag is cleared
*/
void Process_BNE(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BNE", offset);

	offset = TenBitFullOffset(offset);
	REG_FILE[PC] = (PSWptr->zero == 0) ? (REG_FILE[PC] + offset) : REG_FILE[PC];
}



/*
	PC <- PSW.C = 1 ? PC+label : PC
	Branch to label if carry flag is set 
*/
void Process_BC(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BC", offset);

	offset = TenBitFullOffset(offset);
	REG_FILE[PC] = (PSWptr->carry == 1) ? (REG_FILE[PC] + offset) : REG_FILE[PC];
}



/*
	PC <- PSW.C = 0 ? PC+label : PC
	Branch to label if carry flag is cleared 
*/
void Process_BNC(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BNC", offset);

	offset = TenBitFullOffset(offset);
	REG_FILE[PC] = (PSWptr->carry == 0) ? (REG_FILE[PC] + offset) : REG_FILE[PC];
}



/*
	PC <- PSW.N = 1 ? PC+label : PC
	Branch to label if negative flag is set
*/
void Process_BN(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BN", offset);

	offset = TenBitFullOffset(offset);
	REG_FILE[PC] = (PSWptr->negative == 1) ? (REG_FILE[PC] + offset) : REG_FILE[PC];
}



/*
	PC <- (PSW.N  .XOR.  PSW.V) = 0 ? PC+label : PC
	Branch to label if greater than or equal 
*/
void Process_BGE(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BGE", offset);

	offset = TenBitFullOffset(offset);
	REG_FILE[PC] = ((PSWptr->negative ^ PSWptr->overflow) == 0) ? (REG_FILE[PC] + offset) : REG_FILE[PC];
}



/*
	PC <- (PSW.N  .XOR.  PSW.V) = 1 ? PC+label : PC
	Branch to label if greater than or equal
*/
void Process_BLT(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BLT", offset);

	offset = TenBitFullOffset(offset);
	REG_FILE[PC] = ((PSWptr->negative ^ PSWptr->overflow) == 1) ? (REG_FILE[PC] + offset) : REG_FILE[PC];
}



/*
	PC <- PC+label
	Branch always (unconditional) to label 
*/
void Process_BAL(signed short offset) {
	/* Print to external file */
	PrintBranchResults("BAL", offset);

	offset = TenBitFullOffset(offset);
	REG_FILE[PC] += offset;
}




/*
	Sign extend a 10-bit offset to a 16-bit value
	@params 10-bit offset
	@return 16-bit offset
*/
signed short TenBitFullOffset(signed short offset) {
	unsigned offset_MSB;

	/* Shift left by one. Replace 3-MSB with zeros */
	offset = (offset << 1) & 0x03FF;		

	/* Get bit-10 of offset */
	offset_MSB = (offset & 0x0200) >> 9;	

	/* Sign extend 13-bit offset */
	if (offset_MSB == 1)
		offset |= 0xFC00;
	else
		offset &= 0x03FF;

	return offset;
}



/*
	Print Results
*/
void PrintBranchResults(char* INST, signed short offset) {
	//printf("Found **%s**	---		Offset:(%04x)\n", INST, offset, offset);
	fprintf(FOUT_INSTS, "Found **%s**	---		Offset(%04x)\n", INST, offset, offset);
}