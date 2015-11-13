/*
 * File: $Header:   \ibm16\hwbuff.h  2-2-93  jess fahland $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file describes the buffer structures and functions used by this
 * 	NDIS driver.
 */
#ifndef HWBUFF_H
#define HWBUFF_H
#include "ndis.h"
#include "buff.h"
#include "hw.h"
#include "giexec.h"

/*
 * these are the currently defined buffer areas. the index numbers must
 * correspond to the structure initialization in 'buff.c'. the structure
 * def is published so that MACROS can take advantage of compile time
 * address generation.
 */
#define TX_BUFF_INDEX	0
#define GREQ_BUFF_INDEX	1

#define MAX_TX_BUFFS	20
#define MAX_GREQ_BUFFS	16

/*
 * a transmit buffer structure must contain at least enough space for a 
 * Transmit Chain descriptor
 */
typedef struct txBuff {
	/*
	 * use this link to maintain free and busy queues
	 */
	bufStructType	link;

	/*
	 * transmit buffer descriptor
	 */
	struct TxBufDesc	tx;

	/*
	 * protocol ID and request handle.
	 */
	WORD	protID;
	WORD	reqHandle;

	/*
	 * buffer for immediate data
	 */
	BYTE immed[MAX_IMMED_LEN];

	/*
	 **************************************************************************
	 **************************************************************************
	 *
	 * this section of the transmit buffer is driver and hardware dependent.
	 *
	 **************************************************************************
	 **************************************************************************
	 */

	/*
	 * must translate from txBufDesc to GCB structures
	 */
	GCB		gcb;

} txBuffType;

/*
 * general requests must save some parameters for the request notification
 */
typedef struct {
	bufStructType	link;
	genRequestType	req;

	/*
	 **************************************************************************
	 **************************************************************************
	 *
	 * this section of the general request buffer is driver and hardware 
	 * dependent.
	 *
	 **************************************************************************
	 **************************************************************************
	 */

	WORD				status;
	WORD				subcode;
} genReqType;

/*
 * init all buffer structures managed by this module
 */
extern void			buffInit(void);


/*
 * alloc/enqueue/free TX buffer structures
 */

/*
 * alloc/free queue elements from the free list.
 */
#define allocTxBuff() (txBuffType *)allocBuff(&buffs[TX_BUFF_INDEX])

#define freeTxBuff(bp) freeBuff(&buffs[TX_BUFF_INDEX],\
						   		(bufStructType *)(bp))

#define enqueueTxBuff(bp) (txBuffType *)enqueueTail(&buffs[TX_BUFF_INDEX],\
													(bufStructType *)(bp))
#define dequeueTxBuff() (txBuffType *)dequeueHead(&buffs[TX_BUFF_INDEX])

#define headTxQueue() (txBuffType *)queueHead(TX_BUFF_INDEX)

/*
 * alloc/enqueue/free General Request buffer structures. the MACROS behave
 * in the same way as for the preceeding TX buffer MACROS.
 */
#define allocGreqBuff() (genReqType *)allocBuff(&buffs[GREQ_BUFF_INDEX])
#define freeGreqBuff(bp) freeBuff(&buffs[GREQ_BUFF_INDEX],\
							 	(bufStructType *)(bp))

#define enqueueGreqBuff(bp) (genReqType *)enqueueTail(&buffs[GREQ_BUFF_INDEX],\
													(bufStructType *)(bp))
#define dequeueGreqBuff() (genReqType *)dequeueHead(&buffs[GREQ_BUFF_INDEX])

#define headGreqQueue() (genReqType *)queueHead(GREQ_BUFF_INDEX)


#endif
