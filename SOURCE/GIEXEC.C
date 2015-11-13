/*
 * File: $Header:   \ibm16\giexec.c  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * Description:
 *          Executor part of Generic Interface. 
 *          This and GI.INC form the main parts of the GI.
 */
#include <stdio.h>
#include <conio.h>
#include <memory.h>
#include "giexec.h"
#include "misc.h"
#include "ndis.h"
#include "hw.h"
#include "strings.h"
#include "print.h"
#include "hwbuff.h"

/*extern void Logit(); */
#define zap(n) inp(0x300 + n)	/* debugging macro for checkpointing with */
								/* a logic analyzer */
/* extern void		Fixed_Wait(ushort); */
extern ulong	_initTime(void);
extern ulong	_getTime(void);

/*extern void		dis_sif(void);
extern void		en_sif(ushort iobase,  ushort shmemOff, ushort shmemSeg,
						ushort irqline, ushort drqline); */
extern void		en_ticker(void);
extern void 	enable_int(void);
extern ushort	disable_int(void);
/*extern void		scb_request(ushort, void *);
extern void 	scb_lrequest(ushort, ulong); */

extern ushort	receivePacket(ushort size);
extern void		txComplete(ushort);
extern void		indicationsComplete(void);
extern void 	gReqComplete(ushort status);
extern ushort	ticks(void);
extern void		sysconfig(void);
/* extern void		_memset(void far *, uchar, ushort); */
extern ushort	swaps(ushort);
/* extern void		_movedata( void far *, void far *, ushort); */
extern void    FillDHB(short, struct TxBufDesc *);
void			gi_abortall(void);
int				bringup(void);
void			settpl(GCB *packet);
ushort			gi_init(void);
int				adap_init(void);
void			gi_shutdown(void);
void			open_adap(ushort options);
void			initTpl(void);
void srb_request(void);
ushort get_tx_request(void);
BYTE GetASB_ReturnCode(void);

ulong swapl(void far *), swaplb(ulong), aform(void *), xform(void *);

/* Null pointer definitions for near and far pointers */

#define FNULL ((char far *)0)

#define CONBUF_EXISTS 	8	/* bit of SSflag, also in GI.INC */
#define REAL_DMA 		1	/* ditto */
#define ATCARD 			2	/* bit of conbuf indicating p1344 card */

/* This macro will write the SIF Interrupt Register */

#define ir_write(n) if (debug & DBG_VERBOSE) printf("ir_write: 0x%x\n",(n));\
					outpw(giop.iobase + SIF_INT, n)
#define OUTW(a,v) outpw((a),(v));


/*
 * This stuff is for debugging purposes. This is not used with Netware.
 * The GI maintains a list of Ring Status Interrupts in the gilog.
 * It also reads counters as necessary and accumulates them in the 32 bit
 * counters stored in lcounters.
 */

#define GILOGLEN 128		/* Number of Ring Statuses we can save */
ushort gilog[GILOGLEN] = {0};	/* Log of Ring Statuses for Debugging */
ushort gilogcnt = 0;		/* Number of Ring Statuses saved in log */
WORD GlobalXmitFrameLength = 0;

extern WORD TAPFlag;
extern unsigned short SSflag;	/* system status flag from gi.inc */

extern ushort receivePending;   /* Flag defined in hw.asm		  */

/* This is the Adapter System Command Block defined in gi.asm. */
extern SCB gi_scb;

/* This is the Adapter System Status Block defined in gi.asm. */
extern SSB gi_ssb;

extern WORD SRB_Offset;
extern WORD SSB_Offset;
extern WORD ARB_Offset;
extern WORD ASB_Offset;
extern WORD Data_Rate;

extern WORD HW_Mchan;


/* The current state of the GI (and therefore the adapter). */
char gi_state = 0;
ushort SRB_Offset;
WORD Init_Flag = 0;
txBuffType *globaltxBuff;
ushort transmit_request = 0;

extern BYTE far *MMIOAddress;
extern BYTE far *RAMAddress;
extern WORD PIOBase;
extern WORD InitRAMAddr;
extern WORD InitMMIOAddr;

