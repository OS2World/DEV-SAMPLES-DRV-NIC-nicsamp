;
; file: $Header:   \bim16\_hwbuff.inc  2-2-93  jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;	This file describes the driver specific buffer structures and
;	functions used by this NDIS driver.
;
include gi.inc

;
; these are the currently defined buffer areas. the index numbers must
; correspond to the structure initialization in 'buff.c'. the structure
; def is published so that MACROS can take advantage of compile time
; address generation.
;
TX_BUFF_INDEX	equ 0
GREQ_BUFF_INDEX	equ 1

NUM_TX_LIST equ MAX_TX_DATABLK+1

;
; a transmit buffer structure must contain at least enough space for a 
; Transmit Chain descriptor. the first section of the structure must be kept
; up to date with the one in _hwbuff.inc.
;

txBuff struc

;	
;	 use this link to maintain free and busy queues
;	
	link	dw ?

;	
;	 transmit buffer descriptor
;	
	tx		db size TxBufDesc dup(?)

;	
;	  protocol ID and request handle.
;	 
	protID		dw ?
	reqHandle	dw ?

;	
;	  buffer for immediate data
;	 
	immed		db MAX_IMMED_LEN dup(0)

;	
;	 **************************************************************************
;	 **************************************************************************
;	 *
;	 * this section of the transmit buffer is driver and hardware dependent.
;	 *
;	 **************************************************************************
;	 **************************************************************************
;	 

;
; must translate from txBufDesc to GCB structures
;
	txgcb			db size GCB dup(0)


txBuff ends

;
;  alloc/enqueue/free TX buffer structures
; 

;
;  alloc/free queue elements from the free list.
; 
; #define allocTxBuff() (txBuffType *)allocBuff(&buffs[TX_BUFF_INDEX])
allocTxBuff	MACRO
ifdef OS2
	push	offset _buffs
	call	_allocBuff
	add		sp, 2
else
	push	si
	mov		si, offset _buffs
	push	si
	call	_allocBuff
	add		sp, 2
	pop		si
endif
ENDM

;#define freeTxBuff(bp) freeBuff(&buffs[TX_BUFF_INDEX],\
;						   		(bufStructType *)(bp))

freeTxBuff MACRO bpPtr
ifdef OS2
	push	bpPtr
	push	offset _buffs
	call	_freeBuff
	add		sp, 4
else
	push	si
	push	bpPtr
	mov		si, offset _buffs
	push	si
	call	_freeBuff
	add		sp, 4
	pop		si
endif
ENDM


;#define enqueueTxBuff(bp) (txBuffType *)enqueueTail(&buffs[TX_BUFF_INDEX],\
;													(bufStructType *)(bp))

enqueueTxBuff MACRO bpPtr
ifdef OS2
	push	bpPtr
	push	offset _buffs
	call	_enqueueTail
	add	sp, 4
else
	push	si
	push	bpPtr
	mov		si, offset _buffs
	push	si
	call	_enqueueTail
	add		sp, 4
	pop		si
endif
ENDM

;#define dequeueTxBuff() (txBuffType *)dequeueHead(&buffs[TX_BUFF_INDEX])

dequeueTxBuff MACRO
ifdef OS2
	push	offset _buffs
	call	_dequeueHead
	add		sp, 2
else
	push	si
	mov		si, offset _buffs
	push	si
	call	_dequeueHead
	add		sp, 2
	pop		si
endif
ENDM

;#define headTxQueue() (txBuffType *)queueHead(TX_BUFF_INDEX)

extrn _buffs:BYTE
headTxQueue equ	word ptr _buffs.buffsBusy
	
