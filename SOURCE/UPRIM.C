/*
 * File: $Header:   \ibm16\uprim.c   2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *
 *		This file contains some of the MAC upper dispatch function 
 *		primitives that are completely generic across all projects.
 *
 *			WORD transmitChain( xmitChainType far *p );
 *			WORD generalRequests( register genRequestType far *p );
 *			WORD systemRequest( register sysRequestType far *p );
 *		
 *		TxComplete invokes the protocol lower dispatch function 
 *		for notifying transmit complete. It also cleans up any 
 *    resources allocated for the transmit.
 *
 *			void txComplete(WORD ndisStatus );
 *	
 */
#include <stdio.h>
#include "uprim.h"
#include "misc.h"
#include "hwbuff.h"

#ifndef XMITCH
/*
 *	Function: transmitChain
 *
 *	Inputs:
 *		p	-	points to a transmit chain info structure provided by the 
 *				protocol
 *
 *	Outputs:
 *				This function returns SUCCESS if the transmit got started
 *				successfully. If it fails later on, it is txComplete() job
 *				to notify the protocol that the transmit failed.
 *
 *	Side Effects:
 *				transmitChain() copies the TxBufDesc structure referenced
 *				by p->TxBufDesc into a local txBuffType buffer structure.
 *				The txBuffType is allocated (dequeued) from a pool of free 
 *				transmit resources using the allocTxBuff() macro. After the
 *				txBuffType has been setup, then it is enqueued onto a transmit
 *				busy queue. If it is the only element on the queue, then
 *				hwXmit() is called to perform the transmit. Otherwise, when
 *				the currently in progress transmit completes, txComplete() 
 *				will schedule the next transmit.
 */
WORD
transmitChain( register xmitChainType far *p )
{
	register txBuffType *txp;
	WORD immedLen;

	/*
	 * allocate a TX buffer structure, bail out if none. Running out of 
	 * transmit resources is legal and may be a common occurrence.
	 */
	if (!(txp = allocTxBuff()))
		return(OUT_OF_RESOURCE);

	/*
	 * save the TxBufDesc in the txBuffType just allocated. copy no more then is
	 * referenced by TxDataCount.
	 */
	_bcopy((void far *)p->txDesc, (void far *)(&txp->tx),
		sizeof(WORD)+sizeof(LPBUF)+sizeof(WORD)+
		(p->txDesc->TxDataCount*sizeof(struct TxDataBlock)));

	/*
	 * set the protocol ID and request handle
	 */
	txp->protID			= p->protID;
	txp->reqHandle		= p->reqHandle;

	/*
	 * copy the immediate data if any and set the TxImmedPtr in the txBuffType
	 * buffer.
	 */
	if (immedLen = p->txDesc->TxImmedLen) {
		_bcopy(	(void far *)p->txDesc->TxImmedPtr,
				(void far *)(txp->immed),
				immedLen);
	}
	txp->tx.TxImmedPtr = (LPBUF)txp->immed;

	/*
	 * call the hardware specific transmit function. txEnQueue returns the
	 * previous value of the head pointer. if NULL, then the list was empty
	 * and hwXmit() needs to be called. hwXmit() must be able to depend on
	 * the fact that it will be called only when it is legal to start a transmit.
	 */
	if (!enqueueTxBuff(txp))
		hwXmit();

	/*
	 * assume that hwXmit() and txComplete() will handle transmit completion
	 * notification to the protocol stack.
	 */
	return(REQUEST_QUEUED);
}
#endif

#ifndef XMITCH
/*
 *	Function: txComplete
 *
 *	Inputs:
 *
 *		ndisStatus - status of the completed transmit. The status code must
 *			be a legal NDIS status code, suitable to pass up to the protocol
 *			stack.
 *
 *	Outputs:
 *
 *		none
 *
 *	Side Effects:
 *
 *		txComplete() manages freeing of the transmit resources allocated by
 *		transmitChain() as well as notifying the protocol stack of the
 *		completed transmit.
 *
 */