/***************************************************************************/
/*																									*/
/*						GI STATE TRANSITION DIAGRAM										*/
/*																									*/
/***************************************************************************/
/*																									*/
/*																									*/
/*	Initial------------->GIS_NOTHING<--+--------------------+					*/
/*					|		|			|															*/
/*	GIF_BRINGUP or >-------*		|			|		*/
/*	+--GIF_STARTUP			|		|			|		*/
/*	|					V		|			|		*/
/*	|				+->BUD Ok?	[n]--+			|		*/
/*	|				|	|[y]				|		*/
/*	|	GIF_BRINGUP >-----*	|				|		*/
/*	|				|	V				|		*/
/*	|				+--GIS_RESET<---+			|		*/
/*	|					|		|			|		*/
/*	|	GIF_INIT or >----------*		|			|		*/
/*	+--GIF_STARTUP			|		|			|		*/
/*	|					V		|			|		*/
/*	|				+->Init Ok? [n]-+			|		*/
/*	|				|	|[y]				|		*/
/*	|	GIF_INIT >--------*	|				|		*/
/*	|				|	V				|		*/
/*	|				+--GIS_CLOSED<--+----+--+		|		*/
/*	|					|		|	|	|		|		*/
/*	+--GIF_STARTUP >----------*		|	|	|		|		*/
/*					|		|	|	|		|		*/
/*					V		|	|	|		|		*/
/*				 Open Ok? [n]-+	|	|		|		*/
/*					|[y]			|	|		|		*/
/*					|			|	|		|		*/
/*					V			|	|		|		*/
/*				 GIS_OPEN			|	|		|		*/
/*					|			|	|		|		*/
/*					|			|	|		|		*/
/*					*-> GIF_CLOSE >-+	|		|		*/
/*					|			|		|		*/
/*					|			|		|		*/
/*					*-> Ring Status----+		|		*/
/*					|	(LWF, ARE, RmR)		|		*/
/*					|				|		*/
/*					*-> GIF_SHUTDOWN >---------------+		*/
/*							 |								|			*/
/*							 +------Adapter Check Interrupt---+			*/
/*										*/
/*										*/
/*										*/
/****************************************************************************/

/* The value of the last Ring Status Interrupt received (for debugging) */

ushort gi_status = 0;

/*
 * When Ring Status interrupts, transmit completions, receive completions,
 * etc. occur, the gi_ssb contains the information
 * from the adapter. The interrupt handler copies the gi_ssb into the
 * ssb_ring at the address pointed to by ssb_tail and then forks the executor.
 * When the executor processes the interrupt, it moves the ssb_head pointer
 * along the ring. When ssb_head and ssb_tail are equal, there are no more
 * interrupts in the ring to be processed.
 */

#define SSB_RING_LENGTH 20	/* An arbitrary number that seems to work */

SSB ssb_ring[SSB_RING_LENGTH] = {{0}};
SSB *ssb_head = ssb_ring;
SSB *ssb_tail = ssb_ring;
SSB *ssb_end = &ssb_ring[SSB_RING_LENGTH];	/* wrap address */

OPEN_PARAMS op = {0};		/* Adapter Open parameter block */

/*
 * Here are the transmit parameter lists and associated stuff.
 * The number of TPLs must be one more than the number of oustanding
 * transmits cached to the adapter.
 */

#define NUMTPLS (1 + MAXPENDING)

IOPL tpl[NUMTPLS] = {{0}};
IOPL *tplp = tpl;		/* This always points to the next free tpl */
IOPL *ftpl = tpl;		/* This points to the next Xmit to complete */
int transmits_pending = 0;	/* The number of outstanding adapter Xmits */
int max_pending = 0;		/* The most ever pending (for debug) */

/*
 * if a transmit error occurs that shuts down the transmitter, then set this 
 * flag so that settpl() will see it and act appropriately.
 */
ushort	txError=0;

/*
 * Transmits are performed at least partially contiguously in order to reduce
 * the number of DMA operations, which have proven to be expensive to the TI
 * Chip Set. Small fragments are copied into a transmit buffer. There are
 * the same number of transmit buffers as there can be transmits cached to the
 * adapter. Although we generally don't use the entire buffer, still Netware
 * makes no claims about the number or sizes of fragments and it is possible
 * that the entire packet will have to be copied so the buffers are full size.
 * The variable tbufselector defines which transmit buffer to use next.
 */

