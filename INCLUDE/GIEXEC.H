/*
 * File: $Header:   \ibm16\giexec.h  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *      Generic interface defintions 
 */

#ifndef GIEXEC_H
#define GIEXEC_H

#include "types.h"
#include "adapter.h"

/*
 * this is the maximum size of a receive packet
 */
#define MAX_RCV_PKT_SIZE 2048
#define MAX_TX_PKT_SIZE MAX_RCV_PKT_SIZE

/* Maximum number of pending transmits cached to the adapter */

#define MAXPENDING 2

extern ushort txgdts[];

/* These are gi_state values */

#define GIS_NOTHING 0	/* GI never called */
#define GIS_RESET   1	/* Bringup has been performed */
#define GIS_CLOSED 2	/* Initialization has been performed */
#define GIS_OPEN   3	/* Adapter Open has been performed */

typedef uchar NODE[6];		    			/* definition of a GI node address */

/* GI function codes */

#define GIF_STARTUP	1	    /* perform Bringup, Init, and Open */
#define GIF_SHUTDOWN	2	    /* reset adapter and unhook interrupts */
#define GIF_RECEIVE	3	    /* Not used */
#define GIF_TRANSMIT	4	    /* Transmit a packet */
#define GIF_BRINGUP	5	    /* Perform Bringup only */
#define GIF_INIT	6	    /* Perform Init only */
#define GIF_CLOSE	7	    /* Close the adapter */
#define GIF_CANCEL	8	    /* Cancel a transmit */
#define GIF_COUNTERS	9	    /* Read Adapter Error Log (counters) */
#define GIF_CANCELSOCK	10	    /* Cancel transmits for a given socket */
#define GIF_READADAP	11	    /* Read from adapter RAM */


/* GI return codes for GCB and ECB requests */

#define GIR_NOTHING	   -1	/* Operation is still progress */
#define GIR_SUCCESS	    0	/* Operation completed successfully */
#define GIR_EBRINGUP	    1	/* Bringup failed */
#define GIR_EINIT	    2	/* Init failed */
#define GIR_EOPEN	    3	/* Open failed */
#define GIR_DISALLOWED	    4	/* Adapter not in correct state for op */
#define GIR_NOTSUPPORTED    5	/* Bad function code */
#define GIR_ABORTED	    6	/* Adapter de-inserted while op in prog. */
#define GIR_CANCELLED	    7	/* Transmit was cancelled by caller */
#define GIR_ETRANSMIT	    8	/* Error in transmit (only if GI has a bug) */
#define GIR_UNREACHABLE     9	/* Destination address not recognized */
#define GIR_NOTCOPIED	    10	/* Transmitted frame was not copied */
#define GIR_NOTFOUND	    11	/* Cancel request didn't match anything */

#define GIFL_FRAGLIST	1	/* GCB Flag - pointer is to a fragment list */
#define GIFL_SUSPRCV	2	/* Not used */

/* This structure defines a fragment in a transmit fragment list */

typedef struct gifrag
{
    uchar far *address;
    ushort count;
} GIFRAG;

/* The Generic Call Block */

typedef struct gcb
{
    struct gcb far *link;
    ushort  status;
    NODE    node;
    ushort  size;
    uchar 	*pointer;
    uchar   flags;
} GCB;

/* GI Open Parameters */

typedef struct giop
{
    ushort  iobase;		/* I/O base address */
    ushort  irqline;		/* Interrupt Request Line */
    ushort  shoff;		/* Shared Memory Offset */
    ushort  shseg;		/* Shared Memory Segment */
    ushort  dmaline;		/* DMA Request/Ack Line */
} GIOP;

extern GIOP giop;

extern OPEN_PARAMS op;

typedef GCB *PACKET;

extern ushort		gi_bringup(void);
extern void			gi_transmit(PACKET packet);
extern void			gi_exit(void);
extern ushort		gi_startup(void);
extern ushort		gi_open(ushort options);
extern void 		gi_setGroup(ulong);
extern void 		gi_setFunc(ulong);
extern ushort		gi_close(void);
extern void			gi_counters(uchar far *);
extern void			sysconfig(void);
#endif
