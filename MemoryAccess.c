/*
Author: Emran Billah (em843554@dal.ca)
	Emulation of Cache and Primary Memory Access
*/
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "cpu.h"
#include "MemoryAccess.h"
#include "Queue.h"


/* Write Instructions to external file */
extern FILE* FOUT_INSTS;

/* Primary Memory */
extern union MEM_OLAY MEM;
extern unsigned long SYS_CLK;

/* Properties of Cache being used (set by the Debugger) */
extern struct CACHE_PROPERTIES CacheProperties;

/* 32 Cache Lines */
extern struct CACHE_LINE CACHE[MAX_CACHE_SIZE];

/* Read from or Write to Memory */
extern void bus(unsigned short address, unsigned short* data, unsigned int wb, unsigned int rw);

/* Cache Memory Queue */
extern Queue* CacheQueue;

/*
	Access contents at location specified by address in MAR from Cache
*/
void AccessCacheMem(unsigned short address, unsigned short* data, unsigned int wb, unsigned int rw) {
	enum CACHE_ACCESS_RESULT result = HIT;	/* Miss=0 or Hit=1 - Always assume HIT, unless modified otherwise */

	unsigned Index;							/* Cache Line number to search against */
	unsigned PageNumber;					/* Cache Page number to search against */
	unsigned PageStartIndex;				/* Cache Page starting Index */
	unsigned PageEndIndex;					/* Cache Page ending Index */

	/* Perform actions based on Organization type */
	switch (CacheProperties.ORG_TYPE) {
	/*
		- If Cache Organization is DIR_MAP type:   
			- Extract Bits 4 to 0 from Address in MAR -> this is the Cache Line Number to search.   
			- If address is ODD
				- If (address - 1) DOES NOT match CACHE[Index].address (Cache MISS) 
					- UPDATE CACHE
			- Else (address is EVEN)
				- If address DOES NOT match CACHE[Index].address (Cache MISS)
					- UPDATE CACHE
			- Save the Index of this Cache Line
			- READ OR WRITE CACHE
	*/
	case DIR_MAP:
		Index = (address) & 0x001F;							/* Bits 4 to 0 - Cache Line number to search against */
		if (address % 2) {									/* Address is ODD */
			if ((address - 1) != CACHE[Index].Address)		/* Cache Miss */
				result = MISS;
		}
		else {												/* Address is EVEN */
			if (address != CACHE[Index].Address)			/* Cache Miss */
				result = MISS;
		}

		if (result == MISS) {								/* Update Cache on MISS */
			UpdateCache(address, data, wb, rw, Index, result);
			fprintf(FOUT_INSTS, "***Cache MISS***\n");
		}
		else
			fprintf(FOUT_INSTS, "***Cache HIT!***\n");

		RW_CacheMem(address, data, wb, rw, Index);			/* Read from or Write to Cache */
		break;
	/*
		- Else If Organization type is ASC type:    
			- If address is ODD
				- If (address - 1) DOES NOT match address of any Cache Line in QUEUE
					- result <- Cache Miss
			- Else (If address is EVEN)
				- If address DOES NOT match address of any Cache Line in QUEUE
					- result <- Cache Miss
			- If result = Cache Miss	
				- UPDATE CACHE
			- MOVE Cache Line to TOP of QUEUE (Highest LRU Index)
			- READ OR WRITE CACHE
	*/
	case ASC:
		if (address % 2) {	/* If address is ODD */
			if ((Index = IsInQueue(CacheQueue, address - 1)) == NULL) { /* Address does not exist in any Queued Line */
				result = MISS;
				Index = 0;	/* Cache Miss-> Put Data and Address to top of Queue */
			}
		}
		else {				/* If adddress is EVEN */
			if ((Index = IsInQueue(CacheQueue, address)) == NULL) {		/* Address does not exist in any Queued Line */
				result = MISS;
				Index = 0;	/* Cache Miss-> Put Data and Address to top of Queue */
			}
		}

		/*
			- If Address is not in Queue
				- Put data in CacheLine Index 0 (Cache Line 0 points to TOP of Queue)
		*/
		if (result == MISS) {								/* Update Cache on MISS */
			UpdateCache(address, data, wb, rw, Index, result);
			fprintf(FOUT_INSTS, "***Cache MISS***\n");
		}
		else
			fprintf(FOUT_INSTS, "***Cache HIT!***\n");

		Index = PushToQueue(CacheQueue, address, data);	/* Enqueue and get Index of Cache Line Enqueued */
		RW_CacheMem(address, data, wb, rw, Index);		/* Read from or Write to Cache */
		break;
	/*
		TODO: Hybrid Cache
	*/
	case HYB:
		//Index		   = -1;
		//PageNumber	   = (address & (0x001C)) >> 2;		/* Cache Page number to search against - Bits 4 to 2 */
		//PageStartIndex = (PageNumber * HYB_CACHE_PAGE_SIZE) - 1;
		//PageEndIndex   = ((PageNumber * HYB_CACHE_PAGE_SIZE) + HYB_CACHE_PAGE_SIZE) - 1;
		//for (int i = PageStartIndex; i <= PageEndIndex; i++) {
		//	if (address == CACHE[i].Address)			/* Cache HIT */
		//		Index = i;
		//}
		//if (Index < 0) {								/* Cache MISS */
		//	AccessPrimaryMem(address, data, wb, rw);	/* Access Primary Memory */
		//	result = MISS;
		//}
		//UpdateCache(address, data, wb, (signed int)PageNumber, result);	/* Update Cache Memory */
		break;
	}

	SYS_CLK += 1;		/* Increment System Clock on Memory Access - Half as long as primary memory */
};



