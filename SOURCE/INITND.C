/*
 * File: $Header:   \ibm16\initnd.c  2-2-93 jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 *	description:
 * 	This module provides NDIS specific initialization. This includes
 *		parsing the protocol.ini parameters and their values and registering
 *		with PROTMAN$.
 */

#include <stdio.h>

#define	INCL_DOS
#define	INCL_DOSDEVICES

#include <os2.h>
#include <fcntl.h>
#include "misc.h"            
#include "ndis.h"            
#include "strings.h"
#include "devhdr.h"          
#include "hw.h"              
#include "hwinit.h"
#include "buff.h"
#include "print.h"

/*
 * prototype the forward referenced functions used and defined in this
 * module.
 */
WORD		openDriver(char *devName);
WORD		openPM(void);
void far	*getPMI(void);
WORD		registerModule(void far *CCPtr);
struct KeywordEntry far *findKey(struct ModCfg far *MCp, char *Key);
BYTE far 	*getString(struct KeywordEntry far *KEp);
WORD		processParms(struct ModCfg far *MCp);


/*
 * Function: ndisInit
 *
 * Inputs:
 *		none
 *
 * Outputs:
 *		returns SUCCES if initialization was successful, !SUCCESS otherwise.
 *
 * Side Effects:
 *
 *		This function controls the generic NDIS intialization:
 *
 *			* Negotiate drivername (MAC$, MAC2$, ... MACn$).
 *			* Open the PROTMAN$.
 *			* GetProtocolManagerInfo.
 *			* Parse the Module configuration image to dispatch action routines 
 *			  per parameter.
 *			* RegisterModule (with protocol manager).
 */
int
ndisInit(void)
{
	struct ModCfg far *MCp;		/* Pointer into the Module Config table	*/	
	char far *dp;
	char *sp;
	char *np;
	int	i;

	/*
	 * Negotiate the drivername, if we can open on our drivername, we are
	 * already installed and must "increment" our driver number.
	 * (MAC$ -> MAC2$ -> MAC3$ -> ... MACn$)
	 * This will only work for MAX drivers = 10, this shouldn't be a problem
	 * for the next several years.
	 */

	/*
	 * start with adapter 1
	 */
	ndisGen.adapterNum = 1;

	/*
	 * try to find a name to be referenced by. If openDriver() succeeds, then
	 * a version of this driver is already loaded and we must find a different
	 * name.
	 */
	if (openDriver(drvrDevName) == SUCCESS) {

		/*
		 * get the alternate driver name and try to open it. The alternate
		 * name is in the form NAME2$. Every time openDriver() is successful,
		 * change the name to try to NAMEn$, where n is one more then the last
		 * number tried. 9 is the biggest number that works with this scheme.
		 */
		i = 2;
		drvrDevName = altDrvrDevName;
		np = (drvrDevName + (fStrlen((char far *)drvrDevName)-1)) - 1;
		while (++ndisGen.adapterNum && (openDriver(drvrDevName) == SUCCESS) &&
				(++i <= MAX_DRIVERS)) {
			ndisGen.adapterNum++;
			*np++;
		}

		/*
		 * bail out if there are too many drivers loaded.
		 */
		if (i > MAX_DRIVERS) {
			printf("%s %s %s\n", drvrDevName, errorPrefix, tooManyErr);
			return(!SUCCESS);
		}

	}

	/*
	 * Copy the drivername into the device header.
	 */
	sp = drvrDevName + DEVHDR_NAME_OFFSET;
	dp = (char far *)DevHdr->SDevName;
	while ((*dp++ = *sp++) != '$');	/* Don't copy the null, stop after "$"	*/

	/*
	 * Open the protocol manager.
	 */
	if (openPM() != SUCCESS) {
		printf("%s %s %s\n", drvrDevName, errorPrefix, notOpenPmErr);
		return(!SUCCESS);
	}

	/*
	 * get a pointer to the protocol manager memory image. thsi is a parsed
	 * image of PROTOCOL.INI.
	 */
	MCp = getPMI();

	/*
	 * Find our drivername in the Module config table.
	 */
	while (MCp != NULL &&
			fStrcmpi(getString(findKey(MCp, (char *)driverNameKW)),
					(drvrDevName + DEVHDR_NAME_OFFSET)) != 0) {

		MCp = MCp->NextModCfg;
	}
	if (MCp == NULL) {
		printf("%s %s %s\n", drvrDevName, errorPrefix, drvNotFndErr);
			return(!SUCCESS);
	}

	/*
	 * copy our module name into the Common Characteristics table from the 
	 * Mod config table.
	 */
	_bcopy((void far *)MCp->ModName, (void far *)MCC.CcName, NAME_LEN);

	/*
	 * Fixup pointer to AdapterDesc.
	 */
	MSC.MscVenAdaptDesc = (BYTE far *)FNDATA(AdapterDescr);

	/*
	 * get the buffer structures initialized.
	 */
	buffInit();

	/*
	 * Config time hook for hardware-specific initialization.
	 * Most hw specific init will be done at processParm-time.
	 * This hook is for "global" hw specific initialization. in particular,
	 * this function may be useful for micro-channel adapters where the POS
	 * registers need to be interogated. hwConfig() is provided by the
	 * project, usually in HW.C.
	 */
	if (hwConfig() != SUCCESS)
		return(!SUCCESS);

	/*
	 * Process the PROTOCOL.INI parameters.
	 */
	if (processParms(MCp) != SUCCESS) {

		/*
		 * processParms will print the error message
		 */
		return(!SUCCESS);
	}

	/*
	 * some adpaters need post processParms() processing. This useful when
	 * some of the PROTOCOL.INI parameters are interdependent and cannot be
	 * fully processed until after all of the values are known.
	 */
	if (hwPostConfig() != SUCCESS)
		return(!SUCCESS);

	/*
	 * Register the module with the protocol manager. use the fixed up
	 * back pointer in the MUD.
	 */
	if (registerModule((void far *)MUD.MudCCp) != SUCCESS) {
		printf("%s %s %s\n", drvrDevName, errorPrefix, regModRejectErr);
		return(!SUCCESS);
	}

	/*
	 * be sure to close PROTMAN$ before leaving.
	 */
	DosClose(ndisGen.PMHandle);

	return(SUCCESS);
}