uchar tbuf[MAXPENDING][MAX_TX_PKT_SIZE] = {{0}};
uchar *LastTxBuf = tbuf[0];


ushort txgdts[MAXPENDING];

int tbufselector = 0;		/* This index indicates the next available tbuf */

/* This is the one and only receive parameter list. */

IOPL rpl = {0};

/* Here are the GI Open parameters. The giopmarker is for pro4conf. */

static char giopmarker[] = "GIOPM";
GIOP giop = {0xa20, 2, 0, 0xe000, 5};

/* This is a convenient holder of our data segment register value. */

ushort dsr = 0;

/*
 * Function: gi_exit( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function is called to unhook our interrupts and reset the state
 * of the GI.
 */

void
gi_exit(void)
{
	gi_state = GIS_NOTHING;
}

/*
 * Function: executor( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function is called when an SSB has been entered into the SSB ring.
 * The SSB is assumed to be pointed to by ssb_head, which is subsequently
 * incremented. All status interrupts from the adapter are handled by
 * this function. In essence, it is an extension of the interrupt service
 * routine. However, it is not called by the ISR recursively. This improves
 * performance since multiple interrupts are queued and dispatched serially
 * by the executor.
 */
void
executor(void)
{
	register ushort tmpw;
	GCB *xmt;
   BYTE tmpb;

top:

	/*
	 * the indication queue is empty
	 */
	if (ssb_head == ssb_tail) {
		return;
	}
	enable_int();
	tmpw = ssb_head->status[0];	 	/* Get the status word used most often */

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x0a, tmpw, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

	switch (ssb_head->command) {	/* Dispatch on command field */

		/* 
		 * Internal check interrupt means we got too many null interrupts.
		 * This is a "software-generated" adapter check, tmpw is not
		 * set.
	 	 */
		case SCB_INTCHECK:

			/* Store the internal check in the log if it's not full */
			if (gilogcnt < GILOGLEN)
				gilog[gilogcnt++] = 0xfffe;

		  	/* We don't do much about this - trash everything 	*/
			gi_exit();
			gi_abortall();

			/* We're closed, HW error.						   	*/	

			MSS.MssStatus &= ~(MS_HW_MASK | MS_OPEN);
			MSS.MssStatus |= HW_FAULT;
			adapterCheck(AdapCheckInoperative);
			break;

		/*
	 	 * Adapter check interrupts are coerced by the interrupt service
	 	 * routine into a form that looks like a regular SSB. The SSB command
	 	 * field is set to zero, which is not a legal SSB interrupt.
	 	 */
		case SCB_ADAPCHECK:

			/* Store the adapter check in the log if it's not full */

			if (gilogcnt < GILOGLEN)
				gilog[gilogcnt++] = 0xffff;

			/* We're closed, HW error.						   	*/	

			MSS.MssStatus &= ~(MS_HW_MASK | MS_OPEN);
			MSS.MssStatus |= HW_FAULT;

			gi_exit();
			gi_abortall();
			
			/* Pass reason up to the stack					*/
			adapterCheck(tmpw);
			break;

		case SCB_RINGSTATUS:

			/* Store the status bits in the log if it's not full */

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x11, tmpw, 0, 0);*/
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

			if (gilogcnt < GILOGLEN)
				gilog[gilogcnt++] = tmpw;

			/* Save the status bits - this is for debugging only */

			gi_status = tmpw;

			/* If the bits indicate the adapter has closed itself */

			if (tmpw & 0x0d00) {
				/* We're closed, HW error.						   	*/	

				MSS.MssStatus &= ~(MS_HW_MASK | MS_OPEN);
				MSS.MssStatus |= HW_FAULT;
				/* Then change the GI state and abort all requests */

				gi_state = GIS_CLOSED;
				gi_abortall();
			}
			ringStatus(tmpw);
			break;

 		/*
		 * Any command (other than Transmit or Receive) may end in
		 * Command Reject.
		 */
		case SCB_REJECT:

			if (debug & DBG_VERBOSE)
				printf("SCB_REJECT\n");

			gReqComplete(GENERAL_FAILURE);
			break;

		/*
	 	 * This case occurs on the completion of an Open request (GIF_STARTUP)
	 	 */

		case SCB_OPEN:

			/* If the open completed successfully */

			if (!tmpw) {

				gi_state = GIS_OPEN;	/* Note the state change */

				/* Inform caller */

				gReqComplete(SUCCESS);
			}
			else {	/* The Open failed */
				gReqComplete(HARDWARE_ERROR);
				gi_exit();
				gi_abortall();			/* trash any other pending requests */
			}
			break;

		case SCB_RECEIVE:
		case SCB_RCV_PENDING:

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x0b, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

			/* We have received a packet */

			/* Call the routine provided by the higher level to process this */

			if (receivePacket(ssb_head->status[1]) == SUCCESS) {

				/* Make sure the ASB is free */

            while (GetASB_ReturnCode() != 0xFF)
               ;

            (BYTE)*(RAMAddress + ASB_Offset) = SCB_RECEIVE;
            (BYTE)*(RAMAddress + ASB_Offset + 2) = 0;
            tmpb = (BYTE)((tmpw & 0xFF00) >> 8);
            (BYTE)*(RAMAddress + ASB_Offset + 6) = tmpb;
            tmpb = (BYTE)(tmpw & 0x00FF);
            (BYTE)*(RAMAddress + ASB_Offset + 7) = tmpb;

            tmpb = (BYTE)((ssb_head->status[3] & 0xFF00) >> 8);
            (BYTE)*(RAMAddress + ASB_Offset + 4) = tmpb;
            tmpb = (BYTE)(ssb_head->status[3] & 0x00FF);
            (BYTE)*(RAMAddress + ASB_Offset + 5) = tmpb;

            (BYTE)*(MMIOAddress + ISRAODD_SET) = 0x10;

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x0c, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */


			}
			else {
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x1c, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

				receivePending = 1;
			}
			break;

		case SCB_TRANSMIT: {
		
			ushort status = SUCCESS;
         WORD FrameLength = 0, i;

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x0d, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

         /*
          * If we are in TAP mode then this frame must be sent before
          * any tracing can be performed.
          */
         if (TAPFlag)
         {
            /*
             * Put trace parameters in the DHB.
             */
            (BYTE)*(RAMAddress + tmpw) = 0;
            (BYTE)*(RAMAddress + tmpw + 1) = 6;  /* No filtering */
            (BYTE)*(RAMAddress + tmpw + 2) = 0;
            (BYTE)*(RAMAddress + tmpw + 3) = 0;
            FrameLength = 124;
         }
         else
         {
            FrameLength += globaltxBuff->tx.TxImmedLen;
            for (i = 0; i < globaltxBuff->tx.TxDataCount; i++)
               FrameLength += globaltxBuff->tx.TxDataBlk[i].TxDataLen;
            FillDHB(tmpw, &globaltxBuff->tx);
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x0e, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

         }


			/* Make sure the ASB is free */

         while (GetASB_ReturnCode() != 0xFF)
            ;

         /*
          * Now set up the ASB.
          */

         (BYTE)*(RAMAddress + ASB_Offset) = 0x0a; /* xmit command */
         (BYTE)*(RAMAddress + ASB_Offset + 1) = (BYTE)ssb_head->status[1];
         (BYTE)*(RAMAddress + ASB_Offset + 2) = 0; /* return code */
         tmpb = ssb_head->status[2];
         (BYTE)*(RAMAddress + ASB_Offset + 5) = tmpb;
         tmpb = (ssb_head->status[2] >> 8);
         (BYTE)*(RAMAddress + ASB_Offset + 4) = tmpb;
         tmpb = FrameLength;
         (BYTE)*(RAMAddress + ASB_Offset + 7) = tmpb; /* frame length */
         tmpb = (FrameLength >> 8);
         (BYTE)*(RAMAddress + ASB_Offset + 6) = tmpb; /* frame length */
         (BYTE)*(RAMAddress + ASB_Offset + 8) = 0;
         (BYTE)*(RAMAddress + ASB_Offset + 9) = 0;

         /*
          * Finally, tell the adapter to transmit the data.
          * Then tell the adapter to interrupt us when the ASB is free.
          */

         (BYTE)*(MMIOAddress + ISRAODD_SET) = 0x10;
         GlobalXmitFrameLength = FrameLength;
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x0f, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

			break;
		}

		case SCB_CLOSE:

			/* Upper level has issued a close request which has completed. */

			if (gi_state == GIS_OPEN)
				gi_state = GIS_CLOSED;		/* Note the state change */
         /*
          * The offset of the SRB has changed.  Its value is in the WRBR
          * odd and even registers.
          */
         tmpb = (BYTE)*(MMIOAddress + WRBREVEN);
         SRB_Offset = (WORD)(tmpb << 8);
         tmpb = (BYTE)*(MMIOAddress + WRBRODD);
         SRB_Offset |= (WORD)tmpb;

			gReqComplete(SUCCESS);
			break;

		case SCB_SETGROUP:
		case SCB_SETFUNC:

			gReqComplete(SUCCESS);
			break;

		case SCB_READLOG:

   	   gReqComplete(SUCCESS);
			break;

      case SCB_PREOPEN:
         if (tmpw == SUCCESS)
      	   gReqComplete(SUCCESS);
         else
      	   gReqComplete(GENERAL_FAILURE);
         break;

      case SCB_TRANSMITDIR:
         if (tmpw != 0xFF)
         {
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0xe2, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

            gReqComplete(GENERAL_FAILURE);
         }
         break;

      case SCB_TRANSMITCOMPLETE:
         (BYTE)*(MMIOAddress + ISRAODD_SET) = 0x01;
         (BYTE)*(RAMAddress + SRB_Offset + 2) = 0;
         if (TAPFlag)
         {
            if (tmpw == 0)
         	   gReqComplete(SUCCESS);
            else
               gReqComplete(GENERAL_FAILURE);
         }
         else
         {
            if (tmpw == 0)
            {

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x1a, tmpw, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

               txComplete(SUCCESS);
               MSS.MssFS++;
					MSS.MssFSByt += (DWORD)GlobalXmitFrameLength;

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x1b, tmpw, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

            }
            else
            {
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0xe1, tmpw, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

               txComplete(TRANSMIT_ERROR);
            }
         }
         break;

		default:
			break;
		}

	/* Adjust the ssb ring pointer to the next slot */
	disable_int();

	if ((++ssb_head) == ssb_end)
		ssb_head = ssb_ring;

	goto top;
}


