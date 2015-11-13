/*
 * $Header:   \ibm16\mca.h  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This header file describes the POS registers for the micro-channel
 * 	architecture.
 */
#ifndef MCA_H
#define MCA_H
#include "hw.h"

/*
 ****************************************************************************
 *
 * Micro-channel specific MACROS
 *
 ****************************************************************************
 */

/*
 * the micro-channel position select register maps the used adapter slots
 */
#define CHANPOS				0x0096
#	define CHANPOS_USED		0x08	/* is the channel in use	*/
#	define CHANPOS_RESET	0x80	/* reset bit				*/
#	define CHANPOS_CHANMSK	0x07	/* channel position mask	*/

/*
 * there are 8 slots in a micro-channel PS/2
 */
#define NUMCHAN	8

/*
 * the POS ID registers are at 100 and 101
 */
#define POSIDREG_LSB	0x0100
#define POSIDREG_MSB	0x0101

/*
 * define the option select data registers 1-4
 */
#define POSREG2		POSIDREG_LSB+2
#define POSREG3		POSIDREG_LSB+3
#define POSREG4		POSIDREG_LSB+4
#define POSREG5		POSIDREG_LSB+5

/*
 * micro-channel subaddress extension registers
 */
#define SUBADDREXTLSB	POSIDREG_LSB+6
#define POSREG6			SUBADDREXTLSB
#define SUBADDREXTMSB	POSIDREG_LSB+7
#define POSREG7			SUBADDREXTMSB

/*
 * the card enable bit lives in POS register 102 at bit 0
 */
#define POSENABLEREG		POSREG2
#define CARD_ENABLE			0x01

/*
 * the channel check active bit determines if there is a channel check needed.
 */
#define CHANNEL_CHECK_POSREG	POSREG5
#define CHANNEL_CHECK_ACTIVE	0x80

/*
 * this structure defines how the POS registers are manipulated.
 */
typedef struct {
	char	*debugStr;

	/*
	 * if posReg is SUBADDREXTLSB, then subAddrIndexXsb is used for the
	 * sub-address extension setup. otherwise, only posRegLsb is used. if 
	 * sub-address extension is used, then dataReg specifies the I/O address
	 * of the data register used in conjunction with the sub-address 
	 * registers.
	 */
	ushort	posReg;
	uchar	subAddrIndexLsb;
	uchar	subAddrIndexMsb;
	ushort	dataReg;

	/*
	 * the value read from the POS register is always anded with 'mask' and
	 * shifted right by 'shift' bits before OR'ing the result into '*var'.
	 */
	uchar	mask;
	uchar	shift;

	/*
	 * the structure may supply either a ponter to a variable, a pointer to
	 * a set function, or both. setFunc returns SUCCESS or !SUCCESS. setFunc
	 * is always passed the value of the variable extracted from the POS
	 * register.
	 */
	uchar	*var;
	ushort	(*setFunc)(uchar);

} posType;

extern posType posInfo[];
extern WORD mChanInit(void);

#endif