/*
 * Function: openDriver
 *
 * Inputs:
 *		devName	- pointer to an ASCIIZ string that represents a possible 
 *				  driver name.
 *
 * Outputs:
 *		return 0 if the driver was opened, non-zero otherwise.
 *
 * Side Effects:
 *
 * 		Open the driver (all we care about is existence, we can ignore
 *		the Handle, Action, etc.)
 */
WORD
openDriver(char *devName)
{
	WORD devHandle;
	WORD action;
	WORD result;

	/*
	 * if the open succeeds, then close the handle and return the result.
	 */
	if (!(result=(WORD)DosOpen((char far *)devName,
						(unsigned far *)&devHandle,
						(unsigned far *)&action,
						SIZE_0,
						NORM_FILE,
						EXIST_ONLY,
						RW_DENY_NONE_PRIV,
						RESERVED_DW_0))) {
		DosClose(devHandle);
	}

	return(result);
}



/*
 * Function: openPM
 *
 * Inputs:
 *		none
 *
 * Outputs:
 *		return 0 if the protocol manager driver was opened, 
 *		non-zero otherwise.
 *
 * Side Effects:
 *
 * 		This function opens the protocol manager for business. Note that it
 *		remains open until closed at a later time.
 */
WORD
openPM(void)
{
	int stat;
	unsigned action;
		
#ifdef OS2
	stat = DosOpen((char far *)protMgrName, (unsigned far *)&ndisGen.PMHandle,
			(unsigned far *)&action, SIZE_0, NORM_FILE, EXIST_ONLY,
			WO_DENY_RW_PRIV, RESERVED_DW_0);
#else
	stat = DosOpen((char far *)protMgrName, (unsigned far *)&ndisGen.PMHandle,
			(unsigned far *)&action, SIZE_0, NORM_FILE, EXIST_ONLY,
			0xc2, RESERVED_DW_0);
#endif

	return (stat==SUCCESS ? SUCCESS : !SUCCESS);
}

