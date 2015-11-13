;
; $Header:   \ibm16\_uprimt.asm   2-2-93  jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;
;   This file instantiates the MAC upper dispatch table primitive functions:
;
; 	SYSTEMREQUEST
; 	TRANSMITCHAIN
; 	TRANSFERDATA
; 	RECEIVERELEASE
; 	INDICATIONSON
; 	INDICATIONSOFF
; 		
;   These are assembly interface functions to standard C functions. Each ASM 
;   function pushes a FAR pointer to the stack parameters and calls it's 
;   associated C support function. The functions that use this method are
;   relatively low bandwidth. INDICATIONSOFF and INDICATIONSON do not use 
;   this method, however, as they are short and very low level.

.xlist
;
; standard segment definitions
;
include segments.inc
;
; NDIS structure and MACRO definitions
;
include ndis.inc
;
; misc support functions
;
include _misc.inc
;
; get Device Helper function codes
;
include devhlp.inc

;
; include these files if the assembly version of TransmitChain is to be used.
; This requires that the project contain _HWBUFF.INC.
;
ifdef XMITCH
include _buff.inc
include _uprim.inc
include _hwbuff.inc
else
;
; use the only the generic model include files
;
include _uprim.inc
endif


.list

;
;-----------------------------------------------------------------------------
;
;
; These MACROs are used to setup and restore DS on a call from the protocol.
; DS is always in the same place on the stack when the protocol calls 
; these functions.
;
SETUP_DS macro

	; setup the stack frame
	ENTRY

	; save all C registers used

	push	ds
	push	bx

	; compute an offset to the last parameter that the calling function
	; put on the stack. the stack will look like:
	;
	;	|-------------|
	;	| 	MAC DS	  | last parameter stacked by calling function
	;	|-------------|
	;	|	ret CS	  |
	;	|-------------|
	;	| 	ret addr  |
	;	|-------------|
	;	|	bp		  |
	;	|-------------| <-- bp
	;	|	ds		  |
	;	|-------------- <-- sp
	;
	dsBpOffset	equ	FAR_BP_OFFSET

	; get the DS value on the stack and put it in DS, then stack a far 
	; pointer to the start of the structure on the stack.

ifdef OS2
	lea	bx, word ptr [bp+dsBpOffset]
	mov	ds, word ptr ss:[bx]
else
	lea	bx, word ptr [bp+dsBpOffset]
	push	word ptr ss:[bx]
	pop	ds
endif

	;
	; the function to be called expects a FAR pointer to the structure on
	; the stack.
	;
	push	ss
	push	bx

endm

; restore saved registers and unwind the stack

RESTORE_DS macro

	; unload the C function parameter

	add	sp,4

	; restore saved registers

	pop	bx
	pop	ds

	; unwind this stack frame

	EXIT

endm

;
;-----------------------------------------------------------------------------
;

;
; this macro defines the assembly function and calls the C interface 
; function. Inputs are the assembly function name, the associated C function
; name, and the number of stack WORDs to return (PASCAL calling conventions).
;
CALL_C	macro	aFunc, cFunc, stackWords
	aFunc	proc far
	public	aFunc
	SETUP_DS
	call	cFunc
	RESTORE_DS
	ret	stackWords*2
	aFunc	endp
endm

;
;-----------------------------------------------------------------------------
;

_TEXT segment

	assume CS:CGROUP, DS:DGROUP

;
; The buffer manipulation functions are instantiated in BUFF.C and _BUFF.ASM.
;
	extrn _allocBuff:NEAR
	extrn _freeBuff:NEAR
	extrn _enqueueTail:NEAR
	extrn _dequeueHead:NEAR
	extrn _hwXmit:NEAR

;
; Publicize the entry points to the indications functions.
;
	public	INDICATIONSON
	public	INDICATIONSOFF


;
; EXTRN the C functions that pair up with the Upper Dispatch primitives.
; In general, these functions are instantiated in UPRIM.C and HW.C.
;
ifndef XMITCH
;
; If the assembly version of TransmitChain is to be used, then don't EXTRN it.
;
		extrn	_transmitChain:near
endif
		extrn	_transferData:near
		extrn	_receiveRelease:near
		extrn	_generalRequests:near
		extrn	_systemRequest:near

;
; Instantiate the Upper Dispatch Table entry points.
;

