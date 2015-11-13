; 
; file: $Header:   \ibm16\ndis.inc  2-2-93  jess fahland $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;
;	Definitions and structures related to the Network Driver Interface 
;	Specification (NDIS) generic model. This version conforms to the
;	NDIS Spec rev 2.0.1.
;
;	Names of structures, types, tables, and functions follow the 
;	NDIS naming conventions, whenever practical.
;
;
byteSize struc
aByte db ?
byteSize ends
wordSize struc
aWord dw ?
wordSize ends
dwordSize struc
aDword dd ?
dwordSize ends
  
; 
; * prevent multiple inclusions
; 					
;ifndef NDIS_H
NDIS_H		equ 0

; * Type definitions used throughout. These match the nomenclature in the NDIS
; * Spec.
; 

; 
; * NDIS drivers cannot have legal NULL pointers. Hence, the use of 0 as a
; * NULL pointer value.
; 
;ifndef NULL
NULL		equ 0
;endif

; 
; * define the length of names and addresses in the Common Characteristics
; * and Specific Characteristics table.
; 
NAME_LEN		equ 16
ADDR_LEN		equ 16
ADDR_SIZE		equ 6

; 
; * maximum number of multicast addresses. This should probably be defined in
; * HW.H since it is more driver specific.
; 
NUM_MCADDRS		equ 4

; 
; * NDIS System request opcodes supported by the generic model.
; 
InitiateBind		equ 1
Bind		equ 2

; 
; * Protocol Manager opcodes used by the generic model
; 
GetPMInfo		equ 1
RegisterModule		equ 2
BindAndStart		equ 3
GetPMLinkage		equ 4

; 
; * NDIS General Request opcodes supported by the generic model
; 
InitiateDiagnostics		equ 1
ReadErrorLog		equ 2
SetStationAddress		equ 3
OpenAdapter		equ 4
CloseAdapter		equ 5
ResetMAC		equ 6
SetPacketFilter		equ 7
AddMulticastAddress		equ 8
DeleteMulticastAddress		equ 9
UpdateStatistics		equ 10
ClearStatistics		equ 11
Interrupt		equ 12
SetFunctionalAddress		equ 13
SetLookAhead		equ 14

; 
; * NDIS Status Indication opcodes
; 
RingStatus		equ 1
AdapterCheck		equ 2
StartReset		equ 3
InterruptStatus		equ 4
EndReset		equ 5

; 
; * All possible return error codes that are defined by the NDIS Spec. 2.0.1
; 
SUCCESS		equ 0000H
WAIT_FOR_RELEASE		equ 0001H
REQUEST_QUEUED		equ 0002H
FRAME_NOT_RECOGNIZED		equ 0003H
FRAME_REJECTED		equ 0004H
FORWARD_FRAME		equ 0005H
OUT_OF_RESOURCE		equ 0006H
INVALID_PARAMETER		equ 0007H
INVALID_FUNCTION		equ 0008H
NOT_SUPPORTED		equ 0009H
HARDWARE_ERROR		equ 000AH
TRANSMIT_ERROR		equ 000BH
NO_SUCH_DESTINATION		equ 000CH
ALREADY_STARTED		equ 0020H
INCOMPLETE_BINDING		equ 0021H
DRIVER_NOT_INITIALIZED		equ 0022H
HARDWARE_NOT_FOUND		equ 0023H
HARDWARE_FAILURE		equ 0024H
CONFIGURATION_FAILURE		equ 0025H
INTERRUPT_CONFLICT		equ 0026H
INCOMPATIBLE_MAC		equ 0027H
INITIALIZATION_FAILED		equ 0028H
GENERAL_FAILURE		equ 00FFH

; 
; * NDIS Adapter Check error codes
; 
AdapCheckInoperative		equ 8000H
AdapCheckIllegalOp		equ 1000H
AdapCheckLocalParityErr		equ 0800H
AdapCheckParityErr		equ 0400H
AdapCheckInternalParityErr		equ 0100H
AdapCheckRingXmitParityErr		equ 0080H
AdapCheckRingRcvParityErr		equ 0040H
AdapCheckXmitOverrun		equ 0020H
AdapCheckRcvOverrun		equ 0010H
AdapCheckUnrecogInterrupt		equ 0008H
AdapCheckUnrecogErrInt		equ 0004H
AdapCheckNoPCSysService		equ 0003H
AdapCheckUnrecogSuperReq		equ 0002H
AdapCheckProgramRequest		equ 0001H

