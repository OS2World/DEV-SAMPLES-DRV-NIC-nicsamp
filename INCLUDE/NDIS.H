/*
 * file $Header:  \ibm16\ndis.h  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 *
 * 		Definitions and structures related to the Network Driver Interface 
 *		Specification (NDIS) generic model. This version conforms to the
 *		NDIS Spec rev 2.0.1.
 *
 *		Names of structures, types, tables, and functions follow the 
 *		NDIS naming conventions, whenever practical.
 *
 *
 */
  
/*
 * prevent multiple inclusions
 */					
#ifndef NDIS_H
#define NDIS_H

/*
 * Type definitions used throughout. These match the nomenclature in the NDIS
 * Spec.
 */
typedef unsigned char 	BYTE;
typedef unsigned short	WORD;
typedef unsigned long	DWORD;

typedef BYTE far * LPBUF;
typedef WORD (far *LPFUN)();
typedef char far *LPSZ;

typedef WORD *FARp;

/*
 * NDIS drivers cannot have legal NULL pointers. Hence, the use of 0 as a
 * NULL pointer value.
 */
#ifndef NULL
#define NULL 0
#endif

/*
 * define the length of names and addresses in the Common Characteristics
 * and Specific Characteristics table.
 */
#define NAME_LEN	16
#define ADDR_LEN	16
#define ADDR_SIZE	 6

/*
 * maximum number of multicast addresses. This should probably be defined in
 * HW.H since it is more driver specific.
 */
#define NUM_MCADDRS	 4

/*
 * NDIS System request opcodes supported by the generic model.
 */
#define InitiateBind				1
#define Bind						2

/*
 * Protocol Manager opcodes used by the generic model
 */
#define GetPMInfo					1
#define RegisterModule  			2
#define BindAndStart				3
#define GetPMLinkage				4

/*
 * NDIS General Request opcodes supported by the generic model
 */
#define InitiateDiagnostics			1
#define ReadErrorLog				2
#define SetStationAddress			3
#define OpenAdapter			 		4
#define CloseAdapter				5
#define ResetMAC					6
#define SetPacketFilter		 		7
#define AddMulticastAddress			8
#define DeleteMulticastAddress 		9
#define UpdateStatistics			10
#define ClearStatistics		 		11
#define Interrupt					12
#define SetFunctionalAddress		13
#define SetLookAhead				14

/*
 * NDIS Status Indication opcodes
 */
#define RingStatus				1
#define AdapterCheck			2
#define StartReset				3
#define InterruptStatus		 	4
#define EndReset				5

/*
 * All possible return error codes that are defined by the NDIS Spec. 2.0.1
 */
#define SUCCESS				 	0x0000
#define WAIT_FOR_RELEASE		0x0001
#define REQUEST_QUEUED			0x0002
#define FRAME_NOT_RECOGNIZED	0x0003
#define FRAME_REJECTED			0x0004
#define FORWARD_FRAME			0x0005
#define OUT_OF_RESOURCE		 	0x0006
#define INVALID_PARAMETER		0x0007
#define INVALID_FUNCTION		0x0008
#define NOT_SUPPORTED			0x0009
#define HARDWARE_ERROR			0x000A
#define TRANSMIT_ERROR			0x000B
#define NO_SUCH_DESTINATION		0x000C
#define ALREADY_STARTED		 	0x0020
#define INCOMPLETE_BINDING		0x0021
#define DRIVER_NOT_INITIALIZED 	0x0022
#define HARDWARE_NOT_FOUND		0x0023
#define HARDWARE_FAILURE		0x0024
#define CONFIGURATION_FAILURE	0x0025
#define INTERRUPT_CONFLICT		0x0026
#define INCOMPATIBLE_MAC		0x0027
#define INITIALIZATION_FAILED	0x0028
#define GENERAL_FAILURE		 	0x00FF

/*
 * NDIS Adapter Check error codes
 */
#define AdapCheckInoperative		0x8000
#define AdapCheckIllegalOp			0x1000
#define AdapCheckLocalParityErr		0x0800
#define AdapCheckParityErr			0x0400
#define AdapCheckInternalParityErr	0x0100
#define AdapCheckRingXmitParityErr	0x0080
#define AdapCheckRingRcvParityErr	0x0040
#define AdapCheckXmitOverrun		0x0020
#define AdapCheckRcvOverrun			0x0010
#define AdapCheckUnrecogInterrupt	0x0008
#define AdapCheckUnrecogErrInt		0x0004
#define AdapCheckNoPCSysService		0x0003
#define AdapCheckUnrecogSuperReq	0x0002
#define AdapCheckProgramRequest		0x0001