;
; Function: __TRANSMITCHAIN
;
; Inputs:
;
; Outputs:
;	returns result of transmitChain()
;
; Side Effects:
;	This function is an upper level dispatch table entry point. It provides
;	a function specific entry point, yet enforces a standard method for 
;	packaging the information passed on the stack and initializing the 
;	DS register. As per the NDIS spec, the target DS is passed on the stack.
;
CALL_C	__TRANSMITCHAIN,		_transmitChain,		5

;
; Function: __TRANSFERDATA
;
; Inputs:
;
; Outputs:
;	returns result of transferData()
;
; Side Effects:
;	This function is an upper level dispatch table entry point. It provides
;	a function specific entry point, yet enforces a standard method for 
;	packaging the information passed on the stack and initializing the 
;	DS register. As per the NDIS spec, the target DS is passed on the stack.
;
CALL_C	__TRANSFERDATA,			_transferData,		6

;
; Function: __RECEIVERELEASE
;
; Inputs:
;
; Outputs:
;	returns result of receiveRelease()
;
; Side Effects:
;	This function is an upper level dispatch table entry point. It provides
;	a function specific entry point, yet enforces a standard method for 
;	packaging the information passed on the stack and initializing the 
;	DS register. As per the NDIS spec, the target DS is passed on the stack.
;
CALL_C	__RECEIVERELEASE,		_receiveRelease,	2

;
; Function: __GENERALREQUESTS
;
; Inputs:
;
; Outputs:
;	returns result of generalRequests()
;
; Side Effects:
;	This function is an upper level dispatch table entry point. It provides
;	a function specific entry point, yet enforces a standard method for 
;	packaging the information passed on the stack and initializing the 
;	DS register. As per the NDIS spec, the target DS is passed on the stack.
;
CALL_C	__GENERALREQUESTS,		_generalRequests,	7

;
; Function: __SYSTEMREQUEST
;
; Inputs:
;
; Outputs:
;	returns result of systemRequest()
;
; Side Effects:
;	This function is an upper level dispatch table entry point. It provides
;	a function specific entry point, yet enforces a standard method for 
;	packaging the information passed on the stack and initializing the 
;	DS register. As per the NDIS spec, the target DS is passed on the stack.
;
CALL_C	SYSTEMREQUEST,			_systemRequest,		7

;-----------------------------------------------------------------------------
;
; Function: INDICATIONSOFF
;
; Inputs:
;
;	on entry the stack looks like:
;
;	|---------------|
;	|	MAC DS	|
;	|---------------| 6
;	|	ret CS	|
;	|---------------| 4
;	|	ret OFF	|
;	|---------------| 2
;	|	BP		|
;	|---------------| <-- BP
;
; Outputs:
;	returns SUCCESS in AX
;
; Side Effects:
;	This far PASCAL assembler function increments the nest level byte and 
;	clears IF in the flags
;
	indicationsOffMACDS	equ	word ptr [bp+FAR_BP_OFFSET]

INDICATIONSOFF	proc far

	ENTRY
	push	ds

	; setup my DS register

	mov	ax, indicationsOffMACDS
	mov	ds, ax

	; increment the indications nest level flag and return with interrupts
	; disabled.

	cli
	inc	_ndisGen.indicationNestLevel

	; return SUCCESS

	mov	ax, SUCCESS

	; restore the protocol DS value

	pop	ds

	EXIT
	ret	2
INDICATIONSOFF	endp

;-----------------------------------------------------------------------------
;
; Function: INDICATIONSON
;
; Inputs:
;
;	on entry the stack looks like:
;
;	|---------------|
;	|	MAC DS	|
;	|---------------| 6
;	|	ret CS	|
;	|---------------| 4
;	|	ref OFF	|
;	|---------------| 2
;	|	BP		|
;	|---------------| <-- BP
;
; Outputs:
;	always returns SUCCESS
;
; Side Effects:
;	This far PASCAL assembler function decrements the nest level byte. 
;	If the nest level is 0, then check to see
;	if indications are pending by calling checkIndPending(). The project 
;	must supply this function. It is generally used to schedule an interrupt
;	to process suspended indications (if any).
;

indicationsOnMACDS	equ	word ptr [bp+FAR_BP_OFFSET]

