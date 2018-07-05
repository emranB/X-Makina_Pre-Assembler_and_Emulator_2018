#pragma once

#define TRUE					1
#define FALSE					0

#define R0						0						/* Register 0 */
#define R1						1						/* Register 1 */
#define R2						2						/* Register 2 */
#define R3						3						/* Register 3 */
#define R4						4						/* Register 4 */
#define R5						5						/* Register 5 */
#define R6						6						/* Register 6 */
#define R7						7						/* Register 7 */

#define LR						4						/* Register 4 - LR */
#define SP						5						/* Register 5 - SP */
#define PSW						6						/* Register 6 - PSW */
#define PC						7						/* Register 7 - PC */

#define REG_WORD_MAX_VAL		16383					/* Max Register value in WORDS */
#define REG_WORD_MIN_VAL		-16385					/* Min Register value in WORDS */

#define REG_BYTE_MAX_VAL		32767					/* Max Register value in BYTES */
#define REG_BYTE_MIN_VAL		-32768					/* Min Register value in WORDS */

#define WORD_MSB(x)				((x) & 0x8000) >> 15	/* Extract bit-15 (MSB) */			
#define WORD_LSB(x)				((x) & 0x0001)			/* Extract bit-0 (LSB) */		

#define BYTE_MSB(x)				((x) & 0x0080) >> 7		/* Extract bit-8 (MSB) */		
#define BYTE_LSB(x)				((x) & 0x0001)			/* Extract bit-0 (LSB) */		

#define HI_BYTE(x)				((x) & 0xFF00) >> 8		/* Extract high Byte from Word */
#define LO_BYTE(x)				((x) & 0x00FF)			/* Extract low Byte from Word */

#define HCF						0xFFFF					/* Highest Memory Location */ 

/*
	Sub-categorizing Instructions, based on 3-MSB
*/
#define INST_TYPE(x)			(((x) & 0xE000) >> 13)	/* Extract 3-MSB of Inst */

/*
	BL Instruction Definitions
*/
#define BL_OFFSET(x)			((x) & 0x1FFF)			/* Extract 13-LSB Offset */

/*
	Branch Instructions Definitions
*/
#define BRANCH_TYPE(x)			(((x) & 0x1C00) >> 10)	/* Extract bits 12 to 10 */
#define BRANCH_OFFSET(x)		((x) & 0x03FF)			/* Extract 10-bit Offset */

/*
Arithemetic Instructions Definitions
*/
#define ARITH_TYPE(x)			(((x) & 0x1F00) >> 8)	/* Extract bits 12 to 8 - Type */
#define ARITH_RC(x)				((x) & 0x0080)			/* Extract bit 7		- Register or Constant */
#define ARITH_WB(x)				((x) & 0x0040)			/* Extract bit 6		- Word or Byte */
#define ARITH_SRC(x)			((x) & 0x0031)			/* Extract bits 5 to 3  - Source or Constant */
#define ARITH_DST(x)			((x) & 0x0007)			/* Extract bits 5 to 3  - Source or Constant */

/*
LD and ST Instructions Definitions
*/
#define LD_ST_PRPO(x)			(((x) & 0x0400) >> 10)	/* Extract bit 10		- Pre or Post */
#define LD_ST_DEC(x)			(((x) & 0x0200) >> 9)	/* Extract bit 9		- Decrement */
#define LD_ST_INC(x)			(((x) & 0x0100) >> 8)	/* Extract bit 8		- Increment */
#define LD_ST_WB(x)				(((x) & 0x0040) >> 6)	/* Extract bit 6		- Word or Byte */
#define LD_ST_SRC(x)			(((x) & 0x0038) >> 3)	/* Extract bits 5 to 3  - Src */
#define LD_ST_DST(x)			((x) & 0x0007)			/* Extract bits 2 to 0  - Dst */

/*
	MOVL, MOVLZ, MOVH Instructions Definitions
*/
#define MOV_BYTE(x)				(((x) & 0x07F8) >> 3)	/* Extract bits 10 to 3	- Offset */
#define MOV_DST(x)				((x) & 0x0007)			/* Extract bits 2 to 0	- Destination */

