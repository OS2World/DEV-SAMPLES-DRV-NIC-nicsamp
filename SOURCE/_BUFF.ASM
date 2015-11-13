;
; $Header:   \ibm16\_buff.asm    2-2-93   jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
; 	This file contains the buffer queue support functions. they are NOT 
;	assembled if BUFF_C is defined.

.xlist
include segments.inc
include _misc.inc
include _buff.inc
include devhlp.inc
.list

;
; standard locations on the stack for incoming buffer pointers
;
	bpPtr	equ	word ptr [bp+6]
	buffPtr	equ word ptr [bp+4]

_TEXT segment

	assume CS:CGROUP, DS:DGROUP

;----------------------------------------------------------------------------
;
; Function: _sizeBuffsStruct
;
; Inputs:
;	none
;
; Outputs:
;	returns the size of the buffer structure that the assembler version of
;	the buffer functions uses.
;
; Side Effects:
;
;	return the size of the buffsStruc structure that is in use by the 
;	assembler version. It should match sizeof(buffType).
;
; WORD sizeBuffsStruc(void)
;
	public	_sizeBuffsStruc
_sizeBuffsStruc	proc near
	mov	ax, size buffsStruc
	ret
_sizeBuffsStruc	endp

;----------------------------------------------------------------------------
;
; Function: _sizeBufStruct
;
; Inputs:
;	none
;
; Outputs:
;	returns the size of the bufStruc in use by the assembler version of the
;	buffer functions. It should match sizeof(bufStructType).
;
; WORD sizeBufStruct(void)
;
	public	_sizeBufStruct
_sizeBufStruct	proc near
	mov	ax, size bufStruc
	ret
_sizeBufStruct	endp

ifndef BUFF_C
;----------------------------------------------------------------------------
;
; Function: _allocBuff
;
; Inputs:
; 	p - pointer to a buffer structure
;
; Outputs:
; 	returns a pointer to a generic queue element, NULL if none. NULL is
;	not an error.
;
; Side Effects:
;
; 	This function allocates a queue element from the head of the free 
;	list. The application is then expected to either free it (using 
;	freeBuff()) or enqueue it using enqueueTail(). Until such time, the
;	queue element is not associated with or linked into any list.
;
;
; bufStructType *allocBuff( register buffType *p )

;{
;	register bufStructType	*bp;
;	register WORD			iFlag;
;
;	/*
;	 * check for empty list
;	 */
;	if (!p->free)
;		return((bufStructType *)NULL);
;
;	DISABLEI(iFlag)
;
;	/*
;	 * pop the first from the list and update the queue head pointer
;	 */
;	bp = p->free;
;	p->free = bp->next;
;
;	RESTOREI(iFlag)
;
;	/*
;	 * return the allocated buffer
;	 */
;	return(bp);
;}
	public	_allocBuff
_allocBuff	proc near
	ENTRY
	push	si

	; setup buffPtr

	mov	bx, buffPtr

	; disable interrupts before looking at memory cached pointers

	disableIntr

	; check for empty list

	mov	si, word ptr [bx].buffsFree
	or	si, si
	jz	_xAllocBuff

	; unlink from the free list

	mov	ax, word ptr [si].next
	mov	word ptr [bx].buffsFree, ax

_xAllocBuff:

	; restore interrupts and exit

	restoreIntr	ax
	mov	ax, si
	pop	si
	EXIT
	ret
_allocBuff	endp

;----------------------------------------------------------------------------
;
; Function: freeBuff
;
; Inputs:
; 	p	- pointer to a buffer structure
;	bp	- pointer to a queue element (buffer pointer)
;
; Outputs:
;	none
;
; Side Effects:
;
;	This function enqueues (or restores) an unlinked queue element to 
;	the head of the free list.
;
; void freeBuff(register buffType *p, register bufStructType *bp)
;

;{
;	WORD	iFlag;
;
;	DISABLEI(iFlag)
;
;	bp->next = p->free;
;	p->free = bp;
;
;	RESTOREI(iFlag)
;}

	public	_freeBuff
_freeBuff	proc near
	ENTRY
	push	si
	push	di

	; get the buffsStruc pointer

	mov	si, buffPtr
	mov	di, bpPtr

	; add to the head of the free list

	disableIntr
	mov	ax, word ptr [si].buffsFree
	mov	word ptr [di].next, ax
	mov	word ptr [si].buffsFree, di
	restoreIntr di

	; cleanup the stack and exit

	pop	di
	pop	si
	EXIT
	ret
_freeBuff	endp

