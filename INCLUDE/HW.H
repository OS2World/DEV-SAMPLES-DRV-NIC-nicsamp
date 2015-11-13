/*
 * File: $Header:   \ibm16\hw.h  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file provides a link into the hardware and driver dependent names
 * 	and structures.
 */
#ifndef HW_H
#define HW_H

#include "uprim.h"

#define microChannel 0xc5

/*
 * general request events are not queued
 */
#define gReqEventsQueued 0

/*
 * DMA for the p1840 is unused. If this is a PS/2, then assume a p1840.
 */
#define SC_MICROCHAN 2
extern BYTE SCFeatures;
#define POSIDLSB 0x01
#define POSIDMSB 0xE0


/*
 * this variable is set when a general request is completed
 */
extern WORD volatile	gReqCompFlag;

extern void ringStatus(WORD status);
extern void adapterCheck(WORD reason);

/*
 * perform power on hardware config
 */
extern WORD hwConfig(void);
extern WORD hwPostConfig(void);

/*
 * set the station address
 */
extern WORD hwSetStationAddress(char far *);


extern WORD adapterInUse(void);
extern void adapterEnable(void);
extern WORD setPosReg2(uchar);
extern WORD setPosReg3(uchar);
extern WORD setPosReg4(uchar);

/*
 * setup the open structure
 */
void setupOpenParms( WORD buffSize, WORD xmits, WORD expStart, WORD expEnd);

struct AIPInfo {
   BYTE AdapterType;
   BYTE AdapterDataRate;
   BYTE AdapterETR;
   BYTE AdapterTotalRAM;
   BYTE AdapterRAMPaging;
   BYTE Adapter4MbpsDHB;
   BYTE Adapter16MbpsDHB;
   BYTE Dummy;
};


#endif