/*
 * NDIS protocol levels, interface type
 */
#define MACLvl						1
#define MACTyp						1

/*
 * NDIS Protocol Table structure definitions
 */

/*
 * NDIS Common Characteristics table
 */
struct CommChar {
	WORD	CcSize;				/* Table size							*/
	WORD	CcLvl;			 	/* Level of table						*/
	WORD	CcSpLvl;			/* Level of spec tables			 		*/
	BYTE	CcMjrVer;		 	/* Module Major version number			*/
	BYTE	CcMnrVer;		 	/* Module Minor version number			*/
	DWORD	CcBindFnc;			/* "Hints" on binding support			*/
	BYTE	CcName[NAME_LEN];	/* Module name							*/
	BYTE	CcUPLevel;		 	/* Upper protocol level			 		*/
	BYTE	CcUIType;			/* Upper interface type			 		*/
	BYTE	CcLPLevel;		 	/* Lower protocol level			 		*/
	BYTE	CcLIType;			/* Lower interface type			 		*/
	WORD	CcModuleID;			/* Module ID (supplied by PM)			*/
	WORD	CcDataSeg;		 	/* Module Data Segment					*/

	/*
	 * System request function pointer
	 */
	WORD	(far pascal *CcSysReq)(DWORD,DWORD,WORD,WORD,WORD);

	/*
	 * links to other NDIS characteristics tables.
	 */
	struct MACSpecChar far	*CcSCp;	/* specific characteristics 		*/
	struct MACSpecStat far	*CcSSp;	/* specific status					*/
	struct MACUprDisp far	*CcUDp;	/* upper dispatch table	 			*/
	struct ProtLwrDisp far	*CcLDp;	/* lower dispatch table	 			*/

	DWORD	CcRsv1;				/* reserved */
	DWORD	CcRsv2;				/* reserved */
};

/*
 * Definintions for the MAC Service Supported Flags.
 * Bits 0-14.
 */
#define BROADCAST_SUPP			(DWORD)0x00000001
#define MULTICAST_SUPP		 	(DWORD)0x00000002
#define FUNC_GROUP_ADDR_SUPP	(DWORD)0x00000004
#define PROMISCUOUS_SUPP		(DWORD)0x00000008
#define SET_STN_ADDR_SUPP		(DWORD)0x00000010
#define STAT_ALWAYS_CURR		(DWORD)0x00000020
#define DIAGNOSTICS_SUPP		(DWORD)0x00000040
#define LOOPBACK_SUPP			(DWORD)0x00000080
#define RECEIVECHAIN_MOSTLY		(DWORD)0x00000100
#define IBM_SOURCE_R_SUPP		(DWORD)0x00000200
#define RESET_MAC_SUPP			(DWORD)0x00000400
#define OPEN_ADAPTER_SUPP		(DWORD)0x00000800
#define INTERRUPT_RQ_SUPP		(DWORD)0x00001000
#define SRC_RTNG_BRDG_SUPP		(DWORD)0x00002000
#define GDT_ADDRESS_SUPP		(DWORD)0x00004000
#define MULTIPLE_XFER_DATA_SUPP	(DWORD)0x00008000
#define RCVLK_FRAME_SIZE_ZERO	(DWORD)0x00010000

/*
 * MAC Service Specific characteristics table
 */
struct MACSpecChar {
	WORD	MscSize;					/* Table size						*/
	BYTE	MscType[NAME_LEN]; 			/* MAC type name					*/
	WORD	MscStnAdrSz;				/* Station address length			*/
	BYTE	MscPermStnAdr[ADDR_LEN];	/* Permanent station address		*/
	BYTE	MscCurrStnAdr[ADDR_LEN];	/* Current station address			*/
	DWORD	MscCurrFncAdr;				/* Current functional address		*/

	struct MCastBuf far *MscMCp;		/* Address of multicast buffer		*/