INDICATIONSON	proc far

	extrn	_checkIndPending:near

	ENTRY
	push	ds

	; setup my DS register

	mov	ax, indicationsOnMACDS
	mov	ds, ax

	; IndicationOn must return interrupts disabled.

	cli

	; decrement the nest level. If zero, then call checkIndPending() to allow
	; the driver to schedule the pending indication.

	dec	_ndisGen.indicationNestLevel
	jnz	_xIndicationsOn

	; Indications are back on, check to see if indications are pending.

	call	_checkIndPending

_xIndicationsOn:

	; return SUCCESS

	mov	ax, SUCCESS

	; restore the protocol DS and BX values

	pop	ds

	EXIT
	ret	2
INDICATIONSON	endp

ifdef XMITCH
;-----------------------------------------------------------------------------
;
; If this MACRO is defined, then assemble the high speed version of 
; TransmitChain(). During the debug phase, it is recommended that the C 
; version be used for better debug output. The assembly version also
; requires that the project provide _HWBUFF.INC as an include file that
; parallels HWBUFF.H.
;

; A full description of transmitChain is provided in UPRIM.C. The C comments
; from that function are provided here as an algorithm guide.

;
;	Function: _transmitChain
;
;	Inputs:
;		p -	points to a transmit chain info structure provided by the 
;			protocol
;
;	Outputs:
;		This function returns SUCCESS if the transmit got started
;		successfully. If it fails later on, it is txComplete() job
;		to notify the protocol that the transmit failed.
;
;	Side Effects:
;		transmitChain() copies the TxBufDesc structure referenced
;		by p->TxBufDesc into a local txBuffType buffer structure.
;		The txBuffType is allocated (dequeued) from a pool of free 
;		transmit resources using the allocTxBuff() macro. After the
;		txBuffType has been setup, then it is enqueued onto a transmit
;		busy queue. If it is the only element on the queue, then
;		hwXmit() is called to perform the transmit. Otherwise, when
;		the currently in progress transmit completes, txComplete() 
;		will schedule the next transmit.
;
; WORD transmitChain( register xmitChainType far *p )
;

;{
;	register txBuffType *txp;
;
;	/*
;	 * allocate a TX buffer structure, bail out if none
;	 */
;	if (!(txp = allocTxBuff()))
;		return(OUT_OF_RESOURCE);
;
;	/*
;	 * save the TxBufDesc in the TX buff just allocated. copy no more then is
;	 * referenced by TxDataCount.
;	 */
;	_bcopy((void far *)p->txDesc, (void far *)FNDATA(&txp->tx),
;		sizeof(WORD)+sizeof(LPBUF)+sizeof(WORD)+
;		(p->txDesc->TxDataCount*sizeof(struct TxDataBlock)));
;
;	/*
;	 * set the protocol ID and request handle
;	 */
;	txp->protID			= p->protID;
;	txp->reqHandle		= p->reqHandle;
;
;	/*
;	 * copy the immediate data if any and set the TxImmedPtr in the txBuffType
;	 * buffer.
;	 */
;	if (p->txDesc->TxImmedLen)
;		_bcopy((void far *)p->txDesc->TxImmedPtr, 
;				(void far *)FNDATA(txp->immed),
;				p->txDesc->TxImmedLen);
;	txp->tx.TxImmedPtr = (LPBUF)txp->immed;
;
;	/*
;	 * call the hardware specific transmit function. txEnQueue returns the
;	 * previous value of the head pointer. if NULL, then the list was empty
;	 * and hwXmit() needs to be called.
;	 */
;	if (!enqueueTxBuff(txp))
;		hwXmit();
;
;	return(REQUEST_QUEUED);
;}

	;
	;	This is how the stack appears after the ENTRY prologue.
	;
	;	|-------------|
	;	| ChainPtr Hi |
	;	|-------------|
	;	| ChainPtr Lo |
	;	|-------------|
	;	| 	ret addr  |
	;	|-------------|
	;	|	bp		  |
	;	|-------------| <-- bp
	;

	;
	; pointer to the stack parameter
	;
	chainPtr	equ	word ptr [bp+NEAR_BP_OFFSET]

	;
	; Define local storage to store the contents of the GDTR
	;
	gdtrSize	equ 6
	gdtrPtr		equ [bp-gdtrSize]
	pType		equ word ptr [bp-gdtrSize-2]

	public _transmitChain
_transmitChain	proc near

	ENTRY	gdtrSize+2
	push	si
	push	di

	;
	; on entry, i know that ss:bx points to a xmitChainType structure
	; on the stack. this is a short cut!!! and avoids a segment register
	; hit.
	;


