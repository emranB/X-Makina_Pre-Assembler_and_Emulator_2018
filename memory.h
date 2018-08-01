#pragma once
/*
	Emulation of primary memory
*/

#define MAX_MEM_SIZE_WORD			(1<<15)		/* Maximum size of memory: 0 to 32768 WORDS */
#define MAX_MEM_SIZE_BYTE			(1<<16)		/* Maximum size of memory: 0 to 65535 BYTES */
#define MAX_CACHE_SIZE				32			/* Maximum size of cache */
#define HYB_CACHE_PAGE_SIZE			4			/* Size of each Page in Hybrid Cache */
#define DEVICES_COUNT				8			/* Number of Devices */
#define	DEVICES_VECTOR_BASE			0xFFCO		/* Start of Devices Vectors Memory Location */
#define MEM_RD(addr, data, bw)		bus(addr, (unsigned short*)(&data), bw, READ)		/* Read from Memory*/	
#define MEM_WR(addr, data, bw)		bus(addr, (unsigned short*)(&data), bw, WRITE)		/* Write to Memory */

/*
	Structure of Memory
	Accessed as Words or Bytes
*/
union MEM_OLAY {
	unsigned short MEM_WORD[MAX_MEM_SIZE_WORD];
	unsigned char MEM_BYTE[MAX_MEM_SIZE_BYTE];
};

/*
	Properties of Cache Memory
	- Active -> Flag set by debugger to indicate whether or not to use Cache, Default: False
	- Organization type to be used -> Set by debugger,  Default: Direct Mapped (DIR_MAP)
	- Replacement Policy to be used -> Set by debugger, Default: Write Through
*/
struct CACHE_PROPERTIES {
	unsigned ACTIVE;
	unsigned ORG_TYPE;
	unsigned REPL_POL;
};

/* 
	Structure of Cache Lines 
	- 16-bit Address in MAR
	- 16-bit Buffer or Data from MBR
	- 1-bit Dirty Bit
*/
struct CACHE_LINE {
	unsigned short Address;
	signed short Data;
	unsigned DirtyBit;
};

/*
	Cache Organization types
	- Direct Mapped (DIR_MAP)
	- Associative (ASC)
	- Hybrid (HYB)
*/
enum CACHE_ORG_TYPE {
	DIR_MAP = 0,
	ASC		= 1,
	HYB		= 2
};

/* 
	Cache Replacement Policies
	- Write Through (WT)
	- Write Back (WB)
*/
enum CACHE_REPL_POL {
	WT = 0,
	WB = 1
};

/*
	Cache Results
*/
enum CACHE_ACCESS_RESULT {
	MISS = 0,
	HIT  = 1
};