void 
txComplete(WORD ndisStatus )
{
	register	txBuffType	*bp;
	register	txBuffType	*newBp;
	WORD					reqHandle;
	WORD					protID;
	WORD					iFlag;

	/*
	 * get the queue values under exclusion. this prevents a protocol hooked
	 * to a timer tick from sending a transmit chain request in between the
	 * time dequeueTxBuff() and headTxQueue() happens.
	 */
	DISABLEI(iFlag)

	bp		= dequeueTxBuff();
	newBp	= headTxQueue();

	RESTOREI(iFlag)

	/*
	 * check that bp exists
	 */
	if (debug && !bp)
		panic(1);

	/*
	 * If the busy queue is not empty, then start the next transmit.
	 */
	if (newBp)
		hwXmit();

	/*
	 * save the protocol ID and request handle before freeing this transmit
	 * buffer. These and the ndisStatus are the only information that needs
	 * to be carried forward to the PldXmitConfirm call. It is best to free
	 * the transmit buffer before notifying the stack because it is possible
	 * that the stack may call transmitChain() within the context of the
	 * PldXmitConfirm call. If transmit resources are tight, then this will
	 * help.
	 */
	reqHandle = bp->reqHandle;
	protID = bp->protID;

	/*
	 * release the transmit buffer
	 */
	freeTxBuff(bp);

	/*
	 * make the upcall to the protocol if the request handle is non-zero
	 */
	if (reqHandle)
		iFlag=(*pldDsptchTble.PldXmitConfirm)(	protID,
											MCC.CcModuleID,
											reqHandle,
											ndisStatus,
											protCC.CcDataSeg );
	if (debug & DBG_VERBOSE) {
		printf("txComplete: %s retStatus %x reqHandle %x\n\n",
				ndisStatus ? "BAD" : "SUCCESS",
				iFlag,
				reqHandle);
	}
	if (debug & DBG_TERSE) {
		printf("C");
	}
}
#endif

/*
 * Define the general request function call table. Each element of the call
 * table corresponds to a general request OP code. Each of the general request
 * functions expects a far pointer to a structure. In all cases, the structure
 * referenced is the same size, but may be interpreted (or casted) in a
 * manner specific to the function. In general, the general request functions
 * are instantiated in HW.C.
 */
typedef WORD (*gReqFuncType)(void far *);
gReqFuncType gReqFuncTab[] = {
	0,
	initiateDiagnostics,
	readErrorLog,
	setStationAddress,
	openAdapter,
	closeAdapter,
	resetMAC,
	setPacketFilter,
	addMulticastAddress,
	deleteMulticastAddress,
	updateStatistics,
	clearStatistics,
	interruptProtocol,
	setFuncAddress,
	setLookahead
};

/*
 *	Function: generalRequests
 *
 *	Inputs:
 *
 *		p - a far pointer to a generic general request structure. This
 *			pointer is not actually used here, rather it is simply passed
 *			on to the request handler.
 *
 *	Outputs:
 *
 *		status - The status of the handler call is returned to the protocol.
 *
 *	Side Effects:
 *
 *		generalRequests() is called from the assembly interface function
 *		GENERALREQUESTS. GENERALREQUESTS provides a far pointer to the
 *		information passed on the stack by the protocol. Part of this
 *		information is the OP code of the request to be performed. The OP code
 *		is used to index into gReqFuncTab[] to make an indirect call to a
 *		specific request handler function.
 */
WORD
generalRequests( genRequestType far *p )
{
	register WORD opCode = p->opCode;

	/*
	 * force interrupts to be enabled.
	 */
	_enableI();

	/*
	 * Check for opcode in range, launch the indirect call if so. Otherwise,
	 * return an error code.
	 */
	if (opCode <= SetLookAhead && opCode >= InitiateDiagnostics) {
		return((*gReqFuncTab[opCode])((void far *)p));
	}
	else
		return(INVALID_FUNCTION);
}

