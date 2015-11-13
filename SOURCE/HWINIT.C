/*
 * File: $Header:  \ibm16\hwinit.c  2-2-93  jess fahland $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file contains hardware and driver specific init code that will
 * 	disappear after driver init time.
 */
#include <stdio.h>
#include <conio.h>
#include "giexec.h"
#include "misc.h"
#include "uprim.h"
#include "hwinit.h"
#include "hwbuff.h"
#include "print.h"
#include "strings.h"
#include "mca.h"
#include "dialogs.h"  

#define NUMGDTS	(1+MAXPENDING)
WORD Get_MiscAT(void);
WORD	gdtAllocArray[NUMGDTS];
struct AIPInfo aip;
extern void getDgroupPhy(void);
extern WORD swaps(WORD);
extern void _enableTicker(void);
extern void _disableTicker(void);


void setupOpenParms(WORD Rcv_Size, WORD Rcv_Num, WORD Xmit_Size, BYTE Xmit_Num);

extern WORD TAPFlag;
BYTE far *RAMAddress;         /* Holds shared RAM GDT */
BYTE far *MMIOAddress;        /* Holds MMIO GDT */
WORD InitRAMAddr;             /* Shared RAM address during init time */
WORD InitMMIOAddr;            /* MMIO address during init time */
extern WORD GDTSelector;      /* tempory GDT holder */
WORD PIOBase;                 /* Base address of I/O registers */
WORD Data_Rate;               /* Current data rate of adapter */
WORD HW_Mchan;                /* Determines if this is  microchannel */
WORD IRQEnableReg;

uchar PosReg2;                /* These hold the POS register values */
uchar PosReg3;
uchar PosReg4;
uchar IntType;


/*
 * Function: environmentInit
 *
 * Inputs:
 *		none
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 *		This function parses the device drive init string looking for
 *		init time parameters. The only parameter supported by this driver
 *		is  /P:. Any string immediately following /P: is interpreted as
 *		the new name for the protocol manager driver device name. For example:
 *
 *		DEVICE=IBM16.OS2 /P:NEWPROT$
 *
 */
WORD
environmentInit(void)
{
	register char *s = ndisGen.devInitStr;
	register char *s2;

	/*
	 * look for /P:
	 */
	while (s) {
		/*
		 * find a '/', if none then just exit.
		 */
		if (!(s=strchr(s,'/'))) {
			return(SUCCESS);
		}

		/*
		 * does this match '/P:' ?
		 */
		if (fStrncmpi((char far *)s, (char far *)"/P:", 3)) {

			/*
			 * keep going if this is not yet a NULL string
			 */
			if (*(++s))
				continue;
			else
				return(SUCCESS);
		}

		/*
		 * set the pointer to the first character of the new protman name
		 */
		s += 3;

		/*
		 * scan for the first NULL or whitespace character and NULL terminate
		 */
		s2 = s;
		while (*s2 && *s2 != ' ' && *s2 != '\t')
			s2++;
		*s2 = '\0';

		/*
		 * set the pointer to the protman name.
		 */
		protMgrName = s;
		break;
	}
	return(SUCCESS);
}

/*
 * Function: hwConfig( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * perform hw config prior to processParms. (not used.)
 */
WORD
hwConfig()
{

	/*
	 * Calculate our dgroup physical base address. This simplifies
	 * physical address calculations.
	 */
	getDgroupPhy();

	/*
	 * get the system configuration. most important is if this is a PS/2.
	 * The side effect is that the SC_MICROCHAN bit in SCFeatures gets set
	 * if it is a PS/2.
	 */

   mChanInit();
   return(SUCCESS);
}

/*
 * Function: getBIA( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * getBIA() - 	Get Burned In Address.
 *		The BIA is read from the adapter and copied to the MSC permanent
 *    and current	station address.
 */
