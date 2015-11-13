/*
 * file: $Header:   \ibm16\hwinit.h  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file describes the structures used for load time parameter
 * 	parsing and hardware initialization.
 */
#ifndef HWINIT_H
#define HWINIT_H
#include "ndis.h"
#include "hw.h"

/*
 * this macro may be a variable if the driver code supports other buses as
 * well as micro-channel (EISA or ISA). in that event, it will be a global
 * byte somewhere that is set by system config code.
 */
#ifndef microChannel
extern WORD microChannel;
#endif

/*
 * define the 2 possible parameter types for use by driverParms[].
 */
#define NUMERIC_PARM	0
#define STRING_PARM	1

/*
 * string parameters can be of undefinite length. for purposes of compile
 * time init, the length is limited.
 */
#define STRING_LEN	32

/*
 * at load time, an array of this structure is supplied containing the 
 * hardware defaults as well as the function pointer to set the parameter.
 */
typedef struct {

	/*
	 * keyword == NULL terminates the array of parameter init structures.
	 */

	char		**keyword;

	/*
	 * 0 - numeric, 1 - string. see below.
	 */
	WORD		paramType;

	/*
	 * One of the following variables will be initialized to the parameter
	 * default value.
	 * paramType determines which of the following variables is to be used.
	 * if the type is 0, then numericParam is valid, else stringParam is
	 * valid. since this is a compile time initialized structure, a union
	 * cannot be used.
	 */
	long		numericParam;
	char		**stringParam;	/* Pointer to an ASCIIZ */

	/*
	 * each parameter has an associated setup function that must be called
	 * with either the valued parsed from the command line or .ini file or it
	 * musted be called with a pointer to the default value provided in this
	 * structure (numericParam or stringParam). all set functions return
	 * SUCCESS if successfull. assume error if otherwise.
	 */
	WORD (*setFcn)(void far *);

} parmInitType;

/*
 * usually instantiated in HWINIT.C
 */
extern parmInitType driverParms[];

/*
 * return 0 if the adpater is not in use, non-zero otherwise. this function is 
 * required by mcainit.c
 */
extern WORD adapterInUse(void);

/*
 * provide an adapter specific enable function.
 */
void adapterEnable(void);

#endif