/* 
	Access contents at location specified by address in MAR when Cache fails
*/
void AccessPrimaryMem(unsigned short address, unsigned short* data, unsigned int wb, unsigned int rw) {
	if (rw == READ) {
		if (wb == WORD)
			*data = (MEM.MEM_BYTE[address + 1] << 8) | MEM.MEM_BYTE[address];
		else
			*data = MEM.MEM_BYTE[address];
	}
	else {
		MEM.MEM_BYTE[address] = (*data) & 0x00FF;					/* Writing LO Byte */
		if (wb == WORD)
			MEM.MEM_BYTE[address + 1] = (*data >> 8) & 0x00FF;		/* Writing HI Byte */
	}

	SYS_CLK += 2;		/* Increment System Clock on Memory Access */
}



/*
	Update Cache Memory:
	- IF wb = BYTE
		- If addr is even
			- Fetch WORD from Primary Memory at location specified by addr 
			- Replace LO Byte with new Data
			(This NEW WORD is the data to save in Cache)
		- Else
			- Fetch WORD from Primary Memory at location specified by (addr-1)
			- Replace HI Byte with new Data
			(This NEW WORD is the data to save in Cache)

	- If ORG_TYPE = DIR_MAP
		- Save Index of Cache Line which maps current address (IndexToReplace)
	- Else If ORG_TYPE = ASC 
		// TODO: 
			Queuing mechanism, Doubly Linked List?
	- Else (If ORG_TYPE = HYB)
		// TODO: 
			Queuing mechanism, Doubly Linked List?
	- Replace data and addr of CacheLine[IndexToReplace] with new data and addr
*/
void UpdateCache(unsigned short address, unsigned short* data, unsigned int wb, 
	unsigned int rw, signed int CacheIndex, unsigned int hit_miss) {

	/* Access Primary Memory to retrieve data at address specified by MAR */
	AccessPrimaryMem(address, data, wb, rw);
	
	/* Preparing new Data */
	if (wb == BYTE) {
		if (address % 2)	/* If Address is ODD - *data is HI Byte */
			*data = ((*data & 0x00FF) << 8) | (MEM.MEM_BYTE[address - 1]);
		else				/* If Address is EVEN - *data is LO Byte */
			*data = (MEM.MEM_BYTE[address + 1] << 8) | (*data & 0x00FF);
	}

	/* Update values for Cache Line for Direct Mapped Cache */
	if (CacheProperties.ORG_TYPE == DIR_MAP) {
		CACHE[CacheIndex].Address = address;
		CACHE[CacheIndex].Data = *data;
		CACHE[CacheIndex].DirtyBit = FALSE;
	}
}



