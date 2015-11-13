;
; $Header:   \ibm16\_buff.inc   2-2-93  jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;      This file describes the buffer structures and functions used by
;      the NDIS generic model.  The structure definitions in this file
;      must exactly parallel those in buff.h.
;

;
; a buffer structure is at a minimum a linked list.
;
;struct bufStruct {
;	struct bufStruct *next;
;};
;typedef struct bufStruct bufStructType;
 
bufStruc struc
	next	dw ?
bufStruc ends

;
; a buffer area is defined in terms of it's structure 'size', the number
; 'num' of structures in the buffer 'buff'. it is maintained as a linked
; list which means that the first element in the structures pointed at
; by 'buff' must be a bufStructType. buffers are always either on the free
; or busy list.
;
;typedef struct {
;	struct bufStruct	*free;
;	struct bufStruct	*busy;
;	struct bufStruct	*endBusy;
;	unsigned short		size;
;	unsigned short		num;
;	struct bufStruct	*buff;
;} buffType;
;

buffsStruc struc
	buffsFree		dw ?
	buffsBusy		dw ?
	buffsEndBusy	dw ?
	buffsSize		dw ?
	buffsNum		dw ?
	buffsPtr		dw ?
buffsStruc ends