	DWORD	MscLinkSpd;					/* Link speed (bits/sec)			*/
	DWORD	MscService; 				/* Services supported flags			*/
	WORD	MscMaxFrame;				/* Maximum frame size				*/
	DWORD	MscTBufCap;					/* Transmit buffer capacity			*/
	WORD	MscTBlkSz;		 			/* Transmit buf alloc block size	*/
	DWORD	MscRBufCap;					/* Receive buffer capacity			*/
	WORD	MscRBlkSz;		 			/* Receive buf alloc block size		*/
	BYTE	MscVenCode[3];	 			/* Vendor code						*/
	BYTE	MscVenAdapter;	 			/* Vendor adapter code				*/
	BYTE far *MscVenAdaptDesc;			/* Ptr to vendor adapter desc		*/
	WORD	MscInterrupt;				/* Interrupt level used				*/
	WORD	MscTxQDepth;				/* Transmit Queue Depth				*/
	WORD	MscMaxDataBlocks;			/* Max buff descriptor data blocks 	*/
};

/*
 * multicast address structure is ADDR_LEN bytes long
 */
struct MCastAddr {
	BYTE	mAddr[ADDR_LEN];
};

/*
 * the multicast address buffer manages NUM_MCADDRS multicast address
 * structures.
 */
struct MCastBuf {
	WORD	McbMax;				/* max # of multicast addresses			*/	
	WORD	McbCnt;				/* curr# "		"			"			*/

	struct MCastAddr McbAddrs[NUM_MCADDRS];
};

/*
 * MAC service-specific status table
 */
struct MACSpecStat {
	WORD	MssSize;			/* Table size							*/
	DWORD	MssDiagDT;			/* Last diagnostic Date/Time			*/
	DWORD	MssStatus;			/* MAC status - bit mask				*/ 
	WORD	MssFilter;			/* Current packet filter				*/
	void far *MssM8Sp;			/* pointer to Media specific status		*/
	DWORD	MssClearDT;			/* Last clear stats Date/Time			*/
	DWORD	MssFR;				/* Frames received: total				*/
	DWORD	MssRFCRC;			/* Receive fail: CRC error				*/
	DWORD	MssFRByt;			/* Frames received: total bytes			*/
	DWORD	MssRFLack;			/* Receive fail: lack of buffers		*/
	DWORD	MssFRMC;			/* Frames received: multicast			*/
	DWORD	MssFRBC;			/* Frames received: broadcast			*/
	DWORD	MssRFErr;			/* rcv fail: errors in general			*/
	DWORD	MssRFMax;			/* rcv fail: exceeds max size			*/
	DWORD	MssRFMin;			/* rcv fail: less than min size			*/
	DWORD	MssFRMCByt;			/* Frames rcvd: multicast bytes			*/
	DWORD	MssFRBCByt;			/* Frames rcvd: broadcast bytes			*/
	DWORD	MssRFHW;			/* rcv fail: hardware error				*/
	DWORD	MssFS;				/* Frames sent: total					*/
	DWORD	MssFSByt;			/* Frames sent: total bytes				*/
	DWORD	MssFSMC;			/* Frames sent: multicast				*/
	DWORD	MssFSBC;			/* Frames sent: broadcast				*/
	DWORD	MssFSBCByt;			/* Frames sent: broadcast bytes			*/
	DWORD	MssFSMCByt;			/* Frames sent: multicast bytes			*/
	DWORD	MssSFTime;			/* Send fail: time-out					*/
	DWORD	MssSFHW;			/* Send fail: hardware error			*/
};

/*
 * Packet filter bit definitions.
 */
#define FLTR_DIRECTED 	0x01
#define FLTR_BRDCST		0x02
#define FLTR_PRMSCS		0x04
#define FLTR_SRC_RTG	0x08

/*
 * MAC status bits in the MAC Specific Characteristics table.
 * Bits 0-2 are coded to reflect the hardware status.
 */
#define HW_MISSING	0x0000
#define HW_BUD_ERR	0x0001
#define HW_CFG_ERR	0x0002
#define HW_FAULT	0x0003
#define HW_SOFT_ERR 0x0004
#define HW_OK		0x0007
#define MS_HW_MASK  HW_OK

#define MS_BOUND	0x0008
#define MS_OPEN		0x0010
#define MS_DIAGS_ON	0x0020


/*
 * 802.3 (Token Ring) status table
 */
