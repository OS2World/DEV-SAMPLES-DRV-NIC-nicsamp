/*
 * File: $Header:   \ibm16\strings.c  2-2-93 jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *           This file contains all of the "user-visible"
 * 			 strings used in the driver.
 *				 The idea is to isolate the ASCII/english stuff, in
 *				 case we need to add foreign language support.
 */
#include <stdio.h>
#include "ndis.h"


/*
 * Adapter_Description - String referenced in Common Characteristics table
 * This string is used to give information about the device driver in a
 * human-readable form.
 */
char *AdapterDescr		=	"IBM Token-Ring 16/4 NDIS Driver";

char *protMgrName		=	"\\DEV\\PROTMAN$";
char *driverNameKW		=	"DRIVERNAME";
char *drvrDevName		=	"\\DEV\\IBM16$";
char *altDrvrDevName		=	"\\DEV\\IBM162$";
char *debugKW			=	"DEBUG";
char *interruptKW		=	"INTERRUPT";
char *versionMsg		=	"Version: ";
/* These were added to supplement the driver parameters
    for Proteon 1340 and 1347 boards. */  
char *AdapterTypeKW		=	"ADAPTERTYPE";
char *RAMAddrKW		=	"RAMADDR";
char *DataRtKW		=	"DATARATE";
/*
 * Error messages.
 */
char *genFailure		=	"Driver initialization failed: Check hardware configuration\n";
char *errorPrefix		=	"Error: ";
char *tooManyErr		=   "Limit on # of installed drivers exceeded.";
char *drvNotFndErr		=   "Drivername not found in PROTOCOL.INI";
char *notOpenPmErr		=	"Could not open PROTMAN$";
char *regModRejectErr	=	"Register module rejected";
char *badParmTypeErr	=	"PROTOCOL.INI parameter is wrong type for: ";
char *hwInitErr			= 	"Hardware initialization failure code ";
char *hwFatalErr		= 	"Hardware initialization failure, unrecognized status code ";

char *parmDmaErr		=	"Illegal DMA channel %d\n";
char *parmIoErr			=	"Illegal IO Base 0x%x\n";
char *parmIrqErr		=	"Illegal IRQ line %d\n";
char *parmIrqErr2		=	"IRQ line %d already in use\n";
char *parmSegErr		=	"Illegal Share Segment 0x%x\n";
char *parmNodeAddrErr1		=	"Badly formatted node address '";
char *parmNodeAddrErr2		=	"'\n";
char *parmNodeAddrErr3		=	"Out of Range node address '";

char *giErrs[] = {
	"Operation completed successfully",
	"Bringup failed",
	"Init failed",
	"Open failed",
	"Adapter not in correct state for operation",
	"Bad function code, not supported",
	"Adapter de-inserted while operation in progress",
	"Transmit was cancelled by caller",
	"Error in transmit",
	"Destination address not recognized",
	"Transmitted frame was not copied",
	"Cancel request didn't match anything"
};