;----------------------------------------------------------------------------
;
; Function: enqueueTail
;
; Inputs:
; 	p	- pointer to a buffer structure
; 	bp	- pointer to a queue element (buffer pointer)
;
; Outputs:
; 	returns a pointer to the head of the list before bp is enqueued, 
; 	NULL if it was empty.
;
; Side Effects:
;
; 	This function enqueues an unlinked queue element to 
; 	the end of the busy list. Work is assumed to proceed in the order that
; 	it was given, hence the reason for enqueueing at the end of the list.
;
;
; bufStructType *enqueueTail( buffType *p, bufStructType *bp)
;

;{
;	bufStructType *prevHead;
;	WORD	iFlag;
;
;	DISABLEI(iFlag)
;
;	/*
;	 * save head pointer
;	 */
;	prevHead = p->busy;
;
;	/*
;	 * link the buffer onto the end of the busy list
;	 */
;
;	/*
;	 * list is not empty
;	 */
;	if (p->endBusy) {
;		p->endBusy->next = bp;
;	}
;
;	/*
;	 * empty list
;	 */
;	else {
;		p->busy	= bp;
;	}
;
;	/*
;	 * update the tail pointer
;	 */
;	p->endBusy = bp;
;
;	/*
;	 * terminate the list
;	 */
;	bp->next = (bufStructType *)NULL;
;
;	RESTOREI(iFlag)
;
;	return(prevHead);
;}
	public	_enqueueTail
_enqueueTail	proc near
	ENTRY
	push	si
	push	di

	; get p and bpPtr

	mov	si, buffPtr
	mov	di, bpPtr

	; provide interrupt exclusion

	disableIntr

;	/*
;	 * save head pointer
;	 */
;	prevHead = p->busy;
;

	mov	ax, word ptr [si].buffsBusy	; don't trash AX after this point

;	/*
;	 * list is not empty
;	 */
;	if (p->endBusy) {
;		p->endBusy->next = bp;
;	}
;
;	/*
;	 * empty list
;	 */
;	else {
;		p->busy	= bp;
;	}

	mov	bx, word ptr [si].buffsEndBusy
	or	bx, bx
	jz	emptyList
	mov	word ptr [bx].next, di
	jmp	short updateList

emptyList:

	mov	word ptr [si].buffsBusy, di

;	/*
;	 * update the tail pointer
;	 */
;	p->endBusy = bp;

updateList:

	mov	word ptr [si].buffsEndBusy, di

;	/*
;	 * terminate the list
;	 */
;	bp->next = (bufStructType *)NULL;

	mov	word ptr [di].next, 0

_xEnqueueTail:

	restoreIntr di
	pop	di
	pop	si
	EXIT
	ret
_enqueueTail	endp

;----------------------------------------------------------------------------
;
; Function: dequeueHead
;
; Inputs:
;	p	- pointer to a buffer structure
;
; Outputs:
;	returns a pointer to the queue element that was dequeued from the
;	head of the busy queue. If the queue was empty, then a panic is
;	initiated.
;
; Side Effects:
;
;	This function enqueues an unlinked queue element to 
;	the end of the busy list. Work is assumed to proceed in the order that
;	it was given, hence the reason for enqueueing at the end of the list.
;
;
; bufStructType *dequeueHead(register buffType *p)
;

;{
;	WORD					iFlag;
;	register bufStructType	*bp;
;
;	/*
;	 * check for empty queue or bp not at head of queue, panic if it is
;	 */
;	if (debug && !p->busy)
;		panic(3);
;
;	DISABLEI(iFlag)
;
;	/*
;	 * pop bp from the front of the busy queue. it that leaves an empty
;	 * queue, then update endBusy.
;	 */
;	bp = p->busy;
;	if (!(p->busy = bp->next))
;		p->endBusy = (bufStructType *)NULL;
;
;	RESTOREI(iFlag)
;
;	return(bp);
;}

	public	_dequeueHead
_dequeueHead	proc near
	ENTRY
	push	si
	push	di

	; provide exclusion

	disableIntr

	; setup p

	mov	si, buffPtr

;	bp = p->busy;

	mov	di, word ptr [si].buffsBusy

;	/*
;	 * pop bp from the front of the busy queue. it that leaves an empty
;	 * queue, then update endBusy.
;	 */
;	if (!(p->busy = bp->next))
;		p->endBusy = (bufStructType *)NULL;

	mov	ax, word ptr [di].next
	mov	word ptr [si].buffsBusy, ax
	or	ax, ax
	jnz	_xDequeueHead
	mov	word ptr [si].buffsEndBusy, 0

_xDequeueHead:

	restoreIntr	ax
	mov	ax, di
	pop	di
	pop	si
	EXIT
	ret
_dequeueHead	endp

endif

_TEXT ends

end

