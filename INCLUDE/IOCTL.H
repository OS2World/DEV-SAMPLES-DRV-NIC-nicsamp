/*
 * $Header:   \ibm16\ioctl.h  2-2-93 jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description: 
 *	    Definitions and structures for Operating system requests.
 *		 This is a subset of functions, only the INIT cmd and genioctl
 *		 are interesting for MAC drivers.
 */

#include "ndis.h"

#ifndef IOCTL_H
#define IOCTL_H

#define	STERR	0x8000		/* Bit 15 - Error					*/
#define	STDON	0x0100		/* Bit  8 - Done					*/
#define GENERR  31			/* general error					*/
#define INVFUN	1			/* INVALID FUNCTION					*/
#define	STECODE	0x00FF		/* Error code:						*/
#define	UNK_CMD	0x0003		/* Unknown command.					*/


struct endSegs {

	WORD	CodeS;			/* End of Code segment				*/
	WORD	DataS;			/* End of Data segment				*/
};

struct init_rq_pkt {

	BYTE 	PktLen;			/* length in bytes of packet		*/
	BYTE 	PktUnit;		/* subunit number of block device	*/
	BYTE	PktCmd;			/* command code						*/
	WORD	PktStatus;		/* status word						*/
	DWORD	PktDOSLink;		/* reserved							*/
	DWORD	PktDevLink;		/* device multiple-request link		*/
	BYTE	InitcUnit;		/* number of units returned			*/

	/* 
	 * Next 4 bytes are the DevHlp far address (on entry) and
	 * on exit must be set to the ending offsets of the code
	 * and data segments.
	 * XXXX fixup for DOS: far pointer to end-of-module
	 */

	union {

		void	(far *InitpDevHlp)();

		struct endSegs	End;

	} InitUnion;

	DWORD	InitpBPB;	  	/* unused (character device)	*/
	BYTE	InitDrv;		/* unused						*/
};

/*
 * OS2 Catagories for genioctl, only a single user defined
 * catagories is supported with a value of 82h
 */
#ifdef OS2
#define	IoctlCat	0x82	/* user defined bit (80h) and a code of 2 */

/*
 * Function Codes
 */

#define IgnoreMask	0x80	/* if set ignore upsupported codes */
#define CodeMask	0x1f	/* function code mask */
#define SendMacStat 0xe0	/* send mac statistics */

/*
 * HW specific function codes should be defined in hw.h
 */


typedef struct GenIoctldef {
	BYTE 	PktLen;			/* length in bytes of packet		*/
	BYTE 	PktUnit;		/* subunit number of block device	*/
	BYTE	PktCmd;			/* command code						*/
	WORD	PktStatus;		/* status word						*/
	DWORD	PktDOSLink;		/* reserved							*/
	DWORD	PktDevLink;		/* device multiple-request link		*/
/*
 * The current implementation does not support ioctl2, the following
 * is the format of the general request block for ioctl.
 */
	BYTE	FunCat;			/* function catagory */
	BYTE	FunCode;		/* Function Code */
	LPBUF	ParBufAddr;		/* paramater buffer address */
	LPBUF	DataBufAddr;	/* data buffer address */
	WORD	SysNum;			/* system file number */
} GenIoctlType;
#else
#define	IoctlCat	0x44	/* gen ioctl */

/*
 * Function Codes
 */

#define IgnoreMask	0x80	/* if set ignore upsupported codes */
#define CodeMask	0x1f	/* function code mask */
#define SendMacStat 0xe0	/* send mac statistics */

/*
 * HW specific function codes should be defined in hw.h
 */


typedef struct GenIoctldef {
	BYTE 	PktLen;			/* length in bytes of packet		*/
	BYTE 	PktUnit;		/* subunit number of block device	*/
	BYTE	PktCmd;			/* command code						*/
	WORD	PktStatus;		/* status word						*/
	DWORD	PktDOSLink;		/* reserved							*/
	DWORD	PktDevLink;		/* device multiple-request link		*/
/*
 * The current implementation does not support ioctl2, the following
 * is the format of the general request block for ioctl.
 */
	BYTE	FunCat;			/* function catagory */
	BYTE	FunCode;		/* Function Code */
	WORD	GIOSI;			/* contence of SI */
	WORD	GIODI;			/* contence of DI */
	LPBUF	DataBufAddr;	/* data buffer address */
} GenIoctlType;

#endif

extern WORD GenIoctl(GenIoctlType far *);

/*
 * Device helper functions
 */
extern unsigned far *LockMem(LPBUF);
extern WORD	UnlockMem (LPBUF);
#endif							
