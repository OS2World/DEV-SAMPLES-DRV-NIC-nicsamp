/*
 * File: $Header:   \ibm16\hw.c  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file provides most of the runtime hardware and driver dependent
 * 	functions.
 */
#include <stdio.h>
#include <conio.h>
#include "misc.h"
#include "ndis.h"
#include "uprim.h"
#include "hwbuff.h"
#include "strings.h"
#include "ioctl.h"
#include "print.h"

/*extern void Logit(); */
extern DWORD _initTime(void);
extern DWORD _getTime(void);
extern void far IntHandler(void);

WORD
adapterInUse(void);

void
adapterEnable(void);

extern WORD Data_Rate;
/*
 * An array of pointers to statistics kept by the MAC driver.
 * This array is used at init time and by the ClearStatistics
 * general request and must be NULL terminated.
 */
DWORD *keptStats[] = {
	&MSS.MssFR,				/* # of frames received.					*/
	&MSS.MssFRByt,			/* # of bytes received.						*/
	&MSS.MssFS,				/* # of frames sent.						*/
	&MSS.MssFSByt,			/* # of bytes sent.							*/
	&MSS.MssFRMC,			/* # of multicast frames received			*/
	&MSS.MssFRBC,			/* # of broadcast frames received			*/
	&MSS.MssFRMCByt,		/*  # of multicast bytes received			*/
	&MSS.MssFRBCByt,		/*  # of broadcast bytes received			*/
/*	&MSS.MssFSBCByt,		 # of broadcat bytes sent					*/
/*	&MSS.MssFSMCByt,		 # of multicast bytes sent				*/
	NULL
};

/*
 * this variable is set when a general request is completed
 */

WORD  volatile	gReqCompFlag=0;

/*
 * The GI status and subcode are posted here.
 */
WORD	GIstatus=0;
WORD	GIsubcode=0;

/*
 * This flag is set if we are running in trace mode, otherwise it is 0.
 */
WORD TAPFlag = 0;

/*
 * This Flag is set if a rcv indication is being held off by the
 * INDICATION OFF.
 */
WORD	receivePending = 0;

/*
 * Flag, opcode and parameter used when a status indication is held off by
 * INDICATION OFF.
 */
WORD	statusIndPending = 0;
WORD	siPendingOpcode=0;
WORD	siPendingParam=0;

extern BYTE far *RAMAddress;         /* Holds shared RAM GDT */

/*
 * allocate space for the TX buffer pool.
 */
static txBuffType txBuffs[MAX_TX_BUFFS];

/*
 * allocate space for the general request queue
 */
static genReqType gReqBuffs[MAX_GREQ_BUFFS];

/*
 * define the buffer structure objects
 */
buffType buffs[] = {
	{
		(bufStructType *)NULL,
		(bufStructType *)NULL,
		(bufStructType *)NULL,
		sizeof(txBuffType),
		MAX_TX_BUFFS,
		(bufStructType *)txBuffs
	},
	{
		(bufStructType *)NULL,
		(bufStructType *)NULL,
		(bufStructType *)NULL,
		sizeof(genReqType),
		MAX_GREQ_BUFFS,
		(bufStructType *)gReqBuffs
	},

	/*
	 * the array is terminated when buff==NULL
	 */
	{ 0 }
};
/*
 *****************************************************************************
 *****************************************************************************
 *
 * this is the HW specific transmit portion of HW.C
 *
 *****************************************************************************
 *****************************************************************************
 */

WORD SRB_Offset = 0xFFFF;
WORD ARB_Offset;
WORD SSB_Offset;
WORD ASB_Offset;
/*
 * Function: GenIoctl( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 *  Stub function:  Always return SUCCESS.
 */
WORD 
GenIoctl(GenIoctlType far *p)
{
	return(SUCCESS);
}

/*
 * Function: hwXmit( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * this function is used to transmit a buffer
 */
void
hwXmit(void)
{

	txBuffType *txp = headTxQueue();

	/*
	 * prepare the GCB structure for transmission. note that the far pointer
	 * to transmitChainComplete() must have the correct CS value, i.e., 
	 * privlege level 0 selector.
	 */

	if (debug & DBG_TERSE)
		printf("X");

	/*
	 * fire away
	 */
	gi_transmit(txp);

}

/*
 * Function: gReqComplete( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * this is the function that the GI will call when a general request is 
 * complete.
 */