/* 
	Read from or Write to Cache 
	- If Control is set to READ
		- If address is ODD	(must be referring to a BYTE Access)
			- *data <- HIGH Byte of Cache Line Data
		- Else (address is EVEN)
			- *data <- WORD from Cache Line Data
			- If wb = BYTE
				- *data <- LOW Byte of Cache Line Data
	- Else (Control is set to WRITE)
		- If address is ODD	(must be referring to a BYTE Access)
			- HIGH Byte of Cache Line Data <- *data
		- Else (address is EVEN)
			- If wb = BYTE
				- LOW Byte of Cache Line Data <- *data
			- Else (wb = WORD)
				- WORD of Cache Line Data <- *data
		- Set Dirty Bit for this Cache Line
		- If Replacement Policy = Write Through
			- RETAIN CACHE CONSISTENCY
*/
void RW_CacheMem(unsigned short address, unsigned short* data, unsigned int wb, unsigned int rw, signed int CacheIndex) {
	
	if (rw == READ) {
		if (address % 2)								/* If address is ODD */
			*data = HI_BYTE(CACHE[CacheIndex].Data);	/* *data <- HIGH Byte from Cache */
		else {											/* If address is EVEN */
			*data = CACHE[CacheIndex].Data;				/* *data <- WORD from Cache Line */
			if (wb == BYTE)
				*data = LO_BYTE(*data);					/* *data <- LOW Byte from Cache Line */
		}
	}
	else {												/* Control set to WRITE */
		if (address % 2)								/* If If Address is ODD */
			CACHE[CacheIndex].Data = ((LO_BYTE(*data)) << 8) | (LO_BYTE(CACHE[CacheIndex].Data));
		else											/* If Address is EVEN */
		{
			CACHE[CacheIndex].Data = ((HI_BYTE(CACHE[CacheIndex].Data)) << 8) | (LO_BYTE(*data));
			if (wb == WORD)
				CACHE[CacheIndex].Data = *data;
		}

		/* Set Dirty Bit by default */
		CACHE[CacheIndex].DirtyBit = 1;

		/* Retain Cache consistency if Organization type is Write Through */
		if (CacheProperties.REPL_POL == WT)
			RetainCacheConsistency(address, data, wb, CacheIndex);
	}
}



/*
	Retain Cache Consistency
	- If address is ODD (must be referring to BYTE Access)
		- BYTE at specified Memory location <- *data
	- Else (address is EVEN)
		- If wb = BYTE
			- BYTE at specified Memory location <- *data
		- Else (wb = WORD)
			- WORD at specified Memory location <- *data
	- Clear Dirty Bit for Cache Line specified by 'CacheIndex'
*/
void RetainCacheConsistency(unsigned short address, unsigned short* data, unsigned int wb, signed int CacheIndex) {
	//if (address % 2)							/* If address is ODD */
	//	MEM.MEM_BYTE[address] = (*data) & 0x00FF;	/* Writing BYTE */
	//else {										/* If address is EVEN */
	//	MEM.MEM_BYTE[address] = (*data) & 0x00FF;					/* Writing LO Byte */
	//	if (wb == WORD)
	//		MEM.MEM_BYTE[address + 1] = (*data >> 8) & 0x00FF;		/* Writing HI Byte */
	//}

	AccessPrimaryMem(address, data, wb, WRITE);
	/* Cache Consistency retained -> Clear Dirty Bit for Cache Line */
	CACHE[CacheIndex].DirtyBit = 0;
}