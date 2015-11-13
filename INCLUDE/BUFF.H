/*
 * file: $Header:   \ibm16\buff.h  2-2-93 jess fahland $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file describes the buffer structures and functions used by the
 *	   NDIS generic model.
 */
#ifndef BUFF_H
#define BUFF_H

/*
 * Struct: bufStruct
 *
 * A buffer structure is at a minimum a linked list. In fact, the first
 * element of a buffer structure must be it's link. The buffer manipulation
 * functions know nothing more about the internals of the buffer structure
 * other than it's 'next' pointer.
 */
struct bufStruct {
	struct bufStruct *next;
};
typedef struct bufStruct bufStructType; /* shorthand */
	
/*
 * Struct: buffType
 *
 * A buffer area is defined in terms of it's structure 'size' and the number
 * 'num' of structures in the buffer 'buff'. It is maintained as a linked
 * list which means that the first element in the structures pointed at
 * by 'buff' must be a bufStructType. buffers are always either on the free
 * or busy list. It is expected that the functions in BUFF.C and _BUFF.ASM
 * are used to manipulate the buffer linkages.
 */
typedef struct {
	struct bufStruct	*free;
	struct bufStruct	*busy;
	struct bufStruct	*endBusy;
	unsigned short		size;
	unsigned short		num;
	struct bufStruct	*buff;
} buffType;

/*
 * Each project defines some buffer resources.
 */
extern buffType buffs[];

/*
 * This macro provides access to the head of the busy queue for a particular
 * buffer type (determined by 'index').
 */
#define queueHead(index) buffs[(index)].busy

/*
 * prototype the buffer manipulation functions.
 */
extern bufStructType *allocBuff( buffType * );
extern void freeBuff(buffType *, bufStructType *);

extern bufStructType *enqueueTail( buffType *, bufStructType *);
extern bufStructType *dequeueHead(buffType *);

extern void buffInit(void);

#endif
