/*
 * file: $Header:   \ibm16\mcainit.c  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file manages the generic PS/2 micro-channel initialization.
 *
 */
#include <stdio.h>
#include <conio.h>

#define	INCL_DOS
#define	INCL_DOSDEVICES

#include <os2.h>
#include "misc.h"
#include "hwinit.h"
#include "hw.h"
#include "strings.h"
#include "mca.h"

#ifndef microChannel
WORD microChannel;
#endif

/*
 * Function: mChanInit
 *
 * Inputs:
 *		none
 *
 * Outputs:
 * 		returns SUCCESS if an adapter was found that is not already in use.
 *
 * Side Effects:
 *
 *		This function gropes the micro-channel bus looking for an adapter
 *		that matches the POS ID values (POSIDLSB and POSIDMSB) supplied in
 *		HW.H. If an adapter is found and is is not already in use, then the
 *		posInfo table supplied in HWINIT.C is used to extract the POS 
 *		register values. Each posInfo entry supplies a set function, usually
 *		instantiated in HWINIT.C, that is called with the masked value of the
 *		POS register contents.
 */
WORD
mChanInit(void)
{
	/*
	 * if this is a micro-channel card, then go thru the PS/2 config
	 * process. all this really does is read config info from the adapter.
	 */
	if (microChannel) {
		ushort 	i;
		posType	*p;

		/*
		 * cycle thru the adapter slots that are loaded, looking for my
		 * adapter product ID.
		 */
		for (i=0; i<NUMCHAN; i++) {
			/*
			 * check to see if this slot is in use by selecting the channel,
			 * reading back the adapter product ID.
			 */
			outp(CHANPOS,i | 0x08);

			if (debug & DBG_VERBOSE)
				printf("mChanInit: checking slot %d\n",i);

			/*
			 * read the product ID from the POS register. if it matches, then
			 * see if the card has been enabled already. if it has, then look
			 * for another adapter since this driver has already been loaded.
			 */
			if (inp(POSIDREG_LSB)==POSIDLSB && inp(POSIDREG_MSB)==POSIDMSB &&
				!adapterInUse()) {
				break;
			}

			/*
			 * close the channel
			 */
			outp(CHANPOS,0);
		}

		/*
		 * if no adapter found, then return an error.
		 */
		if (i >= NUMCHAN) {
			return(!SUCCESS);
		}

		/*
		 * we have found an adapter that we recognize. now loop thru the
		 * option data structure reading config info, first disabling the
		 * adapter.
		 */
		outp(POSREG2, inp(POSREG2) & (~CARD_ENABLE));

		for (p=posInfo; p->posReg; p++) {
			uchar posVal;

			/*
			 * if posRegLsb is the sub-address extension register, then write
			 * the LSB and MSB to the sub-address extension registers and 
			 * read the byte from the register specified by 'dataReg'.
			 */
			if (p->posReg == SUBADDREXTLSB) {
				outp(SUBADDREXTLSB,p->subAddrIndexLsb);
				outp(SUBADDREXTMSB,p->subAddrIndexMsb);
				posVal = (uchar)inp(p->dataReg);
			}

			/*
			 * normal POS register read
			 */
			else {
				posVal = (uchar)inp(p->posReg);
			}
			
			/*
			 * extract the desired value from the POS register
			 */
			posVal =  (uchar)((posVal & p->mask) >> p->shift);

			/*
			 * store the value read to the address indicated after first
			 * masking and shifting. note that the value is always OR'ed 
			 * into the location indicated.
			 */
			if (p->var)
				*p->var |= posVal;

			/*
			 * if the set function exists, then call it
			 */
			if (p->setFunc && (*p->setFunc)(posVal) != SUCCESS)
				return(!SUCCESS);

			if (debug)
				printf(p->debugStr,(ushort)posVal);
		}

		/*
		 * clear the sub-address extension registers
		 */
		outp(SUBADDREXTLSB,0);
		outp(SUBADDREXTMSB,0);

		/*
		 * enable the adapter
		 */
		adapterEnable();
		outp(POSREG2, inp(POSREG2) | CARD_ENABLE);
	}

	/*
	 * close the POS register
	 */
	outp(CHANPOS,0);

	return(SUCCESS);
}