/*
	LDR and STR Instructions Definitions
*/
#define LDR_STR_OFFSET(x)		(((x) & 0x1F00) >> 7)	/* Extract bits 12 to 7	- Offset */
#define LDR_STR_OFFSET_WB(x)	(((x) & 0x0007) >> 6)	/* Extract bit 6		- Word or Byte */
#define LDR_STR_OFFSET_SRC(x)	(((x) & 0x0038) >> 3)	/* Extract bit 5 to 3	- Src */
#define LDR_STR_OFFSET_DST(x)	((x) & 0x0007)			/* Extract bits 2 to 0	- Dst */



/*
	Defining R6 - Program Status Word (PSW).
	16-bit Register.
*/
struct PSW_BITS {
	unsigned
		carry : 1,
		zero : 1,
		negative : 1,
		sleep : 1,
		overflow : 1,
		priority : 3,
		unused : 8;
};

/* 
	Fetch Instruction Bytes from Memory 
*/
unsigned short fetch(void);		

/* 
	States of the CPU 
*/
enum CPU_STATES { FETCH, DECODE, EXECUTE, HANDLE_INTERRUPT, INST_ERROR, DONE };		

/*
	Instruction Opcode Bits for Word or Byte
*/
enum WORD_BYTE { WORD = 0, BYTE = 1 };

/*
	Instruction Action - Read or Write
*/
enum READ_WRITE { READ = 0, WRITE = 1 };

/*
	Types of Instructions
*/
enum INST_TYPES { BRANCH_BL=0, BRANCH=1, ARITHMETIC=3, LD_ST_MOVL_MOVLZ=4, MOVH=5, LDR=6, STR=7 };
enum LD_ST_MOVL_MOVLZ_TYPES { LD=0, ST=1, MOVL=2, MOVLZ=3 };

/*
	Register or Contant
*/
enum RC { REGISTER=0, CONSTANT=1 };

/*
	Instruction Emulation function handlers
*/
/* @input: 13-bit offset */
extern void Process_BL(signed short);

/* @input: 10-bit offset */
extern void Process_BEQ(signed short);
extern void Process_BNE(signed short);
extern void Process_BC(signed short);
extern void Process_BNC(signed short);
extern void Process_BN(signed short);
extern void Process_BGE(signed short);
extern void Process_BLT(signed short);
extern void Process_BAL(signed short);

/* @input: 1-bit Register or Constant, 1-bit Word or Byte, 3-bit Source, 3-bit Destination */
extern void Process_ADD(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_ADDC(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_SUB(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_SUBC(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_DADD(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_CMP(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_XOR(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_AND(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_BIT(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_BIC(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_BIS(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_MOV(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_SWAP(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_SRA(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_RRC(unsigned char, unsigned char, unsigned int, unsigned int);

extern void Process_SWPB(unsigned char, unsigned char, unsigned int, unsigned int);
extern void Process_SXT(unsigned char, unsigned char, unsigned int, unsigned int);
extern void none(unsigned char, unsigned char, unsigned int, unsigned int);

/* @input: 1-bit Pre or Post, 1-bit Decrement, 1-bit Increment, 1-bit Word or Byte, 3-bit Source, 3-bit Destination */
void Process_LD(unsigned char, unsigned char, unsigned char, unsigned char, unsigned int, unsigned int);
void Process_ST(unsigned char, unsigned char, unsigned char, unsigned char, unsigned int, unsigned int);

/* @input: 8-bit Data Byte,3-bit Destination */
void Process_MOVL(unsigned char, unsigned int);
void Process_MOVLZ(unsigned char, unsigned int);
void Process_MOVH(unsigned char, unsigned int);

/* @input: 5-bit Offset, 1-bit Word or Byte, 3-bit Source, 3-bit Destination */
void Process_LDR(unsigned char, unsigned char, unsigned char, unsigned char);
void Process_STR(unsigned char, unsigned char, unsigned char, unsigned char);