; 
; * NDIS protocol levels, interface type
; 
MACLvl		equ 1
MACTyp		equ 1

; 
; * NDIS Protocol Table structure definitions
; 

; 
; * NDIS Common Characteristics table
; 
CommChar struc
	CcSize dw ?				;  Table size							
	CcLvl dw ?			 	;  Level of table						
	CcSpLvl dw ?			;  Level of spec tables			 		
	CcMjrVer db ?		 	;  Module Major version number			
	CcMnrVer db ?		 	;  Module Minor version number			
	CcBindFnc dd ?			;  "Hints" on binding support			
	CcName db size byteSize*NAME_LEN dup(0)	;  Module name							
	CcUPLevel db ?		 	;  Upper protocol level			 		
	CcUIType db ?			;  Upper interface type			 		
	CcLPLevel db ?		 	;  Lower protocol level			 		
	CcLIType db ?			;  Lower interface type			 		
	CcModuleID dw ?			;  Module ID (supplied by PM)			
	CcDataSeg dw ?		 	;  Module Data Segment					

	; 
;	 * System request function pointer
;	 
	CcSysReq dd ?

	; 
;	 * links to other NDIS characteristics tables.
;	 
	CcSCp dd ?	;  specific characteristics 		
	CcSSp dd ?	;  specific status					
	CcUDp dd ?	;  upper dispatch table	 			
	CcLDp dd ?	;  lower dispatch table	 			

	CcRsv1 dd ?				;  reserved 
	CcRsv2 dd ?				;  reserved 
CommChar ends

; 
; * Definintions for the MAC Service Supported Flags.
; * Bits 0-14.
; 
BROADCAST_SUPP		equ (DWORD)WORD)0x00000001H
MULTICAST_SUPP		equ (DWORD)WORD)0x00000002H
FUNC_GROUP_ADDR_SUPP		equ (DWORD)WORD)0x00000004H
PROMISCUOUS_SUPP		equ (DWORD)WORD)0x00000008H
SET_STN_ADDR_SUPP		equ (DWORD)WORD)0x00000010H
STAT_ALWAYS_CURR		equ (DWORD)WORD)0x00000020H
DIAGNOSTICS_SUPP		equ (DWORD)WORD)0x00000040H
LOOPBACK_SUPP		equ (DWORD)WORD)0x00000080H
RECEIVECHAIN_MOSTLY		equ (DWORD)WORD)0x00000100H
IBM_SOURCE_R_SUPP		equ (DWORD)WORD)0x00000200H
RESET_MAC_SUPP		equ (DWORD)WORD)0x00000400H
OPEN_ADAPTER_SUPP		equ (DWORD)WORD)0x00000800H
INTERRUPT_RQ_SUPP		equ (DWORD)WORD)0x00001000H
SRC_RTNG_BRDG_SUPP		equ (DWORD)WORD)0x00002000H
GDT_ADDRESS_SUPP		equ (DWORD)WORD)0x00004000H
MULTIPLE_XFER_DATA_SUPP		equ (DWORD)WORD)0x00008000H
RCVLK_FRAME_SIZE_ZERO		equ (DWORD)WORD)0x00010000H

