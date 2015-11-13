/*
 * file: $Header:   \ibm16\devhdr.h  2-2-93 jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file is the device table defines.  It should mirror
 *    the file devhdr.inc
 */
#ifndef DEVHDR_H
#define DEVHDR_H
#include "ndis.h"

#define DEVNAME_LEN	8

struct SysDev
{
	DWORD	SDevNext; 		/* Pointer to next device header		*/
	WORD	SDevAtt; 		/* Device Attributes					*/
	WORD	SDevStrat;		/* Strategy entry point					*/
	WORD	SDevInt;		/* Interrupt entry point (DOS only)		*/
	char	SDevName[DEVNAME_LEN];	/* Device Name					*/
	WORD	SDevProtCS; 	/* Ring 0 CS Selector					*/
	WORD	SDevProtDS;		/* Ring 0 DS Selector					*/
	WORD	SDevRealCS;		/* Real-mode CS segment					*/
	WORD	SDevRealDS;		/* Real-mode DS segment					*/
};

/*
 * system device header
 */
extern struct SysDev far *DevHdr;

/*
 * these variables get the ending code and data offset after init is complete.
 */
extern WORD CodeEndOffset;
extern WORD DataEndOffset;

/*
 * device helper entry point
 */
extern WORD (far *DevHlp)();

/*	Device Driver Attribute definitions								*/

#define DEV_30			0x0800		/* Supports Open/Close			*/
#define	DEV_CHAR_DEV	0x8000		/* Character Device				*/
#define	DEVLEV_1		0x0100		/* OS/2 						*/

#endif
