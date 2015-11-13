/*
 * file: $Header:   \ibm16\buff.c  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *
 * 		The functions contained in this file provide generic list 
 * 		manipulation. The features include:
 *
 *			Init		-	initialize the buffer queue elements.
 * 			Allocate	- 	allocate a free buffer element from the free list.
 * 			Free		- 	restore a buffer to the free list.
 * 			Enqueue Tail- 	enqueue a buffer element at the end of the busy 
 * 							list.
 *			Dequeue Head-	dequeue a buffer element from the head of the busy
 *							queue.
 *
 *		A high perormance version of these functions can be made if BUFF_C is
 *		not defined. This requires _buff.asm to be assembled and that the 
 *		project provide an assembly version of hwbuff.h (_hwbuff.inc).
 *		BUFF_C should be defined for the initial debug phase since the C 
 *		versions of these functions provide more error checking at runtime.
 * 			
 */
#include <stdio.h>
#include "misc.h"
#include "buff.h"

/*
 * Function: gBuffInit
 *
 * Inputs:
 * 		p - pointer to a buffer structure
 *
 * Outputs:
 * 		none
 *
 * Side Effects:
 *
 * 		This function provides generic buffer initialization. 
 *		The memory pointed at by p->buff is assumed to be a contiguous block
 *		of memory, sufficient to contain p->num blocks of size p->size. Each
 *		of the blocks referenced by p->buff is also assumed to start with a
 *		generic link structure of bufStructType. The generic link enables the
 *		queue manipulation functions to operate on the queue elements without
 *		knowing the contents, other then the link itself.
 */
static void
gBuffInit( register buffType *p)
{
	WORD					i;
	register bufStructType	*bp	= p->buff;
	WORD 					num	= p->num - 1;
	WORD					iFlag;

	/*
	 * make sure nothing interferes
	 */
	DISABLEI(iFlag)

	/*
	 * setup free and busy list pointers, busy always starts empty.
	 */
	p->free		= bp;
	p->busy		= 
	p->endBusy	= (bufStructType *)NULL;

	/*
	 * build the linked list of free buffers. queue elements are carved out
	 * of memory in a linear fashion.
	 */
	for (i=0; i<num; i++) {
		bp->next = (bufStructType *)((char *)bp + p->size);
		bp = bp->next;
	}

	/*
	 * the last element in the list gets grounded.
	 */
	bp->next = (bufStructType *)NULL;

	/*
	 * restore the interrupt flag
	 */
	RESTOREI(iFlag)
}

#ifdef BUFF_C
/*
 * Function: allocBuff
 *
 * Inputs:
 * 		p - pointer to a buffer structure
 *
 * Outputs:
 * 		returns a pointer to a generic queue element, NULL if none. NULL is
 *		not an error.
 *
 * Side Effects:
 *
 * 		This function allocates a queue element from the head of the free 
 *		list. The application is then expected to either free it (using 
 *		freeBuff()) or enqueue it using enqueueTail(). Until such time, the
 *		queue element is not associated with or linked into any list.
 */
bufStructType *
allocBuff( register buffType *p )
{
	register bufStructType	*bp;
			 WORD			iFlag;

	/*
	 * check for empty list, return NULL if so.
	 */
	if (!p->free)
		return((bufStructType *)NULL);

	/*
	 * interrupts can really mess things up
	 */
	DISABLEI(iFlag)

	/*
	 * pop the first element from the list and update the queue head pointer.
	 */
	bp = p->free;
	p->free = bp->next;

	RESTOREI(iFlag)

	/*
	 * return the allocated buffer
	 */
	return(bp);
}

/*
 * Function: freeBuff
 *
 * Inputs:
 * 		p	- pointer to a buffer structure
 *		bp	- pointer to a queue element (buffer pointer)
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 *		This function enqueues (or restores) an unlinked queue element to 
 *		the head of the free list.
 */
void
freeBuff(register buffType *p, register bufStructType *bp)
{
	WORD	iFlag;

	/*
	 * prevent interrupts from scrambling things
	 */
	DISABLEI(iFlag)

	/*
	 * add the element to the head of the free list.
	 */
	bp->next = p->free;
	p->free = bp;

	RESTOREI(iFlag)
}