void
gReqComplete(WORD status)
{
	genReqType *p;

	if (!headGreqQueue()) {
		GIstatus = status;
		gReqCompFlag++;
		return;
	}

	/*
	 * get the general request structure from the Q
	 */
	p = dequeueGreqBuff();

	/*
	 * call request confirm
	 */
	if (p->req.reqHandle) {
		(*pldDsptchTble.PldReqConfirm)(	p->req.protID,
										MCC.CcModuleID,
										p->req.reqHandle,
										status,
										p->req.opCode,
										protCC.CcDataSeg);
	}

	/*
	 * free up the request resource
	 */
	freeGreqBuff(p);
}

/*
 * Function: waitForGreqComplete( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * wait for the last general request to complete. this assumes that someone
 * has cleared gReqCompFlag prior to calling this function.
 */

extern ushort ticks(void);

#define TIMEOUT	60000		/* 60 seconds (one second = 18.2 ticks) */
WORD
waitForGreqComplete(void) {

	DWORD stopTime;

	stopTime = _initTime() + (DWORD)TIMEOUT;

	/*
	 * loop until timeout or the general request completes
	 */
	while (!gReqCompFlag && _getTime() < stopTime)
		;

	/*
	 * if done, then translate the GI status into NDIS status and return. 
	 * this assumes that the caller to GI supplied the address of GIstatus
	 * as the place to put status.
	 */
	if (gReqCompFlag) {

		/*
		 * clear the completion flag on the way out
		 */
		gReqCompFlag = 0;
		return(GIstatus);
	}
	else
		return(GENERAL_FAILURE);
}

/*
 * define a structure for use when opening the adapter.
 * Guide for more details.
 */
typedef struct {

	/*
	 * Number of receive buffers requested.
	 */
   WORD NumRcvBuffs;

	/*
	 * Lengthof each receive buffer.
	 */
	WORD RcvBuffLength;

	/*
	 * Length of transmit buffers.
	 */
	WORD DHBLength;

	/*
	 * Number of transmit buffers.
	 */
	BYTE NumDHB;

} openBuffType;

openBuffType openAdap4 = { 26, 2048, 4464, 1 };
openBuffType openAdap16 = { 22, 2048, 17960, 1 };
openBuffType openAdapTAP = { 20, 2048, 136, 1 };

extern char gi_state;

/*
 * Function: openAdapter( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * open the hardware adapter
 */
WORD 
openAdapter(openAdapType far *p )
{
	WORD openStat;
	WORD i;

	/*
	 * If we're already open, return immediately.
	 */
	if (MSS.MssStatus & MS_OPEN) {
		return(SUCCESS);
	}


	/*
	 * clear the indication complete flag
	 */
	ndisGen.needIndCmplt = 0;

   if (p->openOptions)
   {
      TAPFlag = 1;
      setupOpenParms(openAdapTAP.RcvBuffLength, openAdapTAP.NumRcvBuffs,
                               openAdapTAP.DHBLength, openAdapTAP.NumDHB);
   }
   else if (Data_Rate == 16)
   {
      TAPFlag = 0;
      setupOpenParms(openAdap16.RcvBuffLength, openAdap16.NumRcvBuffs,
                               openAdap16.DHBLength, openAdap16.NumDHB);
   }
   else
   {
      TAPFlag = 0;
      setupOpenParms(openAdap4.RcvBuffLength, openAdap4.NumRcvBuffs,
                               openAdap4.DHBLength, openAdap4.NumDHB);
   }


   gi_state = GIS_NOTHING;
	/*
	 * Call the hardware specific startup. Nonzero return indicates
	 * general failure.
	 */
	if (gi_open(p->openOptions) != SUCCESS) {

	   /*
   	 * reset the hardware and try again
   	 */
      gi_state = 0;
      return(GENERAL_FAILURE);
	}

	/*
 	 * wait for synchronous completion
 	 */
	if ((openStat = waitForGreqComplete()) != SUCCESS)
      return(GENERAL_FAILURE);

  	MSS.MssStatus |= MS_OPEN;

   /*
    * If Trace mode is wanted we must do a transmit
    */
   if (TAPFlag)
   {
      (BYTE)*(RAMAddress + SRB_Offset) = SCB_TRANSMITDIR;
      (BYTE)*(RAMAddress + SRB_Offset + 1) = 0;
      (BYTE)*(RAMAddress + SRB_Offset + 4) = 0;
      (BYTE)*(RAMAddress + SRB_Offset + 5) = 0;
      srb_request();

   	if ((openStat = waitForGreqComplete()) != SUCCESS)
         return(GENERAL_FAILURE);
   }
	return(openStat);
}

