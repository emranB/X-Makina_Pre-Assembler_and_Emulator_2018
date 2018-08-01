/*
Author: Emran Billah (em843554@dal.ca)
	Emulation of Device, Cache and Primary Memory
	- Read or Write to Memory
*/
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "cpu.h"
#include "Devices.h"
#include "Queue.h"


/* Read from or Write to Memory */
extern void bus(unsigned short address, unsigned short* data, unsigned int wb, unsigned int rw);

/* Access Cache Memory */
extern void AccessPrimaryMem(unsigned short, unsigned short*, unsigned int, unsigned int);

/* Access Primary Memory */
extern void AccessCacheMem(unsigned short, unsigned short*, unsigned int, unsigned int);

/* Update contents of Cache Memory */
extern void UpdateCache(unsigned short, unsigned short*, unsigned int, unsigned int, signed int, unsigned int);

/* Read from or Write to Cache */
extern void RW_CacheMem(unsigned short, unsigned short*, unsigned int, unsigned int, signed int);

/* Retain Cache Consistency */
extern void RetainCacheConsistency(unsigned short, unsigned short*, unsigned int, signed int);

/*
	Initializing Primary Memory.
	Array of Bytes or Words.
	Range: 0 to 65535 Bytes, or, 0 to 32768 Words
*/
extern union MEM_OLAY MEM;
extern unsigned long SYS_CLK;

/* Properties of Cache being used (set by the Debugger) */
extern struct CACHE_PROPERTIES CacheProperties;

/* Initialize 32 Cache Lines */
extern struct CACHE_LINE CACHE[MAX_CACHE_SIZE];

/* Access Cache Memory */
extern void AccessCacheMem(unsigned short, unsigned short*, unsigned int, unsigned int);

/* Access Primary Memory */
extern void AccessPrimaryMem(unsigned short, unsigned short*, unsigned int, unsigned int);

/* Cache Memory Queue */
Queue* CacheQueue;

/*
	Initialize Cache Memory
	- Give each line and ivalid address values (HCF).
	- Set Default Organization Type  -> Write Through
	- Set Default Repalcement Policy ->  Direct Mapped
*/
void InitMemory(void) {
	CacheProperties.ACTIVE   = FALSE;
	CacheProperties.ORG_TYPE = WT;
	CacheProperties.REPL_POL = DIR_MAP;

	/* Create Queue for ASC and HYB Cache */
	CacheQueue = CreateQueue(MAX_CACHE_SIZE);

	/* 
		If Orgatnization type is not Direct Mapped
		- Make Cache Lines Point to Queue
	*/
	if (CacheProperties.ORG_TYPE != DIR_MAP)
		CacheQueue->Top = CACHE;
}



/* 
	Read from or Write to Memory.
	Locations 0000 to 000F are reserved for Memory Mapped Devices
	@params		address		Value of MAR
				data		Value of MBR
				wb			Indicator of Word (0) or Byte (1)
				rw			Indicator of Read or Write action
	@return		void
*/
void bus(unsigned short address, unsigned short* data, unsigned int wb, unsigned int rw) {
	if (address < (2 * DEVICES_COUNT)) {
		/* 
			Accessing Memory Mapped Devices.
			Notice that only writing to either the SCR addr or Data addr
			is allowed at a time, ie. writing Words are not supported.
			Hence, data is casted to "unsigned char*" type.
		*/
		AccessDeviceMem(address, (unsigned char*) data, rw);
		SYS_CLK += 2;		/* Increment System Clock on Memory Access */
	}
	else {
		/* 
			Access Cache Memory if Cache is Active,
			Otherwise, Access Primary Memory
		*/
		if (CacheProperties.ACTIVE)
			AccessCacheMem(address, data, wb, rw);
		else
			AccessPrimaryMem(address, data, wb, rw);
	}
}