struct MAC8023Stat {
	WORD	M83sSize;			/* Table size							*/
	WORD	M83sVer;			/* Version								*/
	DWORD	M83sRFAln;			/* Receive fail: Alignment error		*/
	DWORD	M83sRMask;			/* Receive fail bit mask (below)		*/
	DWORD	M83sRFOvrn;			/* Receive fail: Overrun				*/
	DWORD	M83sFSCols;			/* Frames sent: after collisions		*/
	DWORD	M83sFSDfr;			/* Frames sent: after deferring			*/
	DWORD	M83sSFColMx; 		/* Frames not sent: Max collisions		*/
	DWORD	M83sTotCol;			/* Total collision during tran attempts	*/
	DWORD	M83sTotLCol; 		/* Total late collisions				*/
	DWORD	M83sFSCol1;		 	/* Frames sent: after 1 collision		*/
	DWORD	M83sFSColM;			/* Frames sent: multpl collisions		*/
	DWORD	M83sFSHrtB;		 	/* Frames sent: CD heart beat			*/
	DWORD	M83sJabber;			/* Jabber errors						*/
	DWORD	M83sLostCS;			/* Lost carrier sense during tran		*/
	DWORD	M83sTMask;			/* Transmit fail bit mask (below)		*/
};

/*
 * Receive error failure codes.
 */
#define M83Sb_RFCRC		1		/* Recv bit	0	CRC error				*/
#define M83Sb_RFFE		2		/*			1	Framing error			*/
#define M83Sb_RFMaxSz	4		/*			2	Frame size > Max		*/

/*
 * Transmit error failure codes.
 */
#define M83Sb_TFColMx	1		/* Xmit bit 0	Excessive collisions	*/
#define M83Sb_TFCrr		2		/*			1	Carrier check failed	*/
#define M83Sb_TFShrt  	4		/*			2	Short circuit			*/
#define M83Sb_TFOpen  	8		/*			3	Open circuit			*/
#define M83Sb_TFLong  	16		/*			4	Frame size > Max		*/
#define M83Sb_TFRemFl	32		/*			5	Remote fail to defer	*/



/*
 * 802.5 (Ethernet) status table
 */
struct MAC8025Stat {
	WORD	M85sSize;			/* Table size							*/
	WORD	M85sVer;			/* Version								*/
	DWORD	M85sFCS;			/* FCS error or repeat frame code error */
	DWORD	M85sRMask;			/* Receive fail bit mask (below)		*/
	DWORD	M85s5hbtt;			/* "5 half-bit transition" errors.		*/
	DWORD	M85sACerr;			/* Access Control errors				*/
	DWORD	M85sFsAbrtD;		/* Frames sent: w/ abort delimiter		*/
	DWORD	M85sFsNrcv;			/* Frames sent: Not received.			*/
	DWORD	M85sFrNoBuf;		/* Frames recognized, no buffers		*/
	DWORD	M85sFcopyErr;		/* Frame copy errors					*/
	DWORD	M85sFreqErr;		/* Frequency errors						*/
	DWORD	M85sMonRegen;		/* Active Monitor regeneration count.	*/
	DWORD	M85sRsrvd1;		 	/* reserved								*/
	DWORD	M85sRsrvd2;			/* reserved 							*/
	DWORD	M85sRsrvd3;			/* reserved								*/
	DWORD	M85sTMask;			/* Transmit fail bit mask (below)		*/
};

/*
 * Receive error failure codes.
 */
#define	M85Sb_RFCon		1		/* Recv bit	0	Receiver Congestion		*/
#define	M85Sb_RFCopy	2		/* 			1	Frame Copy error		*/

/*
 * Transmit error failure codes.
 */
#define M85Sb_TFunrun	1		/* Xmit bit 0	Transmit underrrun		*/
#define M85Sb_TFLine	2		/* 			1	Transmit Line error		*/
#define M85Sb_TFAbDlm	4		/* 			2	Abort delimiter xmitted	*/
#define M85Sb_TFLost	8		/* 			3	Frame Lost				*/
#define M85Sb_TFToken	16		/* 			4	Token error				*/


/*
 * MAC upper dispatch table
 */
struct MACUprDisp {

	struct CommChar far *MudCCp;			/* Back pointer to CC table		*/

	/*
	 * MAC entry points, all are assembly functions that conform to the 
	 * Microsoft FAR PASCAL calling conventions.
	 */
	WORD (far pascal *MudGReq)(WORD,WORD,WORD,DWORD,WORD,WORD);
	WORD (far pascal *MudXmitChain)(WORD,WORD,LPBUF,WORD);
	WORD (far pascal *MudXferData)(LPBUF,WORD,LPBUF,WORD);
	WORD (far pascal *MudRcvRelease)(WORD,WORD);
	WORD (far pascal *MudIndOn)(WORD);
	WORD (far pascal *MudIndOff)(WORD);
};

/*
 * protocol lower dispatch table
 */
struct ProtLwrDisp {

	struct CommChar far *PldCCp;				/* Back pointer to CC table	*/