/*
 * Function: closeAdapter( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * close the adapter hardware
 */
WORD
closeAdapter( register genRequestType far *p )
{
	WORD	status;

	/*
	 * We can't close if we're not open.
	 */
	if (!(MSS.MssStatus & MS_OPEN)) {
		return(INVALID_FUNCTION);
	}

   goto escape;
	/* this next block was added to allow the pending transmit to complete
	 * before resetting the hardware and clearing the linked lists
	 *
	 * ALGORITHM:
	 *
	 * DISABLE INTERRUPTS
	 * if any xmits active
	 *   remove all xmits from busy list except first (active)
	 *   ENABLE INTERRUPTS
	 *   wait for busy list to clear (xmit done)
	 *   restore remaining xmits to busy list
	 * ENABLE INTERRUPTS
	 */
	{
		int saveflags;
		struct bufStruct *pNextXmits;

		DISABLEI(saveflags);

		if (buffs[TX_BUFF_INDEX].busy)	/* if frame is being transmitted */
		{
			/* save chain after current head of list as that
			 * fram is currently transmitting
			 */
			pNextXmits = buffs[TX_BUFF_INDEX].busy->next;
			buffs[TX_BUFF_INDEX].busy->next = 0;	/* terminate list */

			RESTOREI(saveflags);

			/* wait for frame to clear */
			while (buffs[TX_BUFF_INDEX].busy)
				;

			/* place remaining list back onto busy list */
			buffs[TX_BUFF_INDEX].busy = pNextXmits;
		}

		RESTOREI(saveflags);
	}
escape:
	/*
	 * call the hardware specific close
	 */
	gi_close();

	MSS.MssStatus &= (~MS_OPEN);
	MSC.MscCurrFncAdr = 0;

 	/*
	 * wait for synchronous completion
	 */
	status = waitForGreqComplete();

	/*
	 * scrub the buffers
	 */
	buffInit();

	return(status);
}

/*
 * Function: setStationAddress( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * set the station address
 */
WORD
setStationAddress(statAddrType far *p)
{
	/*
	 * if the station is already open, then fail
	 */
	if (MSS.MssStatus & MS_OPEN)
		return(INVALID_FUNCTION);

	/*
	 * copy the station address into our MAC specific characteristics table
	 */
	_bcopy(	(void far *)p->statAddr,
			(void far *)FNDATA(MSC.MscCurrStnAdr),
			sizeof(NODE));

	if (debug & DBG_VERBOSE) {
		printf("setStationAddress:\n");
		printData((void far *)MSC.MscCurrStnAdr,sizeof(NODE));
	}

	return(SUCCESS);
}

/*
 * Function: generalRequestConfirm( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * this is the upcall function that should be given when making adapter
 * general requests. (Not used, all requests are synchronous)
 */
void
generalRequestConfirm(void)
{
}

/*
 * Function: receiveRelease( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * receive release is not supported
 */
WORD
receiveRelease( rcvReleaseType far *p )
{
	return(NOT_SUPPORTED);
}

/*
 * Function: initiateDiagnostics( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * initiate diagnostics is not supported
 */
WORD
initiateDiagnostics( genRequestType far *p )
{
	return(NOT_SUPPORTED);
}

/*
 * Function: readErrorLog( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * Read adapter Error Log
 */

WORD
readErrorLog( errLogType far *p )
{
	WORD readStat;

	gi_counters((BYTE far *)p->logAddr);

	/*
	 * wait for synchronous completion
	 */
	readStat = waitForGreqComplete();
	return(readStat);
}

/*
 * Function: resetMAC( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * reset MAC is not supported
 */
WORD
resetMAC( genRequestType far *p )
{
	return(NOT_SUPPORTED);
}

/*
 * Function: setPacketFilter( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * For now, all we can support is directed and broadcast packets,
 * reject attempts to set source routing bits.
 */
WORD
setPacketFilter( setFilterType far *p )
{
	WORD fm;
   BYTE tmpbyte;

	fm = p->filterMask;
	if (fm & FLTR_SRC_RTG) {
		return(GENERAL_FAILURE);
	}
	MSS.MssFilter = fm;

	return(SUCCESS);

}

