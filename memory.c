#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "cpu.h"


/*
	Initializing Primary Memory.
	Array of Bytes or Words.
	Range: 0 to 65535 Bytes, or, 0 to 32768 Words
*/
union MEM_OLAY MEM;
extern unsigned long SYS_CLK;


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
	}
	else {
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
	}

	SYS_CLK+=2;		/* Increment System Clock on Memory Access */
}