	DWORD PldIFF;								/* Interface flags			*/

	/*
	 * Protocol stack entry points.
	 */
	WORD (far pascal *PldReqConfirm)(WORD,WORD,WORD,WORD,WORD,WORD);
	WORD (far pascal *PldXmitConfirm)(WORD,WORD,WORD,WORD,WORD);
	WORD (far pascal *PldRcvLkAhead)(WORD,WORD,WORD,LPBUF,LPBUF,WORD);
	WORD (far pascal *PldIndComplete)(WORD,WORD);
	WORD (far pascal *PldRcvChain)(WORD,WORD,WORD,LPBUF,LPBUF,WORD);
	WORD (far pascal *PldStatInd)(WORD,WORD,LPBUF,WORD,WORD);
};

/*
 * Data structures for TransmitChain (Tx), TransferData (TD), and
 * ReceiveChain (Rx).
 */
struct TxDataBlock {
	BYTE	TxPtrType; 			/* 0 => Physical pointer, 1 => GDT pointer	*/
	BYTE	TxRsvdByte;			/* Reserverd, must be zero.					*/
	WORD	TxDataLen; 			/* Data block length in bytes.				*/
	LPBUF TxDataPtr;   			/* Far pointer to data block.				*/
};

/*
 * Maximum number of data blocks supported in a descriptor.
 */
#define MAX_DATABLK	8

/*
 * immediate data buffer length cannot exceed 64 bytes.
 */
#define MAX_IMMED_LEN	64

/*
 * Maximum number of data blocks supported in a Tx descriptor.
 */
#define MAX_TX_DATABLK	MAX_DATABLK

/*
 * TransmitChain descriptor structure definition
 */
struct TxBufDesc {
	WORD	TxImmedLen;			/* Byte count of immediate data (max = 64)	*/
	LPBUF	TxImmedPtr;			/* Virtual address of Immediate data		*/
	WORD	TxDataCount;		/* Number of Data Blocks (max = 8)			*/

	struct TxDataBlock TxDataBlk[MAX_TX_DATABLK];
};

/*
 * TransferData data block description
 */
struct TDDataBlock {
	BYTE	TDPtrType;				/* 0 => Physical pointer, 1 => GDT pointer	*/
	BYTE	TDRsvdByte;				/* Reserverd, must be zero.						*/
	WORD	TDDataLen;				/* Data block length in bytes.					*/
	LPBUF TDDataPtr;				/* Far pointer to data block.						*/
};

/*
 * Max number of TransferData blocks
 */
#define MAX_TD_DATABLK	MAX_DATABLK

/*
 * TransferData descriptor structure
 */
struct TDBufDesc {
	WORD	TDDataCount;			/* Number of Data Blocks (max = 8)				*/

	struct TDDataBlock TDDataBlk[MAX_TD_DATABLK];
};

/*
 * ReceiveChain data block structure definition
 */
struct RxDataBlock {
	WORD	RxDataLen;				/* Length of the Data Block						*/
	LPBUF RxDataPtr;				/* Far Pointer to Data Block						*/
};

/*
 * MAx number of ReceiveChain data blocks
 */
#define MAX_RX_DATABLK	MAX_DATABLK

/*
 * ReceiveChain descriptor structure definition
 */
struct RxBufDesc {
	WORD	RxDataCount;			/* Number of Data Blocks.							*/

	struct RxDataBlock RxDataBlk[MAX_RX_DATABLK];
};

/* 
 * Data structures for the "Module Configuration" structure parsed from
 * the PROTOCOL.INI file. see NDIS spec for details.
 */
struct Param {

	WORD ParamType;				/* 0 => 31 bit signed integer, 1 => string	*/
	WORD ParamLen;				/* String length (including null) or 4.		*/

	/*
	 * the parameter immediately follows this structure, it can be any
	 * length or type and follows the union structure. the AWK script that 
	 * generates ndis.inc is just dumb enough that it's not able to parse 
	 * union definitions. so all references to ParNum and ParStr are indirect.
	 */
	/*
	 *    union ParValu 
	 *       {
	 *       long ParNum;
	 *       BYTE ParStr[1];
	 *       } ParVal;
	 */
};

struct KeywordEntry {
	struct KeywordEntry far *NextKeywordEntry;		/* Forward pointer 		*/
	struct KeywordEntry far *PrevKeywordEntry;		/* Back Pointer			*/

	BYTE	KeyWord[NAME_LEN]; 	/* Keyword on left side of "="				*/
	WORD	NumParams;			/* Number of parameters on r. side of "=" 	*/