/*
 * Function: addMulticastAddress( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * The "multicast" address is really the 802.5 Group address.
 * Unlike 802.3 multicast, only a single Group address may be set.
 * First we check to see if there is already a group address set.
 * If there is, return error.
 */

WORD
addMulticastAddress( addMultiType far *p )
{
	/*
	 * We only care about bytes 2-5 of the "multicast" address.
	 * For 802.5, bytes 0,1 are implied to be either C0 00 or 80 00.
	 */

	ulong ga=*(ulong far *)(&p->multiAddr[2]);	/* get the Group address	*/

	WORD	retStatus;

	if (MCBuff.McbCnt){
		return(INVALID_FUNCTION);
	}
	else {
		MCBuff.McbCnt++;
	}

	MCBuff.McbCnt++;
    _bcopy((BYTE far *)p->multiAddr,
		   (BYTE far *)&MCBuff.McbAddrs[0], ADDR_SIZE);

	/*
	 * If we're not open, we don't need to fiddle with the hardware.
	 */
	if ((MSS.MssStatus & MS_OPEN) == 0) {
		op.grpadr = ga;
		return(SUCCESS);
	}
	/*
	 * When we set the Group address, all we care about are the
	 * last 4 bytes (the first two are implied to be either C0 00 or
	 * 80 00).
	 * The gi code treats it like a DWORD.
	 */
	gi_setGroup(ga);

	/*
	 * wait for synchronous completion
	 */
	retStatus = waitForGreqComplete();
	if (retStatus != SUCCESS) {
		MCBuff.McbCnt--;
	}
	op.grpadr = ga;
	return(retStatus);
}

/*
 * Function: deleteMulticastAddress( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * delete a multicast address
 */
WORD
deleteMulticastAddress( addMultiType far *p )
{
	p = NULL;
	if (MCBuff.McbCnt) {
		MCBuff.McbCnt = 0;
		return(SUCCESS);
	}
	else
		return(INVALID_FUNCTION);
}

/*
 * Function: updateStatistics( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * update the statistics table
 */
WORD
updateStatistics( genRequestType far *p )
{
	return(SUCCESS);
}

/*
 * Function: clearStatistics( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * clear the statistics from the table
 */
WORD
clearStatistics( genRequestType far *p )
{
	DWORD **statP = keptStats;
	while (*statP) {
		**statP = 0;
		statP++;
	}
	return(SUCCESS);
}

/*
 * Function: interruptProtocol( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * interrupt the protocol
 */
WORD
interruptProtocol( genRequestType far *p )
{
	return(INVALID_FUNCTION);
}

/*
 * Function: setFuncAddress( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 *  Sets the functional address in the hardware
 */
WORD
setFuncAddress( setFuncAddrType far *p )
{
	WORD	retStatus;
	genReqType *bp;

	ulong fa=*(ulong far *)(p->funcAddr);

	if (fa == MSC.MscCurrFncAdr) {
		return(SUCCESS);
	}

	if (debug & DBG_VERBOSE) {
		printf("setFuncAddr: 0x%x:%x\n",(WORD)(fa>>16),(WORD)fa);
	}

	/*
	 * setup the address
	 */
	MSC.MscCurrFncAdr	=
	op.fncadr			= fa;

	/*
	 * If we're not open, we don't need to fiddle with the hardware.
	 * The functional address will be set via the open parameters.
	 */
	if (!(MSS.MssStatus & MS_OPEN)) {
		return(SUCCESS);
	}

	/*
	 * allocate a GREQ buffer
	 */
	if (!(bp = allocGreqBuff())) {
		return(OUT_OF_RESOURCE);
	}

	/*
	 * setup the general request structure and enqueue it.
	 */
	bp->req = *(genRequestType far *)p;
	enqueueGreqBuff(bp);

	gi_setFunc(fa);

	return(REQUEST_QUEUED);
}

/*
 * Function: setLookahead( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
WORD
setLookahead( setLookaheadType far *p )
{
	return(SUCCESS);
}

/*
 * Function: checkIndPending( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * this function is called from IndicationsOn() when indications have been
 * reenabled.
 * We can assume that interrupts are disabled.
 * If an indication is pending, we need to schedule an interrupt
 * (via SCB_CLEAR) to ensure the indication is processed at interrrupt time.
 *
 */

#define ir_write(n) outpw(giop.iobase + SIF_INT, n)