/*
 * Function: callPM
 *
 * Inputs:
 *		RqB - far pointer to a GEN_IOCTL request block.
 *
 * Outputs:
 *		return 0 if the IOCTL was successful, 
 *		non-zero otherwise.
 *
 * Side Effects:
 *
 *		This function provides a GENIOCTL interface to the protocol manager.
 *		It is usually used for registering the module and getting a far
 *		pointer to the module config image in protocol manager memory space.
 *
 */
WORD
callPM(struct RqBlk far *RqB)
{
	WORD stat;

#ifdef OS2
	stat = DosDevIOCtl(NULL, (void far *)RqB, 0x58, 0x81, ndisGen.PMHandle);
#else
	stat = DosDevIOCtl(NULL, (void far *)RqB, sizeof(struct RqBlk), 0x81, 
						ndisGen.PMHandle);
#endif
	if (stat != SUCCESS) {
		return (stat);
	}
	return (RqB->Status);
}


/*
 * registerModule()
 *
 * Functional description:
 */

/*
 * Function: registerModule
 *
 * Inputs:
 *		CCPtr - far pointer to the NDIS Common Characteristics Table.
 *
 * Outputs:
 *		return 0 if the IOCTL was successful, 
 *		non-zero otherwise.
 *
 * Side Effects:
 *
 *		Registers the module with the Protocol manager. The protocol manager
 *		will give this pointer to the protocol stack at bind time. This is
 *		how the protocol stack knows how to call systemRequest() to initiate
 *		the bind process.
 *
 */
WORD
registerModule(void far *CCPtr)
{
	struct RqBlk ReqBlock;

	ReqBlock.Opcode = RegisterModule;
	ReqBlock.Pointer1 = CCPtr;

	/*
	 * Null layer under MAC
	 */
	ReqBlock.Pointer2 = NULL;

	return (callPM(&ReqBlock));
}

/*
 * Function: getPMI
 *
 * Inputs:
 *		none
 *
 * Outputs:
 * 		returns a far pointer to the module config image if successful, NULL
 *		otherwise.
 *
 * Side Effects:
 *
 *		This function generates a GENIOCTL to the protocol manager requesting
 *		a pointer to the module config image. The module config image is a
 *		memory based description of the parameters in PROTOCOL.INI.
 *
 */
void far *
getPMI()
{
	struct RqBlk ReqBlock;

	/*
	 * select the correct GENIOCTL op code.
	 */
	ReqBlock.Opcode = GetPMInfo;

	/*
	 * let callPM() do the messy work.
	 */
	if (callPM(&ReqBlock) == SUCCESS) {
		return(ReqBlock.Pointer1);
	}
	else {
		return(NULL);
	}
}

/*
 * Function: getPMI
 *
 * Inputs:
 *		MCp - far pointer to the module config image.
 *
 * Outputs:
 *		returns SUCCESS if all of the PROTOCOL.INI parameters were correctly
 *		processed, !SUCCESS otherwise.
 *
 * Side Effects:
 *
 *		This function manages processing of the PROTOCOL.INI parameters for
 *		this NDIS driver. After finding it's driver name in the mod config
 *		image, it tries to match the parameter name with a keyword in the
 *		driverParms table. Each table entry contains a default value for the
 *		parameter as well as an operator function. If the parameter has 
 *		been set in PROTOCOL.INI, that value is used in the call to the 
 *		operator function, otherwise the default value from the driverParms
 *		table is used in the call to the operator function.
 */
