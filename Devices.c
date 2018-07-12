#include <stdio.h>
#include <stdlib.h>
#include "cpu.h"
#include "memory.h"
#include "Devices.h"

#define DEVICES_CNT			8			/* Max number of Devices */
#define LINE_SZ				256			/* Max size of line to read */
#define DELIMITERS			" \t\n"		/* Delimiters to tokenize line with */

/* Initializing Array of 8-Devices */
struct DEVICE Devices[DEVICES_CNT];

/* Next Incoming Sginal for Devices */
struct NEXT_DEV_SIGNAL NextDevSig;

/* Memory Access (Global) */
extern union MEM_OLAY MEM;

/* System Clock (Global) */
extern unsigned long SYS_CLK;

/* Register File  */
extern signed short REG_FILE[];	

/* Current Program PSW  */
extern struct PSW_BITS* PSWptr;

FILE* fp_IN;
FILE* fp_OUT;
char  LINE[LINE_SZ];			/* Line read from devices file */
char* token;					/* Tokens of Lines being read */



/*
	Read configuration of all (8) devices.
	Read the information of the first device only.
*/
void InitDevices(void) {
	/* Open external file to write output of devices to */
	if ((fp_OUT = fopen("../Debug/DevicesOutput.txt", "w")) == NULL)
		printf("Error opening Devices output file!\n");

	/* Printing column headers to external file */
	fprintf(fp_OUT, "-----------------------------------------------------------------------------\n");
	fprintf(fp_OUT, "Program Counter	|	System Clock	|	Device #	|	Data	|	Action\n");
	fprintf(fp_OUT, "-----------------------------------------------------------------------------\n");

	/* Read configuration of devices from external file */
	if ((fp_IN = fopen("IO/stimuli.txt", "r")) == NULL)
		printf("Error opening Devices input file!\n");

	unsigned dev = 0;		/* First device*/

	/* 
		Read first 8-lines of devices file.
		First column: Output = 0, Input = 1.
		Second column: Processing Time.
	*/
	while (dev < DEVICES_CNT) {
		fgets(LINE, LINE_SZ, fp_IN);
		token				   = strtok(LINE, DELIMITERS);
		Devices[dev].DEV_NUM   = atoi(token);
		token				   = strtok(NULL, DELIMITERS);
		Devices[dev].IO		   = atoi(token);
		token				   = strtok(NULL, DELIMITERS);
		Devices[dev].PROC_TIME = atoi(token);
		dev++;
	}

	/* Read and save information about first device */
	ReadNextDeviceSignal(fp_IN);
}



/*
	Check for status change in Devices.
	If Set/Reset IE, DBA and/or OV Bits in SCR.
	Memory mapped devices can access memory directly (without BUS)
*/

void CheckDevices(void) {
	unsigned dev;
	struct DEV_SCR* dev_scr;
	unsigned short dev_scr_addr  = (NextDevSig.DEV_NUM) * 2;
	unsigned short dev_data_addr = dev_scr_addr + 1;

	/*
		If next incoming device signal exists and 
		time has surpassed, process the device.
		- Get SCR of the dev
		- Modify bits as required
		- Copy incoming data to device data location
	*/
	if (NextDevSig.INC_TIME && SYS_CLK > NextDevSig.INC_TIME) {
		dev_scr = (struct DEV_SCR *) &MEM.MEM_BYTE[dev_scr_addr];
		AccessDeviceMem(dev_data_addr, &(NextDevSig.DATA), WRITE);
		/* Read and save information about next device */
		ReadNextDeviceSignal(fp_IN);
	}

	/*
		Check if any output device has finished writing
		- If the device is an input
			- Carry out Interrupt subroutine if IE = 1
		- If the device type is output, and system clock 
		  has surpassed its Incoming + Processing time
			- Print data to external file
			- Set DBA and OV to 0
			- Handle Interrupts
	*/
	for (dev = 0; dev < DEVICES_CNT; dev++) {
		/* Get this device's scr */
		dev_scr = (struct DEV_SCR *) &MEM.MEM_BYTE[dev * 2];

		/* If device is an Input, check for IE bit */
		if (Devices[dev].IO == INPUT && dev_scr->DBA == 1) {
			if (dev_scr->IE) {
				HandleInterrupt(dev);
			}
		}

		/* If device is an Active Ouput and has finished processing */
		if ((SYS_CLK >= (Devices[dev].DONE_TIME)) && (Devices[dev].ACTIVE_OUT)) {
			dev_data_addr = (dev * 2) + 1;
			printf("Sys_Clk = %d, Dev End Time = %d, **Device %d says %s**\n", 
					SYS_CLK, Devices[dev].DONE_TIME, dev, &(MEM.MEM_BYTE[dev_data_addr]));
			PrintToDevicesFile(dev, &(MEM.MEM_BYTE[dev_data_addr]), WRITE);

			/* Done writing -> Data Buffer available again */
			(dev_scr->DBA == 1) ? (dev_scr->OV = 1) : (dev_scr->DBA = 1);
			Devices[dev].ACTIVE_OUT = FALSE;

			if (dev_scr->IE)
				HandleInterrupt(dev);
			
		}
	}

	printf("Sys Clk = %d - ok up to here\n", SYS_CLK); 
	getchar();
}