; 
; * MAC Service Specific characteristics table
; 
MACSpecChar struc
	MscSize dw ?					;  Table size						
	MscType db size byteSize*NAME_LEN dup(0) 			;  MAC type name					
	MscStnAdrSz dw ?				;  Station address length			
	MscPermStnAdr db size byteSize*ADDR_LEN dup(0)	;  Permanent station address		
	MscCurrStnAdr db size byteSize*ADDR_LEN dup(0)	;  Current station address			
	MscCurrFncAdr dd ?				;  Current functional address		

	MscMCp dd ?		;  Address of multicast buffer		

	MscLinkSpd dd ?					;  Link speed (bits/sec)			
	MscService dd ? 				;  Services supported flags			
	MscMaxFrame dw ?				;  Maximum frame size				
	MscTBufCap dd ?					;  Transmit buffer capacity			
	MscTBlkSz dw ?		 			;  Transmit buf alloc block size	
	MscRBufCap dd ?					;  Receive buffer capacity			
	MscRBlkSz dw ?		 			;  Receive buf alloc block size		
	MscVenCode db size byteSize*3 dup(0)	 			;  Vendor code						
	MscVenAdapter db ?	 			;  Vendor adapter code				
	MscVenAdaptDesc dd ?			;  Ptr to vendor adapter desc		
	MscInterrupt dw ?				;  Interrupt level used				
	MscTxQDepth dw ?				;  Transmit Queue Depth				
	MscMaxDataBlocks dw ?			;  Max buff descriptor data blocks 	
MACSpecChar ends

; 
; * multicast address structure is ADDR_LEN bytes long
; 
MCastAddr struc
	mAddr db size byteSize*ADDR_LEN dup(0)
MCastAddr ends

; 
; * the multicast address buffer manages NUM_MCADDRS multicast address
; * structures.
; 
MCastBuf struc
	McbMax dw ?				;  max # of multicast addresses				
	McbCnt dw ?				;  curr# "		"			"			

	McbAddrs db size MCastAddr*NUM_MCADDRS dup(0)
MCastBuf ends

; 
; * MAC service-specific status table
; 
MACSpecStat struc
	MssSize dw ?			;  Table size							
	MssDiagDT dd ?			;  Last diagnostic Date/Time			
	MssStatus dd ?			;  MAC status - bit mask				 
	MssFilter dw ?			;  Current packet filter				
	MssM8Sp dd ?			;  pointer to Media specific status		
	MssClearDT dd ?			;  Last clear stats Date/Time			
	MssFR dd ?				;  Frames received: total				
	MssRFCRC dd ?			;  Receive fail: CRC error				
	MssFRByt dd ?			;  Frames received: total bytes			
	MssRFLack dd ?			;  Receive fail: lack of buffers		
	MssFRMC dd ?			;  Frames received: multicast			
	MssFRBC dd ?			;  Frames received: broadcast			
	MssRFErr dd ?			;  rcv fail: errors in general			
	MssRFMax dd ?			;  rcv fail: exceeds max size			
	MssRFMin dd ?			;  rcv fail: less than min size			
	MssFRMCByt dd ?			;  Frames rcvd: multicast bytes			
	MssFRBCByt dd ?			;  Frames rcvd: broadcast bytes			
	MssRFHW dd ?			;  rcv fail: hardware error				
	MssFS dd ?				;  Frames sent: total					
	MssFSByt dd ?			;  Frames sent: total bytes				
	MssFSMC dd ?			;  Frames sent: multicast				
	MssFSBC dd ?			;  Frames sent: broadcast				
	MssFSBCByt dd ?			;  Frames sent: broadcast bytes			
	MssFSMCByt dd ?			;  Frames sent: multicast bytes			
	MssSFTime dd ?			;  Send fail: time-out					
	MssSFHW dd ?			;  Send fail: hardware error			
MACSpecStat ends

; 
; * Packet filter bit definitions.
; 
FLTR_DIRECTED		equ 01H
FLTR_BRDCST		equ 02H
FLTR_PRMSCS		equ 04H
FLTR_SRC_RTG		equ 08H

; 
; * MAC status bits in the MAC Specific Characteristics table.
; * Bits 0-2 are coded to reflect the hardware status.
; 
HW_MISSING		equ 0000H
HW_BUD_ERR		equ 0001H
HW_CFG_ERR		equ 0002H
HW_FAULT		equ 0003H
HW_SOFT_ERR		equ 0004H
HW_OK		equ 0007H
MS_HW_MASK		equ HW_OK

MS_BOUND		equ 0008H
MS_OPEN		equ 0010H
MS_DIAGS_ON		equ 0020H


