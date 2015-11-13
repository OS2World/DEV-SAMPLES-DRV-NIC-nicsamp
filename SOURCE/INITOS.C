/*
 * $Header:   \ibm16\initos.c  2-2-93 jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *
 *		This module supplies operating system specific initialization. The
 *		only external entry point is OSinit(). OSinit() handles pointer
 *		fixup for OS/2 and invokes ndisInit().
 *
 */
#include <stdio.h>
#include "devhdr.h"
#include "ndis.h"
#include "ioctl.h"
#include "strings.h"
#include "misc.h"
#include "version.h"
#include "print.h"

/* declared as extern in strings.h, used in routine void OSinit( ) */
char *ClientCopyright		=	"Sample OS/2 Token-Ring Driver, for the IBM NDIS Driver Developer's Toolkit      ";

/*
 * extern the start of the code and data segments
 */
extern WORD startInitData;
extern void far *_startInitData(void);
extern void startInitCode(void);

/*
 * extern the Device Helper entry point for DOS
 */
#ifdef DOS	
extern WORD far _DevHlp(void);
#endif 

/*
 * NDIS init function, put it here so that the ndis.h --> ndis.inc generation
 * doesn't get messed up. it's a kludge...
 */
extern WORD ndisInit(void);

/*
 * each project must provide an environmental init function.
 */
extern WORD environmentInit(void);

/*
 * codeFix and dataFix are arrays of near pointers to locations in
 * Code and Data space that need init-time fixup. Under OS/2 1.0 (and up)
 * the far pointers at bind time are Ring 3 selectors. Before making
 * them visible, we need to fix them up with our Ring 0 selectors.
 * The Ring 0 selectors are suppplied to us in the Device Header.
 * This "feature" may disappear in later versions of OS/2.
 */ 
WORD *codeFix[] = {
#ifdef OS2
	(WORD *)&MCC.CcSysReq+1,
	(WORD *)&MUD.MudGReq+1,
	(WORD *)&MUD.MudXmitChain+1,
	(WORD *)&MUD.MudXferData+1,
	(WORD *)&MUD.MudRcvRelease+1,
	(WORD *)&MUD.MudIndOn+1,
	(WORD *)&MUD.MudIndOff+1,
#endif
	(WORD *)NULL
};

#define CODEFIXLEN sizeof(codeFix) / sizeof(WORD *)

WORD *dataFixPreInit[] = {
	&MCC.CcDataSeg,
#ifdef OS2
	(WORD *)&MCC.CcSCp+1,
	(WORD *)&MCC.CcSSp+1,
	(WORD *)&MCC.CcUDp+1,
	(WORD *)&MCC.CcLDp+1,
	(WORD *)&MSC.MscMCp+1,
	(WORD *)&MSC.MscVenAdaptDesc+1,
	(WORD *)&MSS.MssM8Sp+1,
	(WORD *)&MUD.MudCCp+1,
#endif
	(WORD *)NULL
};
WORD *dataFixPostInit[] = {
#ifdef OS2
	(WORD *)&DevHdr+1,
#endif
	(WORD *)NULL
};

#define DATAFIXLEN sizeof(dataFix) / sizeof(WORD *)


/*
 * Function: setFailure
 *
 * Inputs:
 *		reqPkt - far pointer to the initialization request packet.
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 *		This function sets up the return structure for the Operating
 *		System such that the device driver is de-installed.
 */
void
setFailure(struct init_rq_pkt far *reqPkt)
{
	if (os2) {
		reqPkt->InitUnion.End.DataS = 
		reqPkt->InitUnion.End.CodeS = 0;
	}
	else
		*(DWORD far *)&reqPkt->InitUnion.End = (DWORD)DevHdr;
	reqPkt->PktStatus = STERR;
}

/*
 * Function: OSinit
 *
 * Inputs:
 *		reqPkt - far pointer to an initialization request packet that was
 *				 originally given to the strategy entry point.
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 * 		This function does OS initialization generic to all NDIS drivers:
 *
 *			Saves DevHlp Address,
 *	 		Prints Load-Time Message,
 *			Does Code and Data Segment fixups,
 *			Calls NDIS init routine,
 *			Inserts return values in request packet.
 *
 */	