/*
	Handle Interrupt according to Priority
	@params unsigned int Device number (Index)
	@return void
*/
void HandleInterrupt(unsigned dev) {
	/* Location of Device Vector */
	unsigned short EA = VECTOR_BASE + (4 * dev);
	
	/* Reading Device PSW */
	unsigned char  dev_psw_lo   = MEM.MEM_BYTE[EA];
	unsigned char  dev_psw_hi   = MEM.MEM_BYTE[EA+1];
	unsigned short dev_psw_val  = (dev_psw_hi << 8) | dev_psw_lo;
	struct PSW_BITS* DEV_PSW    = (struct PSW_BITS *) (&dev_psw_val);

	/* Reading entry point (PC) of the Device's ISR */
	unsigned char  dev_isr_lo   = MEM.MEM_BYTE[EA+2];
	unsigned char  dev_isr_hi   = MEM.MEM_BYTE[EA+3];
	unsigned short DEV_ISR_ADDR = (dev_isr_hi << 8) | dev_isr_lo;

	/*
		NOTE: ALL THE PUSHES AND PULLS GO THROUGH THE BUS
		AND INCREMENT SYSTEM CLOCK
		- Push PC
		- Push PSW
		- Push LR
		- PSW <- PSW of handler (memory[Vector Address])
		  PSW.SLP <- 0
		- PC <- Address of handler (memory[Vector Address + 2])
		- LR <- $FFFF
	
		Decrement stack by 2 after every push, to keep it 'growing' 
	*/
	MEM_WR(REG_FILE[SP], REG_FILE[PC], WORD);
	REG_FILE[SP] -= 2;
	MEM_WR(REG_FILE[SP], REG_FILE[PSW], WORD);
	REG_FILE[SP] -= 2;
	MEM_WR(REG_FILE[SP], REG_FILE[LR], WORD);
	REG_FILE[SP] -= 2;

	PSWptr		  = DEV_PSW;
	PSWptr->sleep = 0;

	REG_FILE[PC] = DEV_ISR_ADDR;
	REG_FILE[LR] = 0xFFFF;
	printf("Done ISR!\n");
}



/*
	Emulation of closing devices
*/
void CloseDevices(void) {
	fclose(fp_IN);
	fclose(fp_OUT);
}



/*
	Access Device Memory.
	Notice that only writing to either the SCR addr or Data addr
	is allowed at a time, ie. writing Words are not supported.
*/
void AccessDeviceMem(unsigned short address, unsigned char* data, unsigned int rw) {

	/* Get Device number */
	unsigned dev_num = address / 2;

	/* Get Device SCR */
	unsigned short dev_scr_addr = dev_num * 2;
	struct DEV_SCR* dev_scr = (struct DEV_SCR*) &MEM.MEM_BYTE[(dev_num * 2)];

	/* 
		If accessing Device SCR,
		- Allow programmer to write to any bit
	*/
	if (address == dev_scr_addr) {
		if (rw == READ)
			*data = MEM.MEM_BYTE[dev_scr_addr];
		else
			MEM.MEM_BYTE[dev_scr_addr] = *data;
	}
	/*
		Else, If accessing Device Data,
		- For Input Devices
			-> If data is Read
				- Clear DBA and OV bits
		- For Output Devices
			-> If data is Written
				- Set DBA and/or OV bits
	*/
	else {
		if (Devices[dev_num].IO == INPUT) {
			if (rw == READ) {
				*data = MEM.MEM_BYTE[address];
				dev_scr->DBA = 0;
				dev_scr->OV = 0;
				PrintToDevicesFile(dev_num, *data, READ);
			}
		}
		else { /* Output Device */
			if (rw == WRITE) {
				MEM.MEM_BYTE[address] = *data;
				//(dev_scr->DBA == 1) ? (dev_scr->OV = 1) : (dev_scr->DBA = 1);
				/* Data Buffer not available */
				dev_scr->DBA = 0;
				dev_scr->OV = 0;

				Devices[dev_num].ACTIVE_OUT = TRUE;
				Devices[dev_num].DONE_TIME = SYS_CLK + Devices[dev_num].PROC_TIME;
			}
		}
	}
	
}



/*
	Read next device signal (first row) info only.
	Save info as NextDev by tokenizing each column.
	- First token  -> Incoming Time
	- Second token -> Index of Device being triggered
	- Third token  -> Data for the Device
*/
void ReadNextDeviceSignal(FILE* fp) {

	/* If no devices are left to read, set INC_TIME to FALSE */
	NextDevSig.INC_TIME = FALSE;

	if (fgets(LINE, LINE_SZ, fp) != NULL) {
		token = strtok(LINE, DELIMITERS);
		NextDevSig.INC_TIME = atoi(token);
		token = strtok(NULL, DELIMITERS);
		NextDevSig.DEV_NUM = atoi(token);
		token = strtok(NULL, DELIMITERS);
		NextDevSig.DATA = *token;

		/* Save the Device's Incoming Time */
		struct DEVICE* dev = &Devices[NextDevSig.DEV_NUM];
		dev->INC_TIME = NextDevSig.INC_TIME;
	}
}



/*
	Print devices output to external file
*/
void PrintToDevicesFile(unsigned DEV_NUM, unsigned char* DATA, unsigned RD_or_WR) {
	fprintf(fp_OUT, "	%4x", REG_FILE[PC]);
	fprintf(fp_OUT, "				%d",  SYS_CLK);
	fprintf(fp_OUT, "				%d",  DEV_NUM);
	fprintf(fp_OUT, "				%s",  DATA);
	((RD_or_WR) == READ ) ? fprintf(fp_OUT, "		READ") : fprintf(fp_OUT, "		    WRITE");
	fprintf(fp_OUT, "\n");
}