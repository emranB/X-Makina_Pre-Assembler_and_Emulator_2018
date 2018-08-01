/*
	Emulation of all processes carried out by the Central Processing Unit (CPU)
	- Fetch Instructions from Memory
	- Decode Instructions via Masking
	- Map Instructions to their function handlers 
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "cpu.h"
#include "memory.h"
#include "Instructions.h"


int WAITING_FOR_SIGNAL = TRUE;			/* Flag to handle Signals */
extern struct PSW_BITS* PSWptr;			/* Pointer to PSW */
enum CPU_STATES state;					/* State of the CPU */
enum WORD_BYTE wb;						/* Determining Word or Byte type, based on Instruction Opcode */
extern union MEM_OLAY MEM;				/* Main Memory */
FILE* FOUT_INSTS;						/* Write Instructions to external file */
unsigned InstsProcessed = 0;			/* Count of number of Instructions executed */

/*
	System Clock.
	Range of Values: 0 to 4,294,967,295
*/
extern unsigned long SYS_CLK;

/*
	Initializing Register File.
	16-bit Registers.
	Range of Values: -32768 to 32767
*/
signed short REG_FILE[] = {
	0,	/* R0 - General Purpose Register */
	0,	/* R1 - General Purpose Register */
	0,	/* R2 - General Purpose Register */
	0,	/* R3 - General Purpose Register */
	0,	/* R4 - LR */
	0,	/* R5 - SP */
	0,	/* R6 - PSW */
	0	/* R7 - PC */
};

/* 16-bit Instruction Register */
unsigned short INST;							

/* Check for changes in Status of Devices */
extern void CheckDevices(void);

/* System Clock Breakpoint - set by Debugger */
extern unsigned long SYS_CLK_BREAKPOINT;


/* Branch Instructions functions pointer table */
void(*BRANCH_PTR[])(unsigned, ...) = {
	Process_BEQ,	/* Bits: (12, 11, 10) = 0 0 0; Same as BZ */
	Process_BNE,	/* Bits: (12, 11, 10) = 0 0 1; Same as BNE */
	Process_BC,		/* Bits: (12, 11, 10) = 0 1 0; Same as BHS */
	Process_BNC,	/* Bits: (12, 11, 10) = 0 1 1; Same as BLO */
	Process_BN,		/* Bits: (12, 11, 10) = 1 0 0 */
	Process_BGE,	/* Bits: (12, 11, 10) = 1 0 1 */
	Process_BLT,	/* Bits: (12, 11, 10) = 1 1 0 */
	Process_BAL		/* Bits: (12, 11, 10) = 1 1 1 */
};

/* Arithmetic Instructions functions pointer table */
void(*ARITHMETIC_PTR[])(unsigned, ...) = {
	Process_ADD,   /* Bits: (12, 11, 10, 9, 8) = 0 0 0 0 0 */
	none,
	Process_ADDC,  /* Bits: (12, 11, 10, 9, 8) = 0 0 0 1 0 */
	none,
	Process_SUB,   /* Bits: (12, 11, 10, 9, 8) = 0 0 1 0 0 */
	none,
	Process_SUBC,  /* Bits: (12, 11, 10, 9, 8) = 0 0 1 1 0 */
	none,
	Process_DADD,  /* Bits: (12, 11, 10, 9, 8) = 0 1 0 0 0 */
	none,
	Process_CMP,   /* Bits: (12, 11, 10, 9, 8) = 0 1 0 1 0 */
	none,
	Process_XOR,   /* Bits: (12, 11, 10, 9, 8) = 0 1 1 0 0 */
	none,
	Process_AND,   /* Bits: (12, 11, 10, 9, 8) = 0 1 1 1 0 */
	none,
	Process_BIT,   /* Bits: (12, 11, 10, 9, 8) = 1 0 0 0 0 */
	Process_SRA,   /* Bits: (12, 11, 10, 9, 8) = 1 0 0 0 1 */
	Process_BIC,   /* Bits: (12, 11, 10, 9, 8) = 1 0 0 1 0 */
	Process_RRC,   /* Bits: (12, 11, 10, 9, 8) = 1 0 0 1 1 */
	Process_BIS,   /* Bits: (12, 11, 10, 9, 8) = 1 0 1 0 0 */
	Process_SWPB,  /* Bits: (12, 11, 10, 9, 8) = 1 0 1 0 1 */
	Process_MOV,   /* Bits: (12, 11, 10, 9, 8) = 1 0 1 1 0 */
	Process_SXT,   /* Bits: (12, 11, 10, 9, 8) = 1 0 1 1 1 */
	Process_SWAP   /* Bits: (12, 11, 10, 9, 8) = 1 1 0 0 0 */
};


