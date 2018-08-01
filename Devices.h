/*
	Emulation of Memory Mapped Devices
*/

#pragma once

#define	VECTOR_BASE				0xFFC0	/* Starting Location of Device Vector addresses */

/*
	Device Type
*/
enum DEV_TYPE {
	OUTPUT=0, INPUT=1
};

/*
	Structure of Device Status/Control Register
*/
struct DEV_SCR {
	unsigned IE  : 1;			/* Bit-0 - Interrupt Enable */
	unsigned IO  : 1;			/* Bit-1 - Output = 0, Input = 1 */
	unsigned DBA : 1;			/* Bit-2 - For Output Devices: Data Buffer Available,
								For Input Devices:  Data Byte Available */
	unsigned OV  : 1;			/* Bit-3 - Overrun */
	unsigned RES : 4;			/* Bits 4 to 7 - Unused */
};

/*
	Structure of Interrupt Vectors
*/
struct ISR_VECTOR {
	struct PSW_BITS* DEV_PSW;	/* PSW of ISR at location 'n' */
	unsigned short   ISR_ADDR;	/* Addr (PC) of ISR at location 'n+1' */
};

/*
	Structure to define properties of Devices
*/
struct DEVICE {
	unsigned DEV_NUM;			/* Device number */
	unsigned IO;				/* Output = 0, Input = 1 */
	unsigned INC_TIME;			/* Time of Status Change */
	unsigned PROC_TIME;			/* Time required to process Device */
	unsigned DONE_TIME;			/* INCOMING_TIME + PROCESSING TIME */
	unsigned ACTIVE_OUT;		/* Flag to detect if output device is active */

	unsigned char     DATA;		/* Data associated with Device*/
	struct DEV_SCR    SCR;		/* 8-bit Status Control Register */
	struct ISR_VECTOR ISR_VECT;	/* Vector for device ISR */
};

/*
	Properties of next incoming signal for devices
*/
struct NEXT_DEV_SIGNAL {
	unsigned long INC_TIME;		/* Time of Status Change */
	unsigned      DEV_NUM;		/* Output = 0, Input = 1 */
	unsigned char DATA;			/* Data of Next Device */
};

/* Read information about next device from external stimuli file */
void ReadNextDeviceSignal(FILE*);

/* Initialize devices */
void InitDevices(void);

/* Emulation of closing devices - Close file to read from and write to */
void CloseDevices(void);

/* Access Device Memory */
void AccessDeviceMem(unsigned short, unsigned char*, unsigned int);

/* Print devices output to external file */
void PrintToDevicesFile(unsigned, unsigned char*, unsigned);

/* Handle Interrupts */
void HandleInterrupt(unsigned);