/*
 * Function: gi_bringup( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
ushort
gi_bringup(void)
{
	short temp;

	if (gi_state != GIS_NOTHING && gi_state != GIS_RESET)
		return (GIR_DISALLOWED);
	gi_exit();			/* Disconnect interrupt and DMA */
	if (temp = bringup()) {	/* Perform bringup - if error */
		return(GIR_EBRINGUP);
	}
	return (GIR_SUCCESS);	/* Bringup Ok */
}

/*
 * Function: gi_init( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
ushort
gi_init(void)
{
	short temp, i;

	if (gi_state == GIS_CLOSED)	 /* Flick state to reset */
		gi_state = GIS_RESET;		/* so next test won't fail */
	if (gi_state != GIS_RESET)		/* Init only from reset */
		return (GIR_DISALLOWED);

	/* Set the interrupt and DMA */

	if (debug & DBG_VERBOSE)
		printf("gi_init: ioBase 0x%x shmem 0x%x:%x irq %d dmaline %d\n",
			giop.iobase, giop.shoff, giop.shseg, giop.irqline, giop.dmaline);
   SRB_Offset = 0xffff;
   Init_Flag = 0;
	if (temp = adap_init()) {	/* Perform init - if error */
		if (debug & DBG_VERBOSE)
			printf("gi_init: adap_init failed\n");
		gi_exit();			/* Unhook interrupt and DMA */
		return (GIR_EINIT);
	}