/*
	Handler for SIGINT (^c) Signal.
	Persists the state of the Signal and prevents program from running further.
	Note: In the "signal()" function call, the handler must be casted to "_crt_signal_t" type.
*/
void SignalHandler() {
	WAITING_FOR_SIGNAL = FALSE;
	signal(SIGINT, (_crt_signal_t)SignalHandler);
}


/* Print contents of Register File */
void PrintRegFile(void);



/*
	Emulation of the CPU.
	This is where the Fetch - Decode - Execute takes places
	- Machine goes in to and infinite loop in the FDE cycle.
	- Loop is broken if:
		-> "^c" Signal is detected
		-> Some condition, set by the debugger, is met
		-> PSW.SLP is set
	Note: Interrupts and Devices are handled at the end of each FDE cycle.
*/
void RunMachine(void) {

	unsigned int   type;							/* Save type of Inst */
	unsigned int   temp_type;						/* Save secondary type of Inst */

	state          = FETCH;							/* Initializing state of CPU */
	signal(SIGINT, (_crt_signal_t)SignalHandler);	/* Handler function for SIGNINT signals */
	InstsProcessed = 0;

	/* Open file to write output of Instructions to */
	if ((FOUT_INSTS = fopen("../Debug/InstructionsOutput.txt", "w")) == NULL)
		printf("Error: Could not open external file to write output of Instructins");

	/* 
		Run loop indefinitely, till at least one of the following conditions is met:
		- SIGINT Signal is detected ('^c' = !WAITING_FOR_SIGNAL)
		- CPU is asleep	(PSWptr->sleep = 1)
		Note: If CPU priority is 7 (Highest Priority), PSWptr->sleep CANNOT be set.
	*/
	while (WAITING_FOR_SIGNAL) {
		switch (state) {
			case FETCH:
				/* Get WORD from Memory */
				if ((INST = fetch()) == FALSE) {	/* If Instrution is not fetched */
					state = HANDLE_DEVICES;
					SYS_CLK++;
					//printf("No fetch\n");
				}
				else
					state = DECODE;
				//printf("PC = %04x	Inst = %04x\n", REG_FILE[PC], INST);
				fprintf(FOUT_INSTS, "PC = %4x	", REG_FILE[PC]);
				REG_FILE[PC] += 2;					/* Increment PC */
				break;
			case DECODE:
				type = INST_TYPE(INST);				/* Get Instruction Type */
				state = EXECUTE;
				break;
			case EXECUTE:
				switch (type) {
				case BRANCH_BL:
					Process_BL(BL_OFFSET(INST));
					break;
				case BRANCH:
					BRANCH_PTR[BRANCH_TYPE(INST)]((BRANCH_OFFSET(INST)));
					break;
				case ARITHMETIC:
					ARITHMETIC_PTR[ARITH_TYPE(INST)](ARITH_RC(INST), ARITH_WB(INST), ARITH_SRC(INST), ARITH_DST(INST));
					break;
				case LD_ST_MOVL_MOVLZ:
					temp_type = (INST & 0x1800) >> 11;		/* Extract bits 12 and 11 */
					switch (temp_type) {
					case LD:
						Process_LD(LD_ST_PRPO(INST), LD_ST_DEC(INST), LD_ST_INC(INST), LD_ST_WB(INST), LD_ST_SRC(INST), LD_ST_DST(INST));
						break;
					case ST:
						Process_ST(LD_ST_PRPO(INST), LD_ST_DEC(INST), LD_ST_INC(INST), LD_ST_WB(INST), LD_ST_SRC(INST), LD_ST_DST(INST));
						break;
					case MOVL:
						Process_MOVL(MOV_BYTE(INST), MOV_DST(INST));
						break;
					case MOVLZ:
						Process_MOVLZ(MOV_BYTE(INST), MOV_DST(INST));
						break;
					}
					break;
				case MOVH:
					Process_MOVH(MOV_BYTE(INST), MOV_DST(INST));
					break;
				case LDR:
					Process_LDR(LDR_STR_OFFSET(INST), LDR_STR_OFFSET_WB(INST), LDR_STR_OFFSET_SRC(INST), LDR_STR_OFFSET_DST(INST));
					break;
				case STR:
					Process_STR(LDR_STR_OFFSET(INST), LDR_STR_OFFSET_WB(INST), LDR_STR_OFFSET_SRC(INST), LDR_STR_OFFSET_DST(INST));
					break;
				}
				SYS_CLK++;	/* Increment SYS_CLK at the end of evey cycle */
				InstsProcessed++;	/* Cound of number of Instructions executed */
				state = HANDLE_DEVICES;
				break;
			case HANDLE_DEVICES:
				CheckDevices();
				state = FETCH;
				break;
		}

		/* SYS_CLK_BREAKPOINT is set by Debugger. Default value = 1000 */
		if (SYS_CLK > SYS_CLK_BREAKPOINT) {
			printf("\n");
			printf("Reached Breakpoint - %d\n", SYS_CLK);
			printf("Instructions Processed - %d\n", InstsProcessed);
			printf("\n");

			PrintRegFile();
			fclose(FOUT_INSTS);

			unsigned InstsProcessed = 0;
			break;
		}
	} /* End of while loop */

}



