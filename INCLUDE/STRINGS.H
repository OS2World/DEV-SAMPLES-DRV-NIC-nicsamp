/*
 * File: $Header:   \ibm16\strings.h  2-2-93  jess fahland  $	
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *      This file contains all the defines for character strings used
 *      in the driver.
 */

#ifndef STRINGS_H
#define STRINGS_H

extern	char *AdapterDescr;

extern	char *protMgrName;
extern	char *driverNameKW;	
extern	char *drvrDevName;		/* E.g.: "\dev\xxxx$" 					*/

#define DEVHDR_NAME_OFFSET	5	/* Index to name w/o "\dev\" prefix		*/

extern	char *altDrvrDevName;	/* E.g.: "\dev\xxx2$" (alternate adapt.)*/

#define DRVR_NUM_OFFSET		9  	/* Index of "2" in "\dev\xxx2$			*/
#define MAX_DRIVERS			8	/* Only support up to "xxx8$"			*/

extern char *debugKW;
extern char *memBaseKW;
extern char *ioBaseKW;
extern char *interruptKW;
extern char *versionMsg;
extern char *ClientCopyright;
extern char *dmaChannelKW;
extern char *initAddrKW;
extern char *nodeAddrKW;
extern char *adapterAddress;
extern char *AdapterTypeKW;
extern char *RAMAddrKW;
extern char *DataRtKW;
/*
 * Error messages. (init time only).
 */

extern	char *genFailure;		
extern	char *errorPrefix;
extern	char *tooManyErr;
extern	char *drvNotFndErr;
extern	char *notOpenPmErr;
extern	char *regModRejectErr;
extern	char *badParmTypeErr;
extern	char *hwInitErr;
extern	char *hwFatalErr;
extern	char *giErrs[];
extern   char *memAllocErr;
extern   char *allocGdtErr;
extern   char *memGDTErr;
extern   char *adapterNotFound;

extern	char *parmDmaErr;
extern	char *parmIoErr;
extern	char *parmIrqErr;
extern	char *parmIrqErr2;
extern	char *parmSegErr;
extern	char *parmNodeAddrErr1;
extern	char *parmNodeAddrErr2;
extern	char *parmNodeAddrErr3;

#endif