; 
; * 802.3 (Token Ring) status table
; 
MAC8023Stat struc
	M83sSize dw ?			;  Table size							
	M83sVer dw ?			;  Version								
	M83sRFAln dd ?			;  Receive fail: Alignment error		
	M83sRMask dd ?			;  Receive fail bit mask (below)		
	M83sRFOvrn dd ?			;  Receive fail: Overrun				
	M83sFSCols dd ?			;  Frames sent: after collisions		
	M83sFSDfr dd ?			;  Frames sent: after deferring			
	M83sSFColMx dd ? 		;  Frames not sent: Max collisions		
	M83sTotCol dd ?			;  Total collision during tran attempts	
	M83sTotLCol dd ? 		;  Total late collisions				
	M83sFSCol1 dd ?		 	;  Frames sent: after 1 collision		
	M83sFSColM dd ?			;  Frames sent: multpl collisions		
	M83sFSHrtB dd ?		 	;  Frames sent: CD heart beat			
	M83sJabber dd ?			;  Jabber errors						
	M83sLostCS dd ?			;  Lost carrier sense during tran		
	M83sTMask dd ?			;  Transmit fail bit mask (below)		
MAC8023Stat ends

; 
; * Receive error failure codes.
; 
M83Sb_RFCRC		equ 1
M83Sb_RFFE		equ 2
M83Sb_RFMaxSz		equ 4

; 
; * Transmit error failure codes.
; 
M83Sb_TFColMx		equ 1
M83Sb_TFCrr		equ 2
M83Sb_TFShrt		equ 4
M83Sb_TFOpen		equ 8
M83Sb_TFLong		equ 16
M83Sb_TFRemFl		equ 32



; 
; * 802.5 (Ethernet) status table
; 
MAC8025Stat struc
	M85sSize dw ?			;  Table size							
	M85sVer dw ?			;  Version								
	M85sFCS dd ?			;  FCS error or repeat frame code error 
	M85sRMask dd ?			;  Receive fail bit mask (below)		
	M85s5hbtt dd ?			;  "5 half-bit transition" errors.		
	M85sACerr dd ?			;  Access Control errors				
	M85sFsAbrtD dd ?		;  Frames sent: w/ abort delimiter		
	M85sFsNrcv dd ?			;  Frames sent: Not received.			
	M85sFrNoBuf dd ?		;  Frames recognized, no buffers		
	M85sFcopyErr dd ?		;  Frame copy errors					
	M85sFreqErr dd ?		;  Frequency errors						
	M85sMonRegen dd ?		;  Active Monitor regeneration count.	
	M85sRsrvd1 dd ?		 	;  reserved								
	M85sRsrvd2 dd ?			;  reserved 							
	M85sRsrvd3 dd ?			;  reserved								
	M85sTMask dd ?			;  Transmit fail bit mask (below)		
MAC8025Stat ends

; 
; * Receive error failure codes.
; 
M85Sb_RFCon		equ 1
M85Sb_RFCopy		equ 2

; 
; * Transmit error failure codes.
; 
M85Sb_TFunrun		equ 1
M85Sb_TFLine		equ 2
M85Sb_TFAbDlm		equ 4
M85Sb_TFLost		equ 8
M85Sb_TFToken		equ 16


; 
; * MAC upper dispatch table
; 
MACUprDisp struc

	MudCCp dd ?			;  Back pointer to CC table		

	; 
;	 * MAC entry points, all are assembly functions that conform to the 
;	 * Microsoft FAR PASCAL calling conventions.
;	 
	MudGReq dd ?
	MudXmitChain dd ?
	MudXferData dd ?
	MudRcvRelease dd ?
	MudIndOn dd ?
	MudIndOff dd ?
MACUprDisp ends

; 
; * protocol lower dispatch table
; 
ProtLwrDisp struc

	PldCCp dd ?				;  Back pointer to CC table	

	PldIFF dd ?								;  Interface flags			

	; 
;	 * Protocol stack entry points.
;	 
	PldReqConfirm dd ?
	PldXmitConfirm dd ?
	PldRcvLkAhead dd ?
	PldIndComplete dd ?
	PldRcvChain dd ?
	PldStatInd dd ?