/*
	Fetch Instruction from location at PC
*/
unsigned short fetch(void) {
	unsigned short inst = FALSE;
	unsigned short eff_addr = REG_FILE[PC];		/* Effective Address */

	/*
		IF NOT Accessing HCF (0xFFFF)
			Read Memory through Bus.
			Note: Bus swaps LO and HI bytes to account for 'Little-Endian-ness'.
		ELSE
			- Pull LR
			- Pull PSW
			  PSW.SLP <- 0
			- Pull PC
	*/
	if (eff_addr != HCF) 
		MEM_RD(eff_addr, inst, WORD);			/* Read from Memory */
	else {	   /* Seeing HCF (0xFFFF) -> Must be returning from ISR */
		REG_FILE[SP] += 2;
		MEM_RD(REG_FILE[SP], REG_FILE[LR], WORD);
		REG_FILE[SP] += 2;
		MEM_RD(REG_FILE[SP], REG_FILE[PSW], WORD);
		PSWptr->sleep = 0;
		REG_FILE[SP] += 2;
		MEM_RD(REG_FILE[SP], REG_FILE[PC], WORD);

		MEM_RD(REG_FILE[PC], inst, WORD);		/* Read from Memory */
	}

	return inst;
}



/*
	Print contents of Register File
*/
void PrintRegFile(void) {
	int i = 0;
	printf("\n\n---------------------------------\n");
	fprintf(FOUT_INSTS, "\n\n---------------------------------\n");
	for (i = 0; i < 8; i++) {
		printf("REGISTER *%d* = *%04x* -> *%d*\n", 
				i, REG_FILE[i] & 0xFFFF, REG_FILE[i] & 0xFFFF);
		fprintf(FOUT_INSTS, "REGISTER *%d* = *%4x* -> *%d*\n", 
				i, REG_FILE[i] & 0xFFFF, REG_FILE[i] & 0xFFFF);
	}
	printf("---------------------------------\n");
	fprintf(FOUT_INSTS, "---------------------------------\n");
}