void
getBIA(void)
{
    BYTE bia[6];
    WORD addroffset;
    BYTE far *tmpaddr;
    int	i;
    ulong tmpphys;


   /*
    * If we already have an address just copy that.
    */
    if ( op.nodadr[0] == 0  &&  op.nodadr[1] == 0  &&
	            op.nodadr[2] == 0  &&  op.nodadr[3] == 0  &&
                	op.nodadr[4] == 0  &&  op.nodadr[5] == 0 ) {

         /*
          * Get a LDT selector pointing to our MMIO
          */
         tmpphys = (ulong)InitMMIOAddr << 4;
         tmpaddr = (BYTE far *)physToUVirt(tmpphys, 8192L, 1);

         addroffset = 0x1F00;

         /*
          * decode the address in the AIP area
          */
         for (i = 0; i < 6; i++)
         {
            bia[i] = (BYTE)*(tmpaddr + addroffset);
            addroffset += 2;
            bia[i] <<= 4;
            bia[i] &= 0xF0;
            bia[i] |= (BYTE)*(tmpaddr + addroffset);
            addroffset += 2;
         }

         /*
          * Release the local descriptor.
          */
         physToUVirt((ulong)tmpaddr, 0L, 2);

	      _bcopy( (void far *)bia, (void far *)MSC.MscPermStnAdr, ADDR_SIZE);
    	   _bcopy( (void far *)bia, (void far *)MSC.MscCurrStnAdr, ADDR_SIZE);

    } else {

		_bcopy( (void far *)op.nodadr, (void far *)MSC.MscPermStnAdr,
			ADDR_SIZE);
		_bcopy( (void far *)op.nodadr, (void far *)MSC.MscCurrStnAdr,
			ADDR_SIZE);

    }

	if (debug & DBG_VERBOSE) {
		printf("getBIA:\n");
		printData((void far *)MSC.MscPermStnAdr, ADDR_SIZE);
	}
}

extern DWORD *keptStats[];
extern OPEN_PARAMS op;

/*
 * Function: setupOpenParms( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 *  Establish open parameters for the hardware.  Memory Size on the card has
 *   been determined automatically by this time.
 */
void
setupOpenParms(WORD Rcv_Size, WORD Rcv_Num, WORD Xmit_Size, BYTE Xmit_Num)
{
   int i;

	if (debug) {
		printf("setupOpenParms: rcvbuffsz %d xmitbuffsz %d numrvcbuffs %d numxmitbuffs %d\n",
			Rcv_Size,
			Xmit_Size,
			Rcv_Num,
			Xmit_Num);
	}
	/*** All open parameter word values are byte-swapped. ***/

	/* Set Adapter buffer size to 256 bytes (264-8) */
   op.command = 0x03;
   op.rcv_size = swaps(Rcv_Size);
   op.rcv_num = swaps(Rcv_Num);
   op.xmt_size = swaps(Xmit_Size);
   op.xmt_num = Xmit_Num;

	/*
    * Fill in MSC table values.
	 */
   if (TAPFlag)
   {
      if (Data_Rate == 16)
         MSC.MscMaxFrame = 17684;
      else
         MSC.MscMaxFrame = 4458;
   }
   else
	   MSC.MscMaxFrame = 	Xmit_Size - 6;
	MSC.MscTBufCap	=	(DWORD)Xmit_Size - 6;
	MSC.MscTBlkSz	=	Xmit_Size - 6;
	/*
	 * receive capacity and block size
	 */

   MSC.MscRBufCap =  (DWORD)RCV_BUFF_CAP;
	MSC.MscRBlkSz	=	Rcv_Size - 8;
}

char *strType[] = { "16th", "15th", "14th", "13th", "12th", "11th", "10th",
                  "9th", "8th", "7th", "6th", "5th", "4th", "3rd", "2nd", "1st" };

char *strDatarates[] = { "None", "4/16 Mbps", "16 Mbps", "4 Mbps" };
char *strETR[] = { "4/16 Mbps", "16 Mbps", "4 Mbps", "None" };
char *strSRAM[] = { "", "", "64K (Top 512 Usable)", "64K (Top 512 Unusable)",
                  "32K", "8K", "N/A" };
char *strRAMPage[] = { "16K,32K page", "32K page", "16K page", "None" };
char *strDHB4[] = { "", "4464", "4096", "2048" };
char *strDHB16[] = { "", "", "", "17960", "16384", "8192", "4096", "2048" };

/*
 * Function: checkConfig( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * Check the function identifiers and checksums in the AIP and print out
 * relevent messages to the user.
 */