;	/*
;	 * allocate a TX buffer structure, bail out if none
;	 */
;	if (!(txp = allocTxBuff()))
;		return(OUT_OF_RESOURCE);
;
; 	keep bx from getting trashed
;

;        mov     ax,HARDWARE_ERROR
;        jmp     _errXtransmitChain

	push	bx
	allocTxBuff
	pop	bx
	or	ax, ax
	jnz	_goodTxp
	mov	ax, OUT_OF_RESOURCE
	jmp	_errXtransmitChain

_goodTxp:

	;
	; setup the processor type for later use
	;
	mov	dx, _ndisGen.procType
	mov	pType, dx

	; ds:[si] --> p->txDesc, es:[di] --> txp
	; ss:[bx] points to xmitChainType on the stack.

	;
	; save DS, BCOPY will trash it.
	;
	push	ds

	;
	; setup to walk the Tx chain
	;
	mov	cx, ds
	mov	es, cx
	mov	di, ax
	lds	si, dword ptr ss:[bx].xmitTxDesc

;	/*
;	 * set the protocol ID and request handle
;	 */
;	txp->reqHandle		= p->reqHandle;
;	txp->protID			= p->protID;
;
	mov	ax, word ptr ss:[bx].xmitProtId
	mov	word ptr es:[di].protID, ax
	mov	ax, word ptr ss:[bx].xmitReqHandle
	mov	word ptr es:[di].reqHandle, ax

;	/*
;	 * save the TxBufDesc in the TX buff just allocated. copy no more then is
;	 * referenced by TxDataCount.
;	 */
;	_bcopy((void far *)p->txDesc, (void far *)FNDATA(&txp->tx),
;		sizeof(WORD)+sizeof(LPBUF)+sizeof(WORD)+
;		(p->txDesc->TxDataCount*sizeof(struct TxDataBlock)));
;
	mov	cx, word ptr ds:[si].TxDataCount
	shl	cx, 1
	shl	cx, 1
	shl	cx, 1
	add	cx, 8

	push	si
	push	di
	lea	di, word ptr es:[di].tx
	mov	dx, pType
	bcopy	ds								; defined in _MISC.INC
	pop	di
	pop	si

;	/*
;	 * copy the immediate data if any and set the TxImmedPtr in the txBuffType
;	 * buffer.
;	 */
;	_bcopy((void far *)p->txDesc->TxImmedPtr, (void far *)FNDATA(txp->immed),
;			p->txDesc->TxImmedLen);
;

	; remember that ES and DS are still swapped, hence the segment override.

;
; for the 16/4 adapter we check to see if the frame size is larger than
; the maximum size supported in the MSC table.  If it is then we return
; invalid parameter.
;
        mov     ax,word ptr ds:[si].TxImmedLen
        push    si
        mov     cx,word ptr ds:[si].TxDataCount
        jcxz    CountDone
AddFrameBytes:
        add     si,8
        add     ax,word ptr ds:[si].TxDataLen
        loop    AddFrameBytes
CountDone:
        pop     si
        cmp     ax,word ptr es:_MSC.MscTBlkSz
        jbe     FrameSizeOK
	mov	ax,INVALID_PARAMETER

        pop     ds
	pop	di
	pop	si
	EXIT
	ret
        
FrameSizeOK:
	cmp	word ptr ds:[si].TxImmedLen, 0
	je	_immedDone
	mov	cx, word ptr ds:[si].TxImmedLen
	push	ds
	push	si
	push	di
	lea	di, word ptr es:[di].immed
	lds	si, dword ptr ds:[si].TxImmedPtr
	mov	dx, pType
	bcopy	ds
	pop	di
	pop	si
	pop	ds

;	txp->tx.TxImmedPtr = (LPBUF)txp->immed;

	lea	ax, word ptr es:[di].immed
	mov	word ptr es:[di].tx.TxImmedPtr, ax
	mov	word ptr es:[di].tx.TxImmedPtr+2, es

_immedDone:

	pop	ds

;	/*
;	 * call the hardware specific transmit function. txEnQueue returns the
;	 * previous value of the head pointer. if NULL, then the list was empty
;	 * and hwXmit() needs to be called.
;	 */
;	if (!enqueueTxBuff(txp))
;		hwXmit();
;
	enqueueTxBuff	di
	or	ax, ax
	jnz	_xTransmitChain
	call	_hwXmit