void
checkIndPending(void)
{
	if (receivePending)
   {
      (BYTE)*(RAMAddress + SRB_Offset) = SCB_RCV_PENDING;
      srb_request();
	}
}

/*
 * Function: ringStatus( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function is called when a change in ring status has been detected.
 * If indications are not enabled the status must be saved and the indication
 * to the protocol stack must be deferred.
 */
void
ringStatus(WORD status)
{
	uchar	indicate = 0xff;

	if (debug & DBG_VERBOSE)
		printf("ringStatus: status 0x%x\n",status);

	if (!(MSS.MssStatus & MS_BOUND))
		return;

	if (ndisGen.indicationNestLevel) {
		statusIndPending = 1;
		siPendingOpcode = RingStatus;
		siPendingParam = status;
	} else {

		/*
	 	 * implicitly turn indications off
	 	 */
	 	ndisGen.indicationNestLevel++;
	    (*pldDsptchTble.PldStatInd)(MCC.CcModuleID,
									status,
									(LPBUF)&indicate,
									RingStatus,
									protCC.CcDataSeg);
		/*
	 	 * if indicate did not get cleared, then turn indications back on.
	 	 */
		ndisGen.indicationNestLevel += indicate;
		ndisGen.needIndCmplt = 1;
	}

}

/*
 * Function: adapterCheck( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function is called when an adapter check interrupt has been detected.
 * If indications are enabled, the status indication is passed to the
 * protocol stack, otherwise, the adapter check reason is saved and a flag
 * is set to indicate that a status indication is pending.
 */
void
adapterCheck(WORD reason)
{
	uchar	indicate = 0xff;

	if (debug & DBG_VERBOSE)
		printf("adapterCheck: reason 0x%x\n",reason);

	if (!(MSS.MssStatus & MS_BOUND))
		return;

	if (ndisGen.indicationNestLevel) {
		statusIndPending = 1;
		siPendingOpcode = AdapterCheck;
		siPendingParam = reason;
	} else {

		/*
	 	 * implicitly turn indications off
	 	 */
	 	ndisGen.indicationNestLevel++;
	    (*pldDsptchTble.PldStatInd)(MCC.CcModuleID,
									reason,
									(LPBUF)&indicate,
									AdapterCheck,
									protCC.CcDataSeg);
		/*
	 	 * if indicate did not get cleared, then turn indications back on.
	 	 */
		ndisGen.indicationNestLevel += indicate;
		ndisGen.needIndCmplt = 1;
	}

}

/*
 * Function: receivePacket( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * this function receives the upcall from the GI executor when a packet has
 * arrived from the network. If indications are enabled (nestLevel == 0) then
 * the indication is given to the protocol stack and the function returns
 * success.
 * currBuff has already (see openAdapter) been set to point to the buffer,
 * all that is passed to us is the size of the recieved packet.
 */
ushort
receivePacket(register WORD size)
{
	uchar	indicate = 0xff;
	ushort	status;

	/*
	 * if the stack has not yet bound, then ignore the packet
	 */
	if (!(MSS.MssStatus & MS_BOUND)) {
		return(SUCCESS);
	}

   /*
    * If our filter is 0 then do not pass packet up to protocol
    */
   if (MSS.MssFilter == 0)
      return(SUCCESS);

	/*
	 * if indications are off, then force the receive to suspend
	 */
	if (ndisGen.indicationNestLevel) {
		return(SUCCESS);
	}

	ndisGen.currBuffSize = size;

	/*
	 * update the receive statistics
	 */
	MSS.MssFRByt += size;
	MSS.MssFR++;
	if ((*(ndisGen.currBuff+10) &
		 *(ndisGen.currBuff+11) &
		 *(ndisGen.currBuff+12) &
		 *(ndisGen.currBuff+13) &
		 *(ndisGen.currBuff+14) &
		 *(ndisGen.currBuff+15)) == 0xff) {
		MSS.MssFRBC++;
		MSS.MssFRBCByt += (DWORD)ndisGen.currBuffSize;
	}
	else if (*(ndisGen.currBuff+10) & 0x80) {
		MSS.MssFRMC++;
		MSS.MssFRMCByt += (DWORD)ndisGen.currBuffSize;
	}

	if (debug & DBG_VERBOSE) {
		printf("receivePacket:\n");
		printData((void far *)ndisGen.currBuff,size);
	}
	if (debug & DBG_TERSE)
		printf("R");

	/*
	 * call the protocol's receiveLookAhead() function
	 */
	ndisGen.indicationNestLevel++;
	status = (*pldDsptchTble.PldRcvLkAhead)(
				MCC.CcModuleID,
				size,
				swaps((WORD)*((WORD far *)ndisGen.currBuff+3)),
				ndisGen.currBuff+8,
				(LPBUF)&indicate,
				protCC.CcDataSeg);

	/*
	 * if the protocol stack wants indications to remain disabled, then
	 * it will clear the indicate flag. Otherwise it will still be set
	 * to -1.
	 */
	ndisGen.indicationNestLevel += indicate;
	ndisGen.needIndCmplt = 1;


	if (debug & DBG_VERBOSE)
		printf("receivePacket: status %d\n",status);
	if (debug & DBG_TERSE)
		printf("%d",(WORD)status);

	return(SUCCESS);
}

