/*
 * File: $Header:   \ibm16\adapter.h   2-2-93 jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *      These are the adapter defines for control and error codes
 */
#include "types.h"

#ifndef ADAPTER_H
#define ADAPTER_H

#define SIF_DATA	0   /* word */
#define SIF_DATAI	2   /* word */
#define SIF_ADDR	4   /* word */
#define SIF_INT 	6   /* word */
#define DATABUF 	8   /* byte */
#define CONBUF		9   /* byte */
#define LENBUF		10  /* word */
#define ADRBUF		12  /* 3 bytes */
#define IRQ_REARM	15  /* byte */

/* IBM 16/4 Tokenring I/O port address offsets */
#define SETUPREAD1 0
#define RESETLATCH 1
#define SETUPREAD2 2
#define RESETRELEASE 2
#define INTENABLE 3

/* MMIO Attachment control area register offsets. */
#define RRREVEN 0x1E00
#define RRRODD 0x1E01
#define WRBREVEN 0x1E02
#define WRBRODD 0x1E03
#define WWOREVEN 0x1E04
#define WWORODD 0x1E05
#define WWCREVEN 0x1E06
#define WWCRODD 0x1E07
#define ISRPEVEN 0x1E08
#define ISRPEVEN_RESET 0x1E28
#define ISRPEVEN_SET 0x1E48
#define ISRPODD 0x1E09
#define ISRPODD_RESET 0x1E29
#define ISRPODD_SET 0x1E49
#define ISRAEVEN 0x1E0A
#define ISRAODD 0x1E0B
#define ISRAODD_RESET 0x1E2B
#define ISRAODD_SET 0x1E4B
#define TCREVEN 0x1E0C
#define TCREVEN_RESET 0x1E2C
#define TCREVEN_SET 0x1E4C
#define TCRODD 0x1E0D
#define TVREVEN 0x1E0E
#define TVRODD 0x1E0F
#define SRPREVEN 0x1E18
#define SRPRODD 0x1E19

/* Individual bits in SIF_INT control/status register */

#define IR_SRESET	0xff00
#define IR_INT_ADAP	0x8000
#define IR_ADAP_RST	0x4000
#define IR_SSB_CLR	0x2000
#define IR_EXECUTE	0x1000
#define IR_SCB_REQ	0x0800
#define IR_RCV_CONT	0x0400
#define IR_RCV_VALID	0x0200
#define IR_XMT_VALID	0x0100
#define IR_INT_SYS	0x0080
#define IR_INIT 	0x0040
#define IR_TEST 	0x0020
#define IR_ERROR	0x0010

/* System Interrupt reason codes */

#define IR_COD_MASK	0x000e

#define IRC_ADAP_CHK	0x0000
#define IRC_IMPL_FRC	0x0001
#define IRC_RING_STS	0x0002
#define IRC_SCB_CLR	0x0003
#define IRC_CMD_STS	0x0004
#define IRC_RCV_STS	0x0005
#define IRC_XMT_STS	0x0006

/* Adapter Bring-Up Error codes */

#define IR_ERR_MASK	0x000f

#define IRE_INITIAL	0x0000
#define IRE_ROM_CRC	0x0001
#define IRE_RAM 	0x0002
#define IRE_INSTRUCT	0x0003
#define IRE_CONTEXT	0x0004
#define IRE_PROTOCOL	0x0005
#define IRE_SYSTEM	0x0006


/* A copy of this structure is filled in when the adapter is initialized. */

typedef struct SRB_init_results
{
   uchar Command;
   uchar Status;
   ulong Reserved;
   ushort Bringup_Code;
   ushort Encode_Address;
   ushort Level_Address;
   ushort Adapter_Address;
   ushort Parms_Address;
   ushort Mac_Address;
} SRB_INIT_RESULTS;

/* The Bringup_Code field from the SRB_INIT_RESULTS structure can be as follows */
#define COULD_NOT_EXECUTE 0x0020


/* Adapter Initialization Error codes */

/* Adapter initialization block structure */

typedef struct adap_iblock
{
    ushort  options;
    uchar   command,
	    transmit,
	    receive,
	    ring,
	    scb_clear,
	    adap_check;
    ushort  rcv_burst,
	    xmt_burst,
	    dma_abort;
    ulong   scb_addr,
	    ssb_addr;
} ADAP_IBLOCK;
				
/* Adapter initialization option bits */

#define AIO_MUSTSET	0x8000
#define AIO_PARITY	0x6000
#define AIO_BRST_SCB	0x1000
#define AIO_BRST_LIST	0x0800
#define AIO_BRST_LSTAT	0x0400
#define AIO_BRST_RCV	0x0200
#define AIO_BRST_XMT	0x0100

/* Adapter System Command Block */

typedef struct scb
{
    ushort  command;
    char   *address;
} SCB;
			      
/* Adapter System Command/Status Block codes */