_xTransmitChain:

	mov	ax, REQUEST_QUEUED

_errXtransmitChain:

	pop	di
	pop	si
	EXIT
	ret
_transmitChain	endp
endif


;
;-----------------------------------------------------------------------------
;

ifdef XMITCH

;
; If this MACRO is defined, then assemble the high speed version of 
; txComplete(). During the debug phase, it is recommended that the C 
; version be used for better debug output. The assembly version also
; requires that the project provide _HWBUFF.INC as an include file that
; parallels HWBUFF.H.
;

; A full description of txComplete() is provided in UPRIM.C. The C comments
; from that function are provided here as an algorithm guide.

;
;	Function: txComplete
;
;	Inputs:
;		ndisStatus - status of the completed transmit. The status code must
;			be a legal NDIS status code, suitable to pass up to the protocol
;			stack.
;
;	Outputs:
;
;		none
;
;	Side Effects:
;
;		txComplete() manages freeing of the transmit resources allocated by
;		transmitChain() as well as notifying the protocol stack of the
;		completed transmit.
;
;
; void txComplete(WORD ndisStatus )
;

;{
;	register	txBuffType	*bp;
;	register	txBuffType	*newBp;
;	WORD					reqHandle;
;	WORD					protID;
;	WORD					iFlag;
;
;	/*
;	 * get the queue values under exclusion. this prevents a protocol hooked
;	 * to a timer tick from sending a transmit chain request in between the
;	 * time dequeue and headTxQueue happens.
;	 */
;	DISABLEI(iFlag)
;
;	bp		= dequeueTxBuff();
;	newBp	= headTxQueue();
;
;	RESTOREI(iFlag)
;
;	/*
;	 * check that bp exists
;	 */
;	if (debug && !bp)
;		panic(1);
;
;	/*
;	 * start the next transmit
;	 */
;	if (newBp)
;		hwXmit();
;
;	/*
;	 * save the protocol ID and request handle before freeing this transmit
;	 * buffer.
;	 */
;	reqHandle = bp->reqHandle;
;	protID = bp->protID;
;
;	/*
;	 * release the transmit buffer
;	 */
;	freeTxBuff(bp);
;
;	/*
;	 * make the upcall to the protocol if the request handle is non-zero
;	 */
;	if (reqHandle)
;		(*pldDsptchTble.PldXmitConfirm)(	protID,
;											MCC.CcModuleID,
;											reqHandle,
;											ndisStatus,
;											protCC.CcDataSeg );
;}
	ndisStatus	equ	word ptr [bp+NEAR_BP_OFFSET]

	public	_txComplete
_txComplete	proc near

	ENTRY

	;
	; SI gets trashed
	;
	push	si

;	/*
;	 * get the queue values under exclusion. this prevents a protocol hooked
;	 * to a timer tick from sending a transmit chain request in between the
;	 * time dequeue and headTxQueue happens.
;	 */
;	DISABLEI(iFlag)
;
;	bp		= dequeueTxBuff();
;	newBp	= headTxQueue();
;
;	RESTOREI(iFlag)
;
	disableIntr
	dequeueTxBuff
	mov	si, ax
	mov	ax, headTxQueue
	restoreIntr	DX

;	/*
;	 * start the next transmit
;	 */
;	if (newBp)
;		hwXmit();
;
	or	ax, ax
	jz	_noWork
	call	_hwXmit

_noWork:

;	/*
;	 * save the protocol ID and request handle before freeing this transmit
;	 * buffer.
;	 */
;	reqHandle = bp->reqHandle;
;	protID = bp->protID;
;
	mov	dx, word ptr [si].reqHandle
	mov	bx, word ptr [si].protID

;	/*
;	 * release the transmit buffer
;	 */
;	freeTxBuff(bp);
;
	freeTxBuff si

;	if (reqHandle)
;		(*pldDsptchTble.PldXmitConfirm)(protID,
;										MCC.CcModuleID,
;										reqHandle,
;										ndisStatus,
;										protCC.CcDataSeg );
	or	dx, dx
	jz	_xTxComplete
	push	bx
	push	word ptr _MCC.CcModuleID
	push	dx
	push	ndisStatus
	push	word ptr _protCC.CcDataSeg
	call	dword ptr [_pldDsptchTble].PldXmitConfirm

_xTxComplete:
	pop	si
	EXIT
	ret
_txComplete	endp

endif

_TEXT ends

end