ProtLwrDisp ends

; 
; * Data structures for TransmitChain (Tx), TransferData (TD), and
; * ReceiveChain (Rx).
; 
TxDataBlock struc
	TxPtrType db ? 			;  0 => Physical pointer, 1 => GDT pointer	
	TxRsvdByte db ?			;  Reserverd, must be zero.					
	TxDataLen dw ? 			;  Data block length in bytes.				
	TxDataPtr dd ?   			;  Far pointer to data block.				
TxDataBlock ends

; 
; * Maximum number of data blocks supported in a descriptor.
; 
MAX_DATABLK		equ 8

; 
; * immediate data buffer length cannot exceed 64 bytes.
; 
MAX_IMMED_LEN		equ 64

; 
; * Maximum number of data blocks supported in a Tx descriptor.
; 
MAX_TX_DATABLK		equ MAX_DATABLK

; 
; * TransmitChain descriptor structure definition
; 
TxBufDesc struc
	TxImmedLen dw ?			;  Byte count of immediate data (max = 64)	
	TxImmedPtr dd ?			;  Virtual address of Immediate data		
	TxDataCount dw ?		;  Number of Data Blocks (max = 8)			

	TxDataBlk db size TxDataBlock*MAX_TX_DATABLK dup(0)
TxBufDesc ends

; 
; * TransferData data block description
; 
TDDataBlock struc
	TDPtrType db ?				;  0 => Physical pointer, 1 => GDT pointer	
	TDRsvdByte db ?				;  Reserverd, must be zero.						
	TDDataLen dw ?				;  Data block length in bytes.					
	TDDataPtr dd ?				;  Far pointer to data block.						
TDDataBlock ends

; 
; * Max number of TransferData blocks
; 
MAX_TD_DATABLK		equ MAX_DATABLK

; 
; * TransferData descriptor structure
; 
TDBufDesc struc
	TDDataCount dw ?			;  Number of Data Blocks (max = 8)				

	TDDataBlk db size TDDataBlock*MAX_TD_DATABLK dup(0)
TDBufDesc ends

; 
; * ReceiveChain data block structure definition
; 
RxDataBlock struc
	RxDataLen dw ?				;  Length of the Data Block						
	RxDataPtr dd ?				;  Far Pointer to Data Block						
RxDataBlock ends

; 
; * MAx number of ReceiveChain data blocks
; 
MAX_RX_DATABLK		equ MAX_DATABLK

; 
; * ReceiveChain descriptor structure definition
; 
RxBufDesc struc
	RxDataCount dw ?			;  Number of Data Blocks.							

	RxDataBlk db size RxDataBlock*MAX_RX_DATABLK dup(0)
RxBufDesc ends

;  
; * Data structures for the "Module Configuration" structure parsed from
; * the PROTOCOL.INI file. see NDIS spec for details.
; 
Param struc

	ParamType dw ?				;  0 => 31 bit signed integer, 1 => string	
	ParamLen dw ?				;  String length (including null) or 4.		

	; 
;	 * the parameter immediately follows this structure, it can be any
;	 * length or type and follows the union structure. the AWK script that 
;	 * generates ndis.inc is just dumb enough that it's not able to parse 
;	 * union definitions. so all references to ParNum and ParStr are indirect.
;	 
	; 
;	 *    union ParValu 
;	 *       {
;	 *       long ParNum
;	 *       BYTE ParStr[1]
;	 *       } ParVal
;	 
Param ends

KeywordEntry struc
	NextKeywordEntry dd ?		;  Forward pointer 		
	PrevKeywordEntry dd ?		;  Back Pointer			

	KeyWord db size byteSize*NAME_LEN dup(0) 	;  Keyword on left side of "="				
	NumParams dw ?			;  Number of parameters on r. side of "=" 	

	Params db size Param*1 dup(0)		;  Actual size depends on NumParams			
KeywordEntry ends

ModCfg struc
	NextModCfg dd ? 	;  Module config images are in a		
	PrevModCfg dd ?	;  double-linked list.					 

	ModName db size byteSize*NAME_LEN dup(0)		;  Bracketed Module Name.				

	KE db size KeywordEntry*1 dup(0)		;  Head of Keyword list, always one 	