/*
 * Function: set4GbGDT
 *
 * Inputs:
 *		none
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 *		This function initialises a privilege level 0 GDT to be a 32 bit
 *		physical 0 based selector for use in transferData(). Since the OS
 *		does not yet support this, some magic numbers are used that 
 *		correspond directly to selector formats as described in the 80386
 *		programmer reference. The selector only gets init'd if this is OS/2
 *		on an 80386 or above.
 *
 */
void
setFourGbGDT(void)
{
	/*
	 * build a physical 0 based GDT selector. then 
	 * find a pointer to it in the GDT and manipulate the limit and
	 * granularity bits directly.
	 */
	if (os2 && !(ndisGen.procType & SIXTEEN_BIT)) {

		char far *gdtp;
		
		/*
		 * get the selector assigned a physical base of 0 with a limit of
		 * 64K.
		 */
		(void)physToGDT(0L, 0xffff, ndisGen.gdt0);

		/*
		 * another OS/2 trick that i know is that the first GDT selector (1)
		 * is self referential, i.e., it points to the beginning of the GDT 
		 * table. remember that the GDT always begins with at least 1 NULL
		 * selector.
		 */
		gdtp = (char far *)0x00080000L;

		/*
		 * now index into the GDT table so that gdtp points to the selector
		 * allocated to hw.gdt0. each descriptor in the GDT is an 8 byte 
		 * quantity. a selector is a direct index into this table if you mask
		 * off the low 3 bits.
		 */
		gdtp += (ndisGen.gdt0 & (~7));

		/*
		 * set the granularity bit so that the descriptor limit is 256 Mb.
		 */
		*(gdtp+6) |= 0x80;

		/*
		 * set the descriptor privilege level to 0
		 */
		*(gdtp+5) &= (~0x60);

		/*
		 * force the limit bits to the max
		 */
		*(gdtp+0) |= 0xff;
		*(gdtp+1) |= 0xff;
		*(gdtp+6) |= 0x0f;
	}

	/*
	 * scrub so that it causes a NULL GDT fault if used.
	 */
	else {
		ndisGen.gdt0 = 0;
	}
}

/*
 *	Function: systemRequest
 *
 *	Inputs:
 *
 *		p - a far pointer to the System Request information structure
 *			provided by SYSTEMREQUEST. Part of the information structure
 *			is the System Request OP code. The only OP code supported by
 *			this model is the Bind OP code. The Bind OP code requests the
 *			transfer of table pointers to/from the upper layer.
 *
 *	Outputs:
 *
 *		status - The status of the bind request.
 *
 *	Side Effects:
 *
 *		systemRequest() supports only the Bind request. At bind time, the 
 *		common characteristics table pointers are traded between the MAC and 
 *		the upper layer. As an optimization, the contents of the upper layer's
 *		lower dispatch table and common characteristics table is copied into 
 *		local memory to avoid a segment register hit for every reference.
 */
WORD
systemRequest( sysRequestType far *p )
{
	/*
	 * determine the processor type at ring 0
	 */
	ndisGen.procType = procType();

	/*
	 * init a 4 Gb 0 based GDT. This is the only place that a level 0 
	 * privilege called is guarenteed.
	 */
	if (os2 && !ndisGen.gdt0set) {
		ndisGen.gdt0set++;
		setFourGbGDT();
	}
 
	/*
	 * verify OP code sanity
	 */
	if (p->opCode == Bind) {

		/*
		 * cache the protocol's CC table in local RAM for quick access.
		 */
		protCC = *(struct CommChar far *)p->charTab;

		/*
		 * cache the protocol's lower dispatch table
		 */
		pldDsptchTble = *protCC.CcLDp;
		
		/*
		 * write the address of our CC table into space provided by the
		 * protocol. Note the use of a level 0 fixed up pointer. See
		 * INITOS.C for further details regarding pointer fixups.
		 */
		*p->tabAddr = (DWORD)MUD.MudCCp;

		/*
		 * note in the Specific charactristics table that the MAC is now
		 * bound.
		 */
		MSS.MssStatus |= (DWORD)MS_BOUND;

		/*
		 * If the OP code is correct, then success is assured.
		 */
		return(SUCCESS);
	}
	return(!SUCCESS);
}