WORD
checkConfig(void)
{
   BYTE far *tmpaddr;
   ulong tmpphys;
   WORD addroffset;
   int i;
   BYTE *aipptr;

   /*
    * Get LDT selector pointing to MMIO
    */
   addroffset = 0x1F60;
   tmpphys = (ulong)InitMMIOAddr << 4;
   tmpaddr = (BYTE far *)physToUVirt(tmpphys, 8192L, 1);


   /*
    * Make sure checksums are OK
    */
   if (((BYTE)*(tmpaddr + addroffset)) & 0xF0)
   {
      printf("AIP Checksum 1 invalid!\n");
      return (!SUCCESS);
   }

   addroffset = 0x1FF0;
   if (((BYTE)*(tmpaddr + addroffset)) & 0xF0)
   {
      printf("AIP Checksum 2 invalid!\n");
      return (!SUCCESS);
   }

   /*
    * Fill our AIP structure with information from the adapter.
    */
   addroffset = 0x1FA0;
   aipptr = &aip.AdapterType;
   for (i = 0; i < 7; i++)
   {
      *aipptr = (BYTE)*(tmpaddr + addroffset);
      aipptr++;
      addroffset += 2;
   }

   /*
    * Spit out information for user.
    */
   printf("\n     Adapter type: %s\n", strType[aip.AdapterType]);
   printf("     Data rates supported: %s\n", strDatarates[aip.AdapterDataRate & 0x03]);
   printf("     Data rate support for Early Token Release: %s\n", strETR[aip.AdapterETR & 0x03]);
   printf("     Total available shared RAM: %s\n", strSRAM[aip.AdapterTotalRAM & 0x07]);
   printf("     Shared RAM page size: %s\n", strRAMPage[aip.AdapterRAMPaging & 0x03]);
   printf("     DHB size available at 4Mbps: %s\n", strDHB4[aip.Adapter4MbpsDHB & 0x03]);
   printf("     DHB size available at 16Mbps: %s\n", strDHB16[aip.Adapter16MbpsDHB & 0x07]);


   /*
    * Release the local descriptor.
    */
   physToUVirt((ulong)tmpaddr, 0L, 2);

   return(SUCCESS);
}



/*
 * Function: setSharedRAM( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * Sets up the shared ram to allow paging through all 64K of on-board RAM.
 */
WORD
setSharedRAM()
{
   BYTE far *tmpaddr;
   ulong tmpphys;
   WORD addroffset;
   BYTE regval;

   /*
    * Get LDT selector pointing to MMIO
    */
   tmpphys = (ulong)InitMMIOAddr << 4;
   tmpaddr = (BYTE far *)physToUVirt(tmpphys, 8192L, 1);


   regval = (BYTE)*(tmpaddr+RRRODD);

   /*
    * The value in the aip.AdapterTotalRAM should map to 16K shared RAM.
    * If this or RRR-odd does not specify 16K no point in going further!
   if ((regval & 0x0C) != 0x04)
   {
      printf("\nError:  Adapter must be configured for 16K shared RAM\n");
      return(!SUCCESS);
   }
    */

   /*
    * Release the local descriptor.
    */
   physToUVirt((ulong)tmpaddr, 0L, 2);
   return(SUCCESS);
}


/*
 * Function: hwPostConfig( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * perform poweron hardware config at "post-processParms" time.
 */
extern char gi_state;

WORD
hwPostConfig()
{
	ushort status;
	int	i;
	DWORD **statP = keptStats;
	openAdapType	op;

	MSC.MscTxQDepth	=	buffs[TX_BUFF_INDEX].num;

	/*
	 * turn on the time delta counter
	 */
	_enableTicker();

   if (!HW_Mchan)
      if (Get_MiscAT() != SUCCESS)
         return (!SUCCESS);
	/*
	 * Get the adapter's encoded address out of the BIOS/MMIO segment.
    * The address is at offset 0x1f00 - 0x1f16. Each even address contains
    * a nibble which represents 1 hex digit of the address. God only knows
    * why they encoded it this way!
	 */
	getBIA();
   /*
    * Look at the configuration of the AIP and report relevent info to user
    */
   if (checkConfig() != SUCCESS)
      return (!SUCCESS);

   /*
    * Configure the shared RAM for paging.
   if (setSharedRAM() != SUCCESS)
      return (!SUCCESS);
    */

	gi_state = 0;
   printf("\nInitializing adapter and running diagnostics...");
	if (status = gi_startup()) {
		if (status <= GIR_NOTFOUND) 
			printf("%s %s '%s'\n", errorPrefix, hwInitErr,giErrs[status]);
		else
			printf("%s %s 0x%x\n",errorPrefix, hwFatalErr, status);

		/*
		 * turn off the time delta counter
		 */
		_disableTicker();
		unSetIrq(giop.irqline);

		return(!SUCCESS);
	}


	/*
	 * All checks on hardware are finished, hardware status is "OK".
	 */
	MSS.MssStatus |= HW_OK;
   printf("\nAdapter initialized OK\n");

	/*
	 * One GDT is allocated for each transmit buffer, also an extra one
	 * is stashed in the ndisGen structure for miscellaneous use.
	 */
	allocGDTSelectors(NUMGDTS, gdtAllocArray);
	for (i=0; i < MAXPENDING; i++) {
		txgdts[i] = gdtAllocArray[i];
	}
	ndisGen.gdt = gdtAllocArray[i];

	/*
	 * Zero-out the "kept" statistics.
	 */
	while (*statP) {
		**statP = 0;
		statP++;
	}

	/*
	 * try to open the adapter with the maximum size expansion RAM
	_bzero((void far *)&op, sizeof(openAdapType));
	if (openAdapter((openAdapType far *)&op) != SUCCESS) {

		printf( genFailure );
		_disableTicker();
		unSetIrq(giop.irqline);
		return(!SUCCESS);
	}
	else {
		return(closeAdapter((genRequestType far *)&op));
	}
	 */

	return(SUCCESS);
}

