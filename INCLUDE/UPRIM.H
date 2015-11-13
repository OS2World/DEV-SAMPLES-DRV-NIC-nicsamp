/*
 * file: $Header:   \ibm16\uprim.h  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *		This file describes the data structures that the MAC upper dispatch
 *		table C function primitives receive from their corresponding ASM
 *		interface functions. The ASM interface functions provide a far pointer
 *		to a stack based structure.
 */
#ifndef UPRIM_H
#define UPRIM_H
#include "ndis.h"

/*
 * Struct: xmitChainType
 *
 * This is the TransmitChain structure that is assembled on the stack 
 * by the upper layer (protocol stack). TRANSMITCHAIN then forms a far 
 * pointer to this structure and passes it to transmitChain(). See the
 * function definition for TransmitChain in the NDIS Spec.
 */
typedef struct {
	WORD					macDs;
	struct TxBufDesc far	*txDesc;
	WORD					reqHandle;
	WORD					protID;
} xmitChainType;

/*
 * Struct: rcvReleaseType
 *
 * ReceiveRelease structure that is assembled on the stack by the upper
 * layer (protocol stack). See the function definition
 * for ReceiveRelese in the NDIS Spec.
 */
typedef struct {
	WORD					macDs;
	WORD					reqHandle;
} rcvReleaseType;

/*
 * Struct: genRequestType
 *
 * GeneralRequests structure that is assembled on the stack by the upper
 * layer (protocol stack). GENERALREQUESTS generates a pointer to this 
 * information on the stack and calls generalRequests().
 *
 * See the function definition
 * for GeneralRequests in the NDIS Spec.
 */
typedef struct {
	WORD					macDs;
	WORD					opCode;
	DWORD					param2;
	WORD					param1;
	WORD					reqHandle;
	WORD					protID;
} genRequestType;

/*
 * Struct: sysRequestType
 *
 * SystemRequest structure that is assembled on the stack by the upper
 * layer (protocol stack). SYSTEMREQUEST forms a far pointer to this 
 * information and calls systemRequest() with a far pointer to it.
 * 
 * See the function definition
 * for SystemRequest in the NDIS Spec.
 */
typedef struct {
	WORD					targetDs;
	WORD					opCode;
	WORD					param3;
	DWORD	far				*tabAddr;
	struct CommChar	far		*charTab;
} sysRequestType;

/*
 ****************************************************************************
 *
 * the following describe incoming upper primitive data
 * structures. Each structure definition corresponds to a lower or upper
 * dispatch table primitive.
 *
 ****************************************************************************
 */

/*
 * Struct: xferDataType
 *
 * transfer data structure that transferData() expects. see NDIS Spec for
 * further details.
 */
typedef struct {
	WORD					macDs;
	struct TDBufDesc far	*tdDesc;
	WORD					frameOffset;
	WORD far				*bytesCopied;
} xferDataType;

/*
 * Struct: errLogType
 *
 * Cast the genRequestType structure to be something specific to the 
 * Read Error Log general request. See the NDIS Spec for further details.
 */
typedef struct {
	WORD					macDs;
	WORD					opCode;
	LPBUF					logAddr;
	WORD					logLen;
	WORD					reqHandle;
	WORD					protID;
} errLogType;

/*
 * Struct: statAddrType
 *
 * Cast the genRequestType structure to be something specific to the 
 * Set Station Address general request.
 */
typedef struct {
	WORD					macDs;
	WORD					opCode;
	LPBUF					statAddr;
	WORD					param1;
	WORD					reqHandle;
	WORD					protID;
} statAddrType;

/*
 * Struct: openAdapType
 *
 * Cast the genRequestType structure to be something specific to the 
 * Open Adapter  general request.
 */
typedef struct {
	WORD					macDs;
	WORD					opCode;
	DWORD					param2;
	WORD					openOptions;
	WORD					reqHandle;
	WORD					protID;
} openAdapType;

/*
 * Struct: setFilterType
 *
 * Cast the genRequestType structure to be something specific to the 
 * Set Filter general request.
 */
typedef struct {
	WORD					macDs;
	WORD					opCode;
	DWORD					param2;
	WORD					filterMask;
	WORD					reqHandle;
	WORD					protID;
} setFilterType;

/*
 * Struct: addMultiType
 *
 * Cast the genRequestType structure to be something specific to the 
 * Add Multicast Address general request.
 */
typedef struct {
	WORD					macDs;
	WORD					opCode;
	LPBUF					multiAddr;
	WORD					param1;
	WORD					reqHandle;
	WORD					protID;
} addMultiType;

/*
 * Struct: setFuncAddrType
 *
 * Cast the genRequestType structure to be something specific to the 
 * Set Functional Address general request.
 */
typedef struct {
	WORD					macDs;
	WORD					opCode;
	LPBUF					funcAddr;
	WORD					param1;
	WORD					reqHandle;
	WORD					protID;
} setFuncAddrType;

/*
 * Struct: setLookaheadType
 *
 * Cast the genRequestType structure to be something specific to the 
 * Set Look Ahead general request.
 */
typedef struct {
	WORD					macDs;
	WORD					opCode;
	DWORD					param2;
	WORD					lookAheadLength;
	WORD					reqHandle;
	WORD					protID;
} setLookaheadType;

/*
 * prototype the general request functions
 */
WORD receiveRelease( rcvReleaseType far *p );
WORD initiateDiagnostics( genRequestType far *p );
WORD readErrorLog( errLogType far *p );
WORD setStationAddress( statAddrType far *p );
WORD openAdapter( openAdapType far *p );
WORD closeAdapter( genRequestType far *p );
WORD resetMAC( genRequestType far *p );
WORD setPacketFilter( setFilterType far *p );
WORD addMulticastAddress( addMultiType far *p );
WORD deleteMulticastAddress( addMultiType far *p );
WORD updateStatistics( genRequestType far *p );
WORD clearStatistics( genRequestType far *p );
WORD interruptProtocol( genRequestType far *p );
WORD setFuncAddress( setFuncAddrType far *p );
WORD setLookahead( setLookaheadType far *p );

/*
 * prototype txComplete() for UPRIM.C.
 */
void txComplete(WORD /* ndisStatus */ );

/*
 * hwXmit() must be provided by the project as an adapter specific function.
 * The prototype is provided here to enforce the linkage from the generic
 * model into the project specific code. hwXmit() is usually instantiated 
 * in HW.C.
 */
void hwXmit(void);

#endif

