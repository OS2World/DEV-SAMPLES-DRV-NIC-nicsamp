/*
 * file: $Header:   \ibm16\misc.c  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file contains the panic function for runtime error checks. 
 *	   It also instantiates the global variable 'debug' if is is not  
 *	   defined by the C preprocessor.
 */
#include <stdio.h>
#include "devhdr.h"
#include "ndis.h"

/*
 * if 'debug' is not defined on the compile statement, then instantiate it as
 * a variable.
 */
#ifndef debug
	WORD debug = 0;
#endif

/*
 * assembly function the implements an 'INT 3' trap.
 */
extern void _brk(short);

/*
 * define an area into which the driver name is copied and NULL termiated.
 */
static char drvName[DEVNAME_LEN+1];

/*
 * Function: panic
 *
 * Inputs:
 *		code - a panic locater code that is printed with the panic message.
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 * 		This function provides panic breaks during the debug phase. The driver
 *		name is printed along with a locator 'code'. The locator code is
 *		intended to be used as a positional indicator in one's source code.
 *		These numbers should all be unique, as duplicates could be confusing.
 *		If the kernel debugger is enabled, an INT 3 will cause all program 
 *		execution to stop and the kernel debugger to become active. The 
 *		break point can be disabled by defining NO_PANIC on the assembly line
 *		for _misc.asm.
 */
void
panic(short code)
{
	short i;

	/*
	 * copy the driver name from the device driver header and NULL 
	 * terminate it.
	 */
	for (i=0; i<DEVNAME_LEN; i++)
		drvName[i] = DevHdr->SDevName[i];
	drvName[DEVNAME_LEN] = '\0';

	/*
	 * print the driver name and locator code.
	 */
	printf("!!! PANIC %d in %s !!!\n",code,drvName);

	/*
	 * call the break point function.
	 */
	_brk(code);
}