	struct Param Params[1];		/* Actual size depends on NumParams			*/
};

struct ModCfg {
	struct ModCfg far *NextModCfg; 	/* Module config images are in a		*/
	struct ModCfg far *PrevModCfg;	/* double-linked list.					*/ 

	BYTE	ModName[NAME_LEN];		/* Bracketed Module Name.				*/

	struct KeywordEntry KE[1];		/* Head of Keyword list, always one 	*/

};									/* or more entries per module.			*/

struct ProIni {
	struct ModCfg MC[1];			/* Head of Module config list.			*/
};

/*
 * request block structure used to register a module
 */
struct RqBlk {
	WORD  Opcode;
	WORD  Status;
	void  far *Pointer1;
	void  far *Pointer2;
	WORD  Word1;
};

/*
 * the minimum lookahead defaults to 64, maximum is 256 bytes.
 */
#define MIN_LOOKAHEAD_DEFAULT	64
#define MAX_LOOKAHEAD			256

/*
 * Device driver CONFIG.SYS lines are a maximum of 132 bytes.
 */
#define DEVINIT_LEN	132

/*
 * define a general structure used to coalesce a variety of variables into
 * one place. these varables include the indications nest flag, level 0
 * CS and DS register values, etc.
 */
struct ndisGeneral {

	/*
	 * this pointer is set when a packet is received. transferData will use
	 * it to move the data up to the protocol. the size of the buffer must
	 * also be remembered. If multiple TransferData calls are supported,
	 * currBuff and currBuffSize must be preserved until the next
	 * ReceiveLookAhead call.
	 */
	BYTE	far *currBuff;
	WORD		currBuffSize;

	/*
	 * the protocol has a notion of minimum lookahead data. it can call down
	 * to change it at will.
	 */
	WORD	minLookAhead;

	/*
	 * indications are tracked using a nesting level byte. If this byte is 
	 * non-zero, then indications are disabled.
	 */
	BYTE	indicationNestLevel;

	/*
	 * allocate space for a GDT selector. the selector is allocated at driver
	 * startup and is generally used to initialize currBuff. gdt0 is used for
	 * OS/2 on a 80386. note that gdt0 cannot be used until after init time.
	 */
	WORD	gdt;
	WORD	gdt0;
	WORD	gdt0set;

	/*
	 * this flag is 0 during init time, non-zero at ring 0 time. The most
	 * important use is for debug after init time. Character output after
	 * init time must go direct to the UART, not to DOSWRITE.
	 */
	BYTE	ring0;

	/*
	 * This flag is set to true everytime an indication (Receive or status)
	 * upcall is made to the stack.
	 * the ISR should check this just before exiting in order to see if an
	 * indication complete is needed. 
	 */
	BYTE	needIndCmplt;

	/*
	 * Specifies which adapter must be used. This number may be non-zero if 
	 * there are multiple adapters in the box. The current generic model
	 * requires one driver per adapter. In the future, one driver should
	 * be able to manage multiple adapters.
	 */
	WORD	adapterNum;

	/*
	 * The PROTMAN$ file handle goes here during init time.
	 */
	WORD	PMHandle;

	/*
	 * remember the processor type
	 */
	WORD	procType;

	/*
	 * store the device= string here at init time.
	 */
	BYTE	devInitStr[DEVINIT_LEN];
};

/*
 * the C function generalRequests() gets a far pointer to a structure of this
 * type.
 */
struct GenReq {
	WORD	GReqProtID;
	WORD	GReqReqHandle;
	WORD	GReqP1;
	DWORD	GReqP2;
	WORD	GReqOpCode;
	WORD	GReqMacDs;
};

/*
 * ndis.c generally instantiates this structure.
 */
extern struct ndisGeneral ndisGen;

/*
 * At bind time, the protocol gives a pointer to it's common characteristics
 * table. The contents of that table are cached in protCC for faster access
 * without incurring segment register hits.
 */
extern struct CommChar protCC;

/*
 * Protocol lower dispatch table. The contents of that table are cached in 
 * pldDsptchTble for faster access without incurring segment register hits.
 */
extern struct ProtLwrDisp pldDsptchTble;

/*
 * Characteristics table names
 */
extern struct MACSpecChar	MSC;
extern struct MACSpecStat	MSS;
extern struct MACUprDisp	MUD;
extern struct MCastBuf		MCBuff;
extern struct CommChar 		MCC;

#endif