/*
 * If we are in trace mode we must do a PRE.OPEN Command.
 */
   if (TAPFlag)
   {
      (BYTE)*(RAMAddress + SRB_Offset) = SCB_PREOPEN;
      for (i = 1; i < 26; i++)
         (BYTE)*(RAMAddress + SRB_Offset + i) = 0;
      if (Data_Rate == 16)
         (BYTE)*(RAMAddress + SRB_Offset + 26) = 1;
      else
         (BYTE)*(RAMAddress + SRB_Offset + 26) = 0;

      srb_request();
   	if (waitForGreqComplete() != SUCCESS)
         return (GIR_EINIT);
   }
	return (GIR_SUCCESS);	/* Init Ok */
}

/*
 * Function: gi_startup( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * gi_startup() - Do bringup and and init, but not open.
 */

ushort
gi_startup(void)
{
	ushort	status;

	status = GIR_NOTHING;

	if (gi_state == GIS_NOTHING) {	/* Need to do Bringup? */
		if (status = gi_bringup()) {
			if (debug & DBG_VERBOSE)
				printf("gi_startup: gi_bringup failed\n");
			return(status);
		}
	}

	if (gi_state == GIS_RESET)	 {	/* Need to do Init? */
		status = gi_init();
		if ((debug  & DBG_VERBOSE) && status)
			printf("gi_startup: gi_init failed\n");
	}
	return(status);
}