/*
 * Function: enqueueTail
 *
 * Inputs:
 * 		p	- pointer to a buffer structure
 *		bp	- pointer to a queue element (buffer pointer)
 *
 * Outputs:
 *		returns a pointer to the head of the list before bp is enqueued, 
 *		NULL if it was empty.
 *
 * Side Effects:
 *
 *		This function enqueues an unlinked queue element to 
 *		the end of the busy list. Work is assumed to proceed in the order that
 *		it was given, hence the reason for enqueueing at the end of the list.
 */
bufStructType *
enqueueTail( register buffType *p, register bufStructType *bp)
{
	bufStructType *prevHead;
	WORD	iFlag;

	/*
	 * prevent interrupts
	 */
	DISABLEI(iFlag)

	/*
	 * save the head pointer as the final output.
	 */
	prevHead = p->busy;

	/*
	 * To link the element onto the end of the busy list, one must be aware of
	 * 2 cases; the list is empty, or it is not.
	 */

	/*
	 * the list is not empty
	 */
	if (p->endBusy) {
		p->endBusy->next = bp;
	}

	/*
	 * empty list
	 */
	else {
		p->busy	= bp;
	}

	/*
	 * A pointer to the end of the busy list is maintained for quick 
	 * enqueueing. This avoids a list traversal from the head to the tail to
	 * find the last element.
	 */
	p->endBusy = bp;

	/*
	 * terminate the list
	 */
	bp->next = (bufStructType *)NULL;

	RESTOREI(iFlag)

	/*
	 * return the original head of the list.
	 */
	return(prevHead);
}

/*
 * Function: dequeueHead
 *
 * Inputs:
 * 		p	- pointer to a buffer structure
 *
 * Outputs:
 *		returns a pointer to the queue element that was dequeued from the
 *		head of the busy queue. If the queue was empty, then a panic is
 *		initiated.
 *
 * Side Effects:
 *
 *		This function enqueues an unlinked queue element to 
 *		the end of the busy list. Work is assumed to proceed in the order that
 *		it was given, hence the reason for enqueueing at the end of the list.
 */
bufStructType *
dequeueHead(register buffType *p)
{
	WORD					iFlag;
	register bufStructType	*bp;

	/*
	 * check for empty queue or bp not at head of queue, panic if it is.
	 */
	if (!p->busy)
		panic(3);

	/*
	 * prevent interrupts
	 */
	DISABLEI(iFlag)

	/*
	 * pop bp from the front of the busy queue. it that leaves an empty
	 * queue, then set endBusy to NULL.
	 */
	bp = p->busy;
	if (!(p->busy = bp->next))
		p->endBusy = (bufStructType *)NULL;

	RESTOREI(iFlag)

	/*
	 * return a pointer to the dequeue element.
	 */
	return(bp);
}
#endif

/*
 * initialize the internal buffer structures
 */
extern WORD sizeBuffsStruc(void);
extern WORD sizeBufStruct(void);

/*
 * Function: buffInit
 *
 * Inputs:
 * 		none
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 *		This function assumes the existence of a user provided buffType 
 *		structure named buffs. It is also assumed that buffs is an array of
 *		buffType structures, terminated when buffs[].buff == NULL. The 
 *		following variables of buffType must be initialized:
 *
 *			num		- number of queue elements
 *			size	- size of each queue element.
 *			buff	- points to a contiguous block of memory large enough to
 *					  contain 'num' elements of 'size' bytes per element.
 *
 *		buffInit() processes each buffs[] array element by calling 
 *		gBuffInit() with a pointer to the buffs[] element in question. If
 *		BUFF_C is not defined, then 2 assembly functions are used to 
 *		determine if the assembly queue element size matches the C queue
 *		element size.
 */
void
buffInit(void)
{
	buffType *p = buffs;

#ifndef BUFF_C
	/*
	 * check that the assembly code matches the C code
	 */
	if (sizeof(buffType) != sizeBuffsStruc() ||
		sizeof(bufStructType) != sizeBufStruct()) {
		printf("buffInit: buffer size incompatibility\n");
		panic(21);
	}
#endif

	/*
	 * process each buffs[] element until buffs[].buff==NULL.
	 */
	while (p->buff) {
		gBuffInit(p++);
	}
}