void OSinit(struct init_rq_pkt far *reqPkt)
{
	WORD **p;

	/*
	 * we are definitely at ring 3. This applies mostly to debug output. At
	 * init time, all character output goes to the screen via DOSWRITE,
	 * otherwise output goes direct to the UART.
	 */
	ndisGen.ring0 = 0;

	/*
	 * initialize the processor type to a default of 16 bit string moves.
	 * When the module is bound, then the real proc type will be determined
	 * at ring 0.
	 */
	ndisGen.procType = PROC_8086;

	/*
	 * Save DevHlp address
	 */
#ifdef OS2
 	DevHlp	= reqPkt->InitUnion.InitpDevHlp;
#else
	/*
	 * DevHlp's are simulated for DOS.
	 */
 	DevHlp	= _DevHlp;
#endif

	/*
	 * save the DEVICE= string for later processing.
	 */
	fStrncpy( (char far *)reqPkt->InitpBPB,
			  (char far *)ndisGen.devInitStr,
			  DEVINIT_LEN);

	/*
	 * print the sign-on message
	 */
	printf("%s %s %s.%s\n\
%s\
Copyright 1993 DWB Associates, Inc.\n\
All Rights Reserved\n",
		AdapterDescr,
		versionMsg,
		MAJOR_VERSION_STR,
		MINOR_VERSION_STR,
		ClientCopyright);

	/*
	 * get the 4 Gb GDT descriptor initialized.
	 */
	if (allocGDTSelectors(1,&ndisGen.gdt0) != SUCCESS) {
		printf("OSinit - cannot alloc GDT0\n");
		setFailure(reqPkt);
		goto exit;
	}

	/*
	 * invoke the environment init function. this is where device init 
	 * strings can be parsed.
	 */
	if (environmentInit() != SUCCESS) {
		setFailure(reqPkt);
		goto exit;
	}

	/*
	 * convert ring 3 code pointers to ring 0.
	 */
	if (os2) {
		for (p=codeFix; *p; p++) {
			**p = DevHdr->SDevProtCS;
		}
	}

	/*
	 * some of the DOS variables may need to get a segment, most noteably
	 * MCC.ModuleDS.
	 */
	for (p=dataFixPreInit; *p; p++) {
		**p = DevHdr->SDevProtDS;
	}

	/*
	 * Initialization failure is a "normal" return from the init command,
	 * we signal failure by "deinstalling" the device driver.
	 * The "ndisInit" function does all NDIS and HW specific initialization.
	 */
	if (ndisInit() == SUCCESS)	{
		if (os2) {
			reqPkt->InitUnion.End.DataS = (WORD)&startInitData;
			reqPkt->InitUnion.End.CodeS = (WORD)startInitCode;
		}
		else {
			*(DWORD far *)&reqPkt->InitUnion.End =
				(DWORD)physToUVirt( virtToPhys(_startInitData()) + 
							(DWORD)DataEndOffset, 0);
		}
		reqPkt->PktStatus = 0;			/* Return Done, OK				*/
	}

	/*
	 * failed init
	 */
	else {
		setFailure(reqPkt);
	}

	/*
	 * post init data pointer fixups
	 */
	if (os2) {
		for (p=dataFixPostInit; *p; p++) {
			**p = DevHdr->SDevProtDS;
		}
	}

exit:

	/*
	 * set the done bit
	 */
	reqPkt->PktStatus |= STDON;			/* Return Done, OK				*/
	reqPkt->InitcUnit = 0;				/* Character device return zero	*/
	reqPkt->InitpBPB  = 0;				/*     "       "      "      "	*/

	/*
	 * upon exit from this thread, we get to be a ring 0 device driver. DOS
	 * will still scribble to the screen.
	 */
	if (os2)
		ndisGen.ring0++;
}