/*
 * Function: gi_open( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 *  Perform bringup, init, and adapter open.
 */
ushort
gi_open(ushort options)
{
	ushort temp;

	if (gi_state == GIS_NOTHING) {	/* Need to do Bringup? */
		if (temp = gi_bringup()) {
			return(temp);
		}
	}

	if (gi_state == GIS_RESET)	 {	/* Need to do Init? */
		if (temp = gi_init())	{
			return(temp);
		}
	}
	open_adap(options);			/* Open the adapter */
	return(SUCCESS);
}

/*
 * Function: gi_close( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
ushort
gi_close(void)
{
	/* Must be open */

	if (gi_state != GIS_OPEN)
	{
		return(GIR_DISALLOWED);
	}

   /*
    * copy Close command into SRB
    */
   (BYTE)*(RAMAddress + SRB_Offset) = SCB_CLOSE;

	srb_request();	/* Close the adapter */
	return(SUCCESS);
}

/*
 * Function: gi_setFunc( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
void
gi_setFunc(ulong funcAddr)
{
   (BYTE)*(RAMAddress + SRB_Offset) = SCB_SETFUNC;
   (BYTE)*(RAMAddress + SRB_Offset + 2) = 0;
   (BYTE)*(RAMAddress + SRB_Offset + 9) = (BYTE)(funcAddr >> 24);
   (BYTE)*(RAMAddress + SRB_Offset + 8) = (BYTE)(funcAddr >> 16);
   (BYTE)*(RAMAddress + SRB_Offset + 7) = (BYTE)(funcAddr >> 8);
   (BYTE)*(RAMAddress + SRB_Offset + 6) = (BYTE)(funcAddr);

	srb_request();
}

/*
 * Function: gi_setGroup( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
void
gi_setGroup(ulong groupAddr)
{
   /*
	scb_lrequest(SCB_SETGROUP, groupAddr);
*/
}


/*
 * Function: gi_shutdown( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
void
gi_shutdown()
{
	if (gi_state != GIS_NOTHING)	/* If not already shutdown */
	{
		gi_exit();			/* Disconnect interrupt and DMA */
		bringup();		/* Reset the board */
	}
	gi_abortall();			/* cancel all other requests */
}

/*
 * Function: gi_counters( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 */
void
gi_counters(uchar far *pointer)
{
	/*
	 * Read the counters - the pointer must refer to a
	 * 14 byte buffer to receive the long counters

/*    scb_lrequest(SCB_READLOG, (ulong)pointer);  */   /* Read update */
}

/*
 * Function: gi_transmit( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function is called to transmit a packet.
 * The passed parameter is a far pointer to a GCB.
 * The higher level NDIS code ensures that the serialization of transmit
 * requests.
 * Interrupts are disabled while this function executes.
 */