#define SCB_RCV_PENDING	0		/* Special status for NDIS driver	*/
#define SCB_INTCHECK	-2
#define SCB_ADAPCHECK	-1
#define SCB_RINGSTATUS	0x84
#define SCB_REJECT	2
#define SCB_OPEN	3
#define SCB_TRANSMIT	0x82
#define SCB_TRANSMITDIR 0x0A
#define SCB_TRANSMITCOMPLETE 0x0F
#define SCB_RECEIVE	0x81
#define SCB_CLOSE	4
#define SCB_SETGROUP	6
#define SCB_SETFUNC	7
#define SCB_READLOG	8
#define SCB_READADAP	11
#define SCB_IMPLENABLE	12
#define SCB_MODIFYPARMS	1
#define SCB_PREOPEN 0x13

/* 
 * The SSB status block has an extra word to accomodate the Adapter
 * Check status. (GI.ASM must agree.)
 */

typedef struct ssb
{
    short   command;
    ushort  status[4];
} SSB;

/*
 * Bit definitions for the Open adapter option word.
 * Definitions are byte-swapped.
 */
#define	WRAP_INTERFACE				0x0080
#define	DISABLE_HARD_ERROR			0x0040
#define	DISABLE_SOFT_ERROR			0x0020
#define	PASS_ADAPTER_MAC_FRAMES		0x0010
#define	PASS_ATTENTION_MAC_FRAMES	0x0008
#define	PAD_ROUTING_FIELD			0x0004
#define	FRAME_HOLD					0x0002
#define	CONTENDER					0x0001
#define	PASS_BEACON_MAC_FRAMES		0x8000

#define	COPY_ALL_MAC_FRAMES			0x0400
#define	COPY_ALL_NON_MAC_FRAMES		0x0200
#define	PASS_FIRST_BUFFER_ONLY		0x0100

#define TAP_TRACEMODE   0x0300

#define NUM_RCV_BLOCKS 29
typedef struct open_params
{
   uchar
      command,
      reserved1[7];
    ushort
	    options;
    uchar
	    nodadr[6];
	ulong
	    grpadr,
	    fncadr;
    ushort
       rcv_num,
	    rcv_size,
	    xmt_size;
   uchar
	   xmt_num,
      reserved2,
      max_sap,
      max_sta,
      max_gsap,
      max_gmem,
      t1_tick1,
      t2_tick1,
      ti_tick1,
      t1_tick2,
      t2_tick2,
      ti_tick2,
      product_id[18],
      pad;

} OPEN_PARAMS;

typedef struct iopl
{
    ulong   link;
    ushort  cstat;
    ushort  size;
    struct afrag
    {
	ushort	count;
	ulong	address;
    } frag[3];
    uchar *xmt;
} IOPL;

/*
 * cstat bit definitions
 */
#define CSTAT_VALID					0x0080
#define CSTAT_FRAME_COMPLETE		0x0040
#define CSTAT_START_FRAME			0x0020
#define CSTAT_END_FRAME				0x0010
#define CSTAT_FRAME_INTERRUPT		0x0008
#define CSTAT_TRANSMIT_ERROR		0x0004
#define CSTAT_PASS_CRC				0x0002
#define CSTAT_PASS_SOURCE_ADDR		0x0001
#define CSTAT_TRAMSIT_ERR_CODE		0xff00

#define CSTAT_XMIT_ERR_ACCESS_PRIORITY	0x0800
#define CSTAT_XMIT_ERR_STRIP			0x2300
#define CSTAT_XMIT_ERR_MAC_FRAME		0x2400
#define CSTAT_XMIT_ERR_LINK				0x2700
#define CSTAT_XMIT_ERR_XMIT_LEN			0x2800
#define CSTAT_XMIT_ERR_STATION_ID		0x4000
#define CSTAT_XMIT_ERR_PROTOCOL_ERR		0x4100
#define CSTAT_XMIT_ERR_ROUTING			0x4400
#define CSTAT_XMIT_ERR_FRAME_TYPE		0xf000
#define CSTAT_XMIT_ERR_NOT_READY		0xfe00
#define CSTAT_XMIT_ERR_READY			0xff00


typedef struct counters
{
    ulong   line,
	    res1,
	    burst,
	    ari_fci,
	    abort_delimiter,
	    res2,
	    lost_frame,
	    rcv_congestion,
	    frame_copied,
	    res3,
	    token,
	    res4,
	    dma_bus,
	    dma_parity;
} COUNTERS;

/*
 * transmit status bit definitions
 */
#define XMIT_STAT_COMMAND_COMPLETE		0x8000
#define XMIT_STAT_FRAME_COMPLETE		0x4000
#define XMIT_STAT_LIST_ERROR			0x2000
#define XMIT_STAT_I_FRAME_ACK			0x1000
#define XMIT_STAT_READY_I_FRAME			0x0800
#define XMIT_STAT_FRAME_SIZE_ERR		0x0080
#define XMIT_STAT_TRANSMIT_THRESHHOLD	0x0040
#define XMIT_STAT_ODD_ADDRESS			0x0020
#define XMIT_STAT_FRAME_ERROR			0x0010
#define XMIT_STAT_ILLEGAL_FORMAT		0x0002

typedef struct srb_struc {
   ushort Offset;
   uchar Page_Select;
   uchar dummy;
} SRB;

#endif