/*
 * Function: transferData( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * transfer data using ndisGen.currBuff. according to the NDIS spec, this 
 * function cannot be called unless it is during a receiveLookAhead() call.
 * this guarantees that ndisGen.currBuff is setup.
 */
WORD
transferData( xferDataType far *p )
{
	struct TDDataBlock far *tdp;
	ushort bytesCopied = 0;
	ushort frameOffset;
	ushort numBlocks = p->tdDesc->TDDataCount;
	ushort i;
   ushort BuffLen;
   ulong dataPtr;
   ushort dataLen;
   ushort bLen;

	/*
	 * cache tdp for faster access
	 */
	tdp = p->tdDesc->TDDataBlk;

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x2c, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

	/*
	 * start at the indicated frame offset
	 */
	frameOffset = p->frameOffset;
	ndisGen.currBuffSize -= frameOffset;
   ndisGen.currBuff += 2;

	/*
	 * move numBlocks from the local packet buffer to the locations indicated
	 * byte block descriptors.
	 * currBuffSize has the number bytes remaining in the frame. (Originally
	 * the entire frame).
	 */
	for (i=0; i < numBlocks; i++, tdp++ )
   {
      dataLen = tdp->TDDataLen;
      BuffLen = swaps((WORD)*((WORD far *)ndisGen.currBuff + 2));
      BuffLen -= frameOffset;

   	dataPtr = (dos || (tdp->TDPtrType==2)) ? (ulong)tdp->TDDataPtr :
						 	physToGDT((ulong)tdp->TDDataPtr, bLen, ndisGen.gdt);

      while (dataLen > BuffLen)
      {
         _bcopy(ndisGen.currBuff + 6 + frameOffset, dataPtr, BuffLen);
         dataLen -= BuffLen;
         dataPtr += BuffLen;
         bytesCopied += BuffLen;
         ndisGen.currBuffSize -= BuffLen;
         frameOffset = swaps((WORD)*((WORD far *)ndisGen.currBuff));
         (ulong)ndisGen.currBuff &= (ulong)0x0000FFFFL;
         (ulong)ndisGen.currBuff |= (ulong)frameOffset << 16;
         BuffLen = swaps((WORD)*((WORD far *)ndisGen.currBuff + 2));
         frameOffset = 0;
      }

		bLen = (dataLen <= ndisGen.currBuffSize) ?
						         dataLen : ndisGen.currBuffSize;

		if (bLen)
      {
   		_bcopy(	ndisGen.currBuff + 6 + frameOffset, dataPtr, bLen);
			frameOffset += bLen;
			bytesCopied += bLen;
		}
		if (!(ndisGen.currBuffSize -= bLen)) {
			break;
		}

	}			  

	/*
	 * update bytes copied before return
	 */
	*p->bytesCopied = bytesCopied;

	if (debug & DBG_TERSE)
		printf("T%d",bytesCopied);

	return(SUCCESS);
}

/*
 * Function: indicationsComplete( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * the hardware interrupt handler will call this function when all packets
 * for this interrupt have been received and processed. this function expects
 * interrupts to be enabled when the call is made.
 */
void
indicationsComplete(void)
{
	WORD iFlag;
	DISABLEI(iFlag);
	if (ndisGen.needIndCmplt) {
		ndisGen.needIndCmplt = 0;
		RESTOREI(iFlag);
		(*pldDsptchTble.PldIndComplete)(MCC.CcModuleID,protCC.CcDataSeg);
		return;
	}
	RESTOREI(iFlag);
}

WORD
adapterInUse(void)
{
   return(0);
}

void
adapterEnable(void)
{
}