/*
 * Function: Get_MMIOAddr()
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * If AT bus, get the MMIO address from the PIO registers.
 */
WORD ints_available[] = { 2, 3, 6, 7 };

WORD
Get_MiscAT()
{
   BYTE ir;
   WORD int_level;

   /*
    * Check our interrupt level for sanity.
    */
   ir = inp(PIOBase);
   int_level = ir & 0x03;
   if (ints_available[int_level] != giop.irqline)
   {
      printf("Interrupt not configured correctly\n");
      return (!SUCCESS);
   }

   /*
    * Set up our MMIO address for intialization.
    */
   InitMMIOAddr = (WORD)(ir >> 1);
   InitMMIOAddr |= 0x0080;
   InitMMIOAddr <<= 8;

   /*
    * Set up our global descriptors to access MMIO addresses and
    * shared RAM.
    */
	allocGDTSelectors(1, &GDTSelector);
   physToGDT(((ulong)InitMMIOAddr << 4), 8192, GDTSelector);
   MMIOAddress = (ulong)GDTSelector << 16;

	allocGDTSelectors(1, &GDTSelector);
   physToGDT(((ulong)InitRAMAddr << 4), 65535, GDTSelector);
   RAMAddress = (ulong)GDTSelector << 16;

   return (SUCCESS);
}



/*
 * the following series of functions are for initializing the parameter
 * init structure.
 */

/*
 * Function: setIObase( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
char	*DEF_IOBASE	= "PRIMARY";

WORD 
setIObase( void far *p )
{
   char far *q;

   if (!HW_Mchan)
   {
      q = (char far *)p;
      if (!strcmp(q, "PRIMARY"))
         giop.iobase = 0x0A20;
      else if (!strcmp(q, "ALTERNATE"))
         giop.iobase = 0x0A24;
      else
      {
   		printf(parmIoErr,giop.iobase);
   		return(!SUCCESS);
   	}

      PIOBase = giop.iobase;

   }

	return(SUCCESS);
}

/*
 * Function: setIrqLine( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * the acceptable interrupt request lines range from 2 to 12
 */
#define MIN_IRQ	2
#define MAX_IRQ	12
#define DEF_IRQ	3

extern void far IntHandler(void);

WORD 
setIrqLine( void far *p )
{

   if (!HW_Mchan)
   {
   	giop.irqline = *(ushort far *)p;
   }

 	if (giop.irqline < MIN_IRQ || giop.irqline > MAX_IRQ) {
  		printf(parmIrqErr,giop.irqline);
  		return(!SUCCESS);
  	}

  	/*
  	 * save the IRQ vector in the MSC table.
  	 */
  	MSC.MscInterrupt = giop.irqline;

  	/*
  	 * get an unshared interrupt.
  	 * must also remember to set the MSC table entry
  	 */
  	if (os2 && setIrq((void far *)IntHandler,
  			giop.irqline,IRQ_NOTSHARED) != SUCCESS) {
  		printf(parmIrqErr2,giop.irqline);
  		return(!SUCCESS);
  	}

   /*
    * On an AT machine this register enables interrupts on the adapter.
    * Microchannel machines are not affected.
    */
   IRQEnableReg = giop.irqline | 0x02F0;

  	/*
  	 * release the interrupt for adapter init time.
  	 */

/*	unSetIrq(giop.irqline); */
	return(SUCCESS);
}


/*
 * Function: setShare( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * the memory share segments are limited to an area of memory. the share seg
 * address must be a multiple of 0x200 as well.
 */
#define MIN_SH_SEG	0xa000	/* minimum value for shseg */
#define MAX_SH_SEG	0xfe00	/* maximum value for shseg */
#define DEF_SH_SEG	0xd800	/* default value for shseg */
#define SH_SEG_MSK	0x1ff		/* multiple divisor */

