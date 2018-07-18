/*
	Instruction Emulation function handlers
*/
/* @input: 13-bit offset */
void Process_BL(signed short);

/* @input: 10-bit offset */
void Process_BEQ(signed short);
void Process_BNE(signed short);
void Process_BC(signed short);
void Process_BNC(signed short);
void Process_BN(signed short);
void Process_BGE(signed short);
void Process_BLT(signed short);
void Process_BAL(signed short);

/* @input: 1-bit Register or Constant, 1-bit Word or Byte, 3-bit Source, 3-bit Destination */
void Process_ADD(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_ADDC(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_SUB(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_SUBC(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_DADD(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_CMP(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_XOR(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_AND(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_BIT(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_BIC(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_BIS(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_MOV(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_SWAP(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_SRA(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_RRC(unsigned char, unsigned char, unsigned int, unsigned int);

void Process_SWPB(unsigned char, unsigned char, unsigned int, unsigned int);
void Process_SXT(unsigned char, unsigned char, unsigned int, unsigned int);
void none(unsigned char, unsigned char, unsigned int, unsigned int);

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