void
gi_transmit(txBuffType *packet)
{
	if (gi_state != GIS_OPEN) {		/* Must be open to transmit */
		txComplete(INVALID_FUNCTION);
		return;
	}

   while (transmit_request)
      transmit_request = get_tx_request();

/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */
/*         Logit(0x2d, 0, 0, 0); */
/*
 *DDDDDDDDDDDDDDDDDDDDDDDD
 */

   /*
    * stash or transmit buffer descriptor and flag that we are attempting
    * a transmit.
    */
   globaltxBuff = packet;
   transmit_request = 1;

   /*
    * Tell the adapter we want to request a transmit.
    */
   (BYTE)*(RAMAddress + SRB_Offset) = SCB_TRANSMITDIR;
   (BYTE)*(RAMAddress + SRB_Offset + 1) = 0;
   (BYTE)*(RAMAddress + SRB_Offset + 4) = 0;
   (BYTE)*(RAMAddress + SRB_Offset + 5) = 0;

   srb_request();
}

ushort get_tx_request(void)
{
   return (transmit_request);
}

/*
 * Function: bringup( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function performs the Bringup Diagnostics. It will wait here for
 * completion. The value returned is:
 *	-1	if no valid response recognized within 3 seconds
 *	0	if successful
 * > 0	if a bringup error was indicated (1 more than the error code)
 */

int
bringup()
{
	int status;

			gi_state = GIS_RESET;	/* Change state */
			status = GIR_SUCCESS; 		/* Ok */

	return (status);
}

/*
 * Function: adap_init( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function performs the Adapter Initialization. It will wait here for
 * completion. The value returned is:
 *	-1	if no valid response recognized within 15 seconds
 *	0	if successful
 * > 0	if an initialization error was indicated
 */
WORD initAddr = 0x200;

int
adap_init()
{
	int status, i;
	ulong t1;
   BYTE far *tmpaddr;
   BYTE far *tmpaddr2;
   ulong tmpphys, tmpphys2;
   BYTE regval;
   WORD Dummy, addroffset, tmpinit, *tmpptr;


   tmpphys = (ulong)InitMMIOAddr << 4;
   tmpaddr = (BYTE far *)physToUVirt(tmpphys, 8192L, 1);

   tmpphys2 = (ulong)InitRAMAddr << 4;
   tmpaddr2 = (BYTE far *)physToUVirt(tmpphys2, 65535L, 1);


   (BYTE)*(tmpaddr + ISRPEVEN_RESET) = 0xBF;


   regval = (BYTE)*(tmpaddr + RRRODD);
   if (TAPFlag)
   {
      regval |= 0x01;
      (BYTE)*(tmpaddr + RRRODD) = regval;
   }
   else
   {
      regval &= 0xfe;
      (BYTE)*(tmpaddr + RRRODD) = regval;
   }

   /*
    * The adapter is reset by writing any value to the Adapter Reset
    * Latch I/O port, waiting at least 50ms and then writing any value
    * to the Adapter Reset Release I/O port.
    */

   outp((PIOBase + RESETLATCH), 0x1);
	t1 = _initTime();					/* Get start time */
   while ((_getTime() - t1) < 80L)
      ;
   outp((PIOBase + RESETRELEASE), 0x1);

   /*
    * If this is an AT bus machine, we must set the RRREVEN register
    * to the shared RAM address.
    */
   if (!HW_Mchan)
   {
      regval = (BYTE)(InitRAMAddr >> 8);
      (BYTE)*(tmpaddr+RRREVEN) = regval;
   }


   /*
    * Set the SRPR register.
    */
   (BYTE)*(tmpaddr+SRPREVEN) = 0x00;

   /*
    * Set interrupt enable bit in in ISRP
    */
   (BYTE)*(tmpaddr + ISRPEVEN_SET) = 0xC0;

   /* Delay 4 seconds */
   while (1)
   {
      tmpptr = &Init_Flag;
      if (*tmpptr)
         break;
   }
	status = -1;		/* No Status yet */

   /*
    * Get offset of SRB in shared RAM. Change to a different page
    * if necessary. Determine the page by looking at the high nibble
    * and dividing by four.  
    */
   regval = (BYTE)*(tmpaddr + WRBREVEN);
   SRB_Offset = (WORD)(regval << 8);
   regval = (BYTE)*(tmpaddr + WRBRODD);
   SRB_Offset |= (WORD)regval;

   /*
    * Wait for diagnostics to complete.
    */
	while (status == -1)	{	/* While no status */
      Dummy = 0xFFFF; /* Fake out the compiler */
		if (((BYTE)*(tmpaddr2 + SRB_Offset)) == 0x80)
      {

		/* If the bits look right for success */

   		if (((BYTE)*(tmpaddr2 + SRB_Offset + 6)) == 0x00)	{
	   		gi_state = GIS_CLOSED;	/* Change state */
		   	status = GIR_SUCCESS; 		/* Ok */
   			if (debug & DBG_VERBOSE)
   				printf("adap_init: SUCCESS\n");
   		}			
   		else  { /* Is there an error */
   			status = (int)((BYTE)*(tmpaddr2 + SRB_Offset + 6));	/* Extract error code */
   			if (debug & DBG_VERBOSE)
   				printf("adap_init: IR_ERROR SIF_INT 0x%x\n",status);
   		}
      }
		else if (_getTime() - t1 >= 15000L) {		/* 15 seconds gone by? */
         SRB_Offset &= Dummy; /* This does nothing, used to trick the compiler */
			if (debug & DBG_VERBOSE)
		 		printf("adap_init: timeout\n");
			break;				/* Give up */
		}
	}
	if ((debug & DBG_VERBOSE) && status)
   	printf("adap_init: failure status 0x%x\n",status);

   /*
    * The top 512 bytes of shared RAM must be intialized to 0.

   if (status == GIR_SUCCESS)
   {
      addroffset = 0xfe00;
      for (i = 0; i < 512; i++)
         (BYTE)*(tmpaddr2 + addroffset++) = 0x00;
   }
    */

   physToUVirt((ulong)tmpaddr, 0L, 2);
   physToUVirt((ulong)tmpaddr2, 0L, 2);

	return (status);
}