WORD
processParms(struct ModCfg far *MCp)
{
	parmInitType *parmp;
	struct KeywordEntry far *kwp;

	/*
	 * Use a pointer to walk through the driverParms table.
	 */
	parmp = driverParms;

	/*
	 * for every keyword defined in the driverParms table, try to find a match
	 * in the module config image. If there is a match, then use that value
	 * when invoking the operator function, otherwise use the default value
	 * provided in the driverParms table.
	 */
	while (parmp->keyword) {

		/*
		 * look for the keyword in the mod config image. If found, then use
		 * the PROTOCOL.INI value.
		 */
		if (kwp = findKey(MCp, (char *)(*parmp->keyword))) {

			/*
			 * if the value type parsed from PROTOCOL.INI does not match the 
			 * type defined in the driverParms table, then print an error 
			 * message and bail out.
			 */
			if (parmp->paramType != kwp->Params[0].ParamType) {
				printf("%s %s %s\n", errorPrefix, badParmTypeErr,
					  *(parmp->keyword));
				return(!SUCCESS);
			}

			/*
			 * call the operator function provided in the driverParms table
			 * with a pointer to the value parsed out of PROTOCOL.INI.
			 *
			 * see ndis.h for a
			 * justification of why it is done without a union.
			 *
			 * the operator function will return SUCCESS or !SUCCESS.
			 */
			if ((*parmp->setFcn)((void far *)(&kwp->Params[1])) !=
				SUCCESS) {
				return(!SUCCESS);
			}
		}
		
		/*
		 * the keyword could not be found in the mod config image, so set it
		 * using the default value from the driverParms table.
		 */
		else {

			/*
			 * pass a pointer to the parameter value. see ndis.h for a
			 * justification of why it is done without a union.
			 */
			/*
			 * numeric type
			 */
			if (parmp->paramType == NUMERIC_PARM) {
				if ((*parmp->setFcn)((void far *)&parmp->numericParam)
					!= SUCCESS){
					return(!SUCCESS);
				}
			}
			/*
			 * string type
			 */
			else {
				if ((*parmp->setFcn)((void far *)*parmp->stringParam)
					!= SUCCESS){
					return(!SUCCESS);
				}
			}

		}

		/*
		 * do the next driverParms entry.
		 */
		parmp++;
	}

	/*
	 * all operator functions returned SUCCESS.
	 */
	return(SUCCESS);
}

/*
 * Function: findKey
 *
 * Inputs:
 *		MCp - far pointer to the module config image.
 *		Key - pointer to an ASCIIZ string that represents the keyword in
 *			  question.
 *
 * Outputs:
 *		returns a far pointer to the keyword entry in the mod config image
 *		if found, NULL otherwise.
 *
 * Side Effects:
 *
 * 		This function searches the module config image (for this driver) for
 *		the keyword 'Key'. If found, it implies that it has been assigned a
 *		value in PROTOCOL.INI. If not found, it implies that the defualt 
 *		value in the driverParms table will be used.
 *
 */
struct KeywordEntry far *
findKey(struct ModCfg far *MCp, char *Key)
{
	struct KeywordEntry far *Kp;

	/*
	 * use a pointer to walk through the keyword entries.
	 */
	Kp = MCp->KE;

	/*
	 * search until a keyword is matched, or we run out of keyword entries for
	 * this driver.
	 */
	do {

		/*
		 * if the string matches, then return a far pointer to the keyword
		 * entry structure.
		 */
		if (fStrcmpi(Kp->KeyWord, Key) == 0) {
			return (Kp);
		}

		/*
		 * go to the next keyword entry
		 */
		Kp = Kp->NextKeywordEntry; 

	} while (Kp != NULL);

	/*
	 * the keyword was not found.
	 */
	return (NULL);			
}

/*
 * Function: getString
 *
 * Inputs:
 *		KEp - far pointer to a keyword entry.
 *
 * Outputs:
 *		returns a far pointer to the keyword entry parameter string value.
 *
 * Side Effects:
 *
 *		This function returns a far pointer to the string value parsed from 
 *		PROTOCOL.INI for this keyword.
 *
 */
BYTE far *
getString(struct KeywordEntry far *KEp)
{
	/*
	 * make this bullet proof. Sometimes the protocol manager (or something)
	 * passes incorrect mod config images.
	 */
	if (KEp != NULL) {
		if (KEp->NumParams >= 1 && KEp->Params[0].ParamType == 1) {
			return ((BYTE far *)(&KEp->Params[1]));
		}
	}

	/*
	 * don't return NULL, just return a pointer to an empty string. This will
	 * force an error at a higher level.
	 */
	return ((BYTE far *)"");
}

