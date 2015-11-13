;
; file: $Header:   \ibm16\_uprim.inc  2-2-93  jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;	This file describe the data structures that the MAC upper dispatch
;	table C function primitives receive from their corresponding ASM
;	interface functions. The ASM interface functions provide a far
;	pointer to a stack based structure.
;

;
; transmit chain
;
xmitChain struc
	xmitMacDs		dw ?
	xmitTxDesc		dd ?
	xmitReqHandle	dw ?
	xmitProtID		dw ?
xmitChain ends

;/*
; * transfer data structure
; */
xferData struc
	xferMacDs		dw ?
	xferTdDesc		dd ?
	xferFrameOffset	dw ?
	xferBytesCopied	dd ?
xferData ends


