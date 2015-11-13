/*
 * File: $Header:   \ibm16\ndis.c  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file contains the static initializations of the MAC tables.
 */
#include "version.h"
#include "ndis.h"
#include "giexec.h"
#include "strings.h"

extern struct CommChar MCC;

/*
 * at bind time, the protocol gives a pointer to it's common characteristics
 * table. From that table, we can also cache a pointer to the lower dispatch
 * function table.
 */
struct CommChar protCC;
struct ProtLwrDisp pldDsptchTble;

/*
 * For Token ring, the multicast address is really the Group address.
 * There can only be one Group address.
 */
struct MCastBuf	MCBuff = {
	 1,				/* Maximum # of entries = 1 for token ring 			*/
	 0				/* Start with zero entries (group address not set)	*/
};

struct MACSpecChar	MSC = {
	sizeof(struct MACSpecChar),		/* Length of MSC					*/
	"802.5",						/* Type is 802.5 (token ring)		*/
	ADDR_SIZE,						/* Station address size in bytes.	*/
	{ 0 },							/* Permanent station address.		*/
	{ 0 },							/* Current station address.			*/
	0,								/* Current functional address		*/
	(struct MCastBuf far *)&MCBuff,	/* pointer to the Multicast table	*/
	16000000L,						/* Sixteen Megabits/Second				*/

	/*
	 * MAC Service flags bit mask; we support:
	 */

	(DWORD)
	(BROADCAST_SUPP|				/* Broadcast frames.				*/
	MULTICAST_SUPP|					/* Really the "group" Group address */
	FUNC_GROUP_ADDR_SUPP|			/* Functional Group address supp.	*/
	SET_STN_ADDR_SUPP|				/* Set station address.				*/
	STAT_ALWAYS_CURR |				/* Statistics always current		*/
	LOOPBACK_SUPP|					/* Self-addr frames are rec'd		*/
	IBM_SOURCE_R_SUPP |				/* SR bit is not adjusted			*/
	/*RESET_MAC_SUPP|*/	   			/* RESET Mac General request.		*/
	OPEN_ADAPTER_SUPP|				/* OPEN/CLOSE adapter.				*/
	GDT_ADDRESS_SUPP),				/* Support GDT or physical address  */
				    				/* End of service flags.			*/

	MAX_RCV_PKT_SIZE,				/* Max frame sent or recvd.			*/
	0,								/* Total Xmit buffer capacity.		*/
	0,								/* Xmit buffer alloc block size		*/
	0,								/* Total Rcv buffer capacity		*/
	0,								/* Rcv buffer alloc block size		*/
	{0x40, 0x00, 0xC9},				/* IEEE vendor code (3 bytes)		*/
	0,								/* Vendor Adapter code (1 byte)		*/
	0,  							/* Adapter Descr. (explicit init)	*/
	0,								/* Interrupt Level					*/
	0								/* Tx Queue Depth					*/

};

extern struct MAC8025Stat MSS802;

struct MACSpecStat	MSS = {
	sizeof(struct MACSpecStat),
	-1,
	0,
	0,
	(void far *)&MSS802,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1
};

struct MAC8025Stat MSS802 = {
	sizeof(struct MAC8025Stat),
	0,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	-1,
	0,
	0,
	0,
	0
};


extern WORD (far pascal __generalRequests)(WORD,WORD,WORD,DWORD,WORD,WORD);
extern WORD (far pascal __transmitChain)(WORD,WORD,LPBUF,WORD);
extern WORD (far pascal __transferData)(LPBUF,WORD,LPBUF,WORD);
extern WORD (far pascal __receiveRelease)(WORD,WORD);
extern WORD (far pascal indicationsOn)(WORD);
extern WORD (far pascal indicationsOff)(WORD);

struct MACUprDisp		MUD	= {
	(struct CommChar far *)&MCC,
	__generalRequests,
	__transmitChain,
	__transferData,
	__receiveRelease,
	indicationsOn,
	indicationsOff
};

extern WORD	(far pascal systemRequest)(DWORD,DWORD,WORD,WORD,WORD);

struct CommChar MCC = {
	sizeof(struct CommChar),
	0,
	0,
	MAJOR_VERSION,
	MINOR_VERSION,
	0,
	"hello world",
	MACLvl,
	MACLvl,
	0,
	MACLvl,
	0,
	0,
	systemRequest,
	(struct MACSpecChar far *)&MSC,
	(struct MACSpecStat far *)&MSS,
	(struct MACUprDisp far *)&MUD
};

/*
 * define a place for some general NDIS variables
 */
struct ndisGeneral ndisGen = {

	(BYTE far *)NULL,	/* no value for currBuff at startup */
	0,					/* 0 currBuffSize */

	/*
	 * the minimum lookahead defaults to 64
	 */
	MIN_LOOKAHEAD_DEFAULT,

	/*
	 * clear indication nest level initially
	 */
	0
};