ModCfg ends

ProIni struc
	MC db size ModCfg*1 dup(0)			;  Head of Module config list.			
ProIni ends

; 
; * request block structure used to register a module
; 
RqBlk struc
	Opcode dw ?
	Status dw ?
	Pointer1 dd ?
	Pointer2 dd ?
	Word1 dw ?
RqBlk ends

; 
; * the minimum lookahead defaults to 64, maximum is 256 bytes.
; 
MIN_LOOKAHEAD_DEFAULT		equ 64
MAX_LOOKAHEAD		equ 256

; 
; * Device driver CONFIG.SYS lines are a maximum of 132 bytes.
; 
DEVINIT_LEN		equ 132

; 
; * define a general structure used to coalesce a variety of variables into
; * one place. these varables include the indications nest flag, level 0
; * CS and DS register values, etc.
; 
ndisGeneral struc

	; 
;	 * this pointer is set when a packet is received. transferData will use
;	 * it to move the data up to the protocol. the size of the buffer must
;	 * also be remembered. If multiple TransferData calls are supported,
;	 * currBuff and currBuffSize must be preserved until the next
;	 * ReceiveLookAhead call.
;	 
	currBuff dd ?
	currBuffSize dw ?

	; 
;	 * the protocol has a notion of minimum lookahead data. it can call down
;	 * to change it at will.
;	 
	minLookAhead dw ?

	; 
;	 * indications are tracked using a nesting level byte. If this byte is 
;	 * non-zero, then indications are disabled.
;	 
	indicationNestLevel db ?

	; 
;	 * allocate space for a GDT selector. the selector is allocated at driver
;	 * startup and is generally used to initialize currBuff. gdt0 is used for
;	 * OS/2 on a 80386. note that gdt0 cannot be used until after init time.
;	 
	gdt dw ?
	gdt0 dw ?
	gdt0set dw ?

	; 
;	 * this flag is 0 during init time, non-zero at ring 0 time. The most
;	 * important use is for debug after init time. Character output after
;	 * init time must go direct to the UART, not to DOSWRITE.
;	 
	ring0 db ?

	; 
;	 * This flag is set to true everytime an indication (Receive or status)
;	 * upcall is made to the stack.
;	 * the ISR should check this just before exiting in order to see if an
;	 * indication complete is needed. 
;	 
	needIndCmplt db ?

	; 
;	 * Specifies which adapter must be used. This number may be non-zero if 
;	 * there are multiple adapters in the box. The current generic model
;	 * requires one driver per adapter. In the future, one driver should
;	 * be able to manage multiple adapters.
;	 
	adapterNum dw ?

	; 
;	 * The PROTMAN$ file handle goes here during init time.
;	 
	PMHandle dw ?

	; 
;	 * remember the processor type
;	 
	procType dw ?

	; 
;	 * store the device= string here at init time.
;	 
	devInitStr db size byteSize*DEVINIT_LEN dup(0)
ndisGeneral ends

; 
; * the C function generalRequests() gets a far pointer to a structure of this
; * type.
; 
GenReq struc
	GReqProtID dw ?
	GReqReqHandle dw ?
	GReqP1 dw ?
	GReqP2 dd ?
	GReqOpCode dw ?
	GReqMacDs dw ?
GenReq ends

; 
; * ndis.c generally instantiates this structure.
; 
extrn _ndisGen:BYTE

; 
; * At bind time, the protocol gives a pointer to it's common characteristics
; * table. The contents of that table are cached in protCC for faster access
; * without incurring segment register hits.
; 
extrn _protCC:BYTE

; 
; * Protocol lower dispatch table. The contents of that table are cached in 
; * pldDsptchTble for faster access without incurring segment register hits.
; 
extrn _pldDsptchTble:BYTE

; 
; * Characteristics table names
; 
extrn _MSC:BYTE
extrn _MSS:BYTE
extrn _MUD:BYTE
extrn _MCBuff:BYTE
extrn _MCC:BYTE

;endif