WORD 
setShare( void far *p )
{

   if (!HW_Mchan)
   {
   	giop.shseg = *(ushort far *)p;
   	giop.shoff	= 0;
      InitRAMAddr = *(WORD far *)p;

   	/*
   	 * check segment values
   	 */
   	if (giop.shseg < MIN_SH_SEG || giop.shseg > MAX_SH_SEG ||
   		(giop.shseg&SH_SEG_MSK)) {
   		printf(parmSegErr,giop.shseg);
   		return(!SUCCESS);
   	}
   }
    return(SUCCESS);

}

/*
 * Function: setDebug( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 *  Set the level of verbosity for run time debug info
 */
WORD
setDebug( WORD far *p )
{
#if !defined(debug)
	debug = *p;
#else
	p = NULL;
#endif
	return(SUCCESS);
}

/*
 * Function: setDataRate( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * Sets the data rate to either 4MBPs or 16MBPs
 *  Defaults to 4.
 */
#define DEF_DATARATE 4
WORD
setDataRate( void far *p )
{
   if (!HW_Mchan)
   {
      Data_Rate = *(WORD far *)(p);
      if (Data_Rate != 16 && Data_Rate != 4)
         return (!SUCCESS);
   }
   else
   {
      if (Data_Rate != *(WORD far *)p)
         return (!SUCCESS);
   }
   return (SUCCESS);
}
/*
* these are the defaults for the Proteon 1340 and 1347 boards
*/
parmInitType driverParms[] = {
    {
	    &debugKW,
	    NUMERIC_PARM,
	    0,
	    NULL,
	    setDebug
    },
    {
	    &AdapterTypeKW,
	    STRING_PARM,
	    NULL,
	    &DEF_IOBASE,
	    setIObase
    },
    {
	    &RAMAddrKW,
	    NUMERIC_PARM,
	    DEF_SH_SEG,
	    NULL,
	    setShare
    },
    {
	    &interruptKW,
	    NUMERIC_PARM,
	    DEF_IRQ,
	    NULL,
	    setIrqLine
    },         
    {
	    &DataRtKW,
	    NUMERIC_PARM,
	    DEF_DATARATE,
	    NULL,
	    setDataRate
    },

    { NULL }			/* End of list */
};


/*
 * POS register 2 contains the RAM Address in bits 1-7.  Bit 0 contains
 * the card enable bit and since the card was disabled before this routine
 * is called we know for sure bit 0 is 0.
 */
WORD
setPosReg2(uchar p)
{

   InitRAMAddr = (WORD)p;
   InitRAMAddr <<= 8;
	allocGDTSelectors(1, &GDTSelector);

   physToGDT(((ulong)InitRAMAddr << 4), 65535, GDTSelector);
   RAMAddress = (ulong)GDTSelector << 16;

   HW_Mchan = 1;

   return(SUCCESS);
}


/*
 * POS register 3 contains the encoded interrupt level LSB in bit 7.  The
 * MSB is in POS register 4 so we wait until we extract this bit to
 * determine what interrupt we are using.  Bit 0 determines whether we
 * use the primary or alternate adapter PIO addresses.  Bit 1 determines
 * what data rate to use (16/4 Mbps)
 */
WORD
setPosReg3(uchar p)
{

   IntType = p & 0x80;
   IntType >>= 7;

   if (p & 0x02)
      Data_Rate = 16;
   else
      Data_Rate = 4;

   if (p & 0x01)
      PIOBase = 0xa24;
   else
      PIOBase = 0xa20;

   return(SUCCESS);
}

WORD IntArray[] = { 2, 3, 10, 11 };

/*
 * POS register 4 contains the BIOS/MMIO address in bits 1-7.  Bit 0 contains
 * the encoded interrupt level MSB.  The encoded value is then translated
 * into the true interrupt level using the array above.
 */
WORD
setPosReg4(uchar p)
{

   InitMMIOAddr = (WORD)(p & 0xFE);
   InitMMIOAddr <<= 8;

	allocGDTSelectors(1, &GDTSelector);

   physToGDT(((ulong)InitMMIOAddr << 4), 8192, GDTSelector);

   MMIOAddress = (ulong)GDTSelector << 16;
   p &= 0x01;
   IntType |= (p << 1);
   giop.irqline = IntArray[IntType];

   return(SUCCESS);
}

posType posInfo[] = {
   {
      NULL,
      POSREG2,
      0,
      0,
      0,
      0xFF,
      0x00,
      &PosReg2,
      setPosReg2
   },
   {
      NULL,
      POSREG3,
      0,
      0,
      0,
      0xFF,
      0x00,
      &PosReg3,
      setPosReg3
   },
   {
      NULL,
      POSREG4,
      0,
      0,
      0,
      0xFF,
      0x00,
      &PosReg4,
      setPosReg4
   },
   { NULL }

};



