/*
	Emulation of primary memory
*/

#define MAX_MEM_SIZE_WORD			(1<<15)		/* Maximum size of memory: 0 to 32768 WORDS */
#define MAX_MEM_SIZE_BYTE			(1<<16)		/* Maximum size of memory: 0 to 65535 BYTES */
#define DEVICES_COUNT				8			/* Number of Devices */
#define	DEVICES_VECTOR_BASE			0xFFCO		/* Start of Devices Vectors Memory Location */
#define MEM_RD(addr, data, bw)		bus(addr, (unsigned short*)(&data), bw, READ)		/* Read from Memory*/	
#define MEM_WR(addr, data, bw)		bus(addr, (unsigned short*)(&data), bw, WRITE)		/* Write to Memory */

/*
	Structure of Memory
	Accessed as Words of Bytes
*/
union MEM_OLAY {
	unsigned short MEM_WORD[MAX_MEM_SIZE_WORD];
	unsigned char MEM_BYTE[MAX_MEM_SIZE_BYTE];
};