/*
 * Function: open_adap( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function will initiate the Open process. No return status is given.
 */

void
open_adap(ushort options)
{
	ulong  t1;
   OPEN_PARAMS *open_ptr;
   int i;

	/* Initialize the SSB ring pointers */

	ssb_head = ssb_tail = ssb_ring;
	/*
	 * Wait half a second to avoid back to back open requests 
	 */
	t1 = _initTime() + 500L;
	while (_getTime() < t1)	
		;

	/* NOTE: All word operands are byte swapped !! */

	/*
	 * Set options from protocol stack. Assume that if promiscuous mode is
	 * set, then one should copy all frames that can be copied.
	 */
	if (options & COPY_ALL_NON_MAC_FRAMES)
		op.options |= (PASS_ADAPTER_MAC_FRAMES|PASS_ATTENTION_MAC_FRAMES|
                                             PASS_BEACON_MAC_FRAMES);

   if (TAPFlag)
      op.options |= TAP_TRACEMODE;
	/*
    * Copy open options to SRB
    */
   open_ptr = &op;
   for (i = 0; i < sizeof(OPEN_PARAMS) - 1; i++)
      (BYTE)*(RAMAddress + SRB_Offset + i) = (BYTE)*((BYTE *)open_ptr + i);

	/* Start the open sequence */

	srb_request();
}

/*
 * Function: gi_abortall( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function aborts outstanding transmit requests.
 */

void
gi_abortall(void)
{

}

/*
 * Function: srb_request( )
 *
 * Inputs: None.
 *
 * Outputs: None.
 *
 * Side Effects:
 *
 * This function sets bit 5 of the ISRA odd register informing
 * the adapter that we have put a command in the SRB.
 * Assumes the SRB has been set up properly!
 */
void srb_request(void)
{
   /*
    * Make sure SRB is free
    */
   while ((BYTE)*(RAMAddress + SRB_Offset + 2) & 0xFF)
      ;
   (BYTE)*(MMIOAddress + ISRAODD) |= 0x20;
}

BYTE GetASB_ReturnCode(void)
{
   return ((BYTE)*(RAMAddress + ASB_Offset + 2));
}
