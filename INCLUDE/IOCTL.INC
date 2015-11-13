;
; $Header:   \ibm16\ioctl.inc  2-2-93 jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description: 
;     Definitions and structures for Operating system requests.
;     This is a subset of functions, only the INIT cmd and
;     genioctl are interesting for MAC drivers.  For ASM


; Status word masks
STERR	EQU	8000H		; Bit 15 - Error
STBUI	EQU	0200H		; Bit  9 - Busy
STDON	EQU	0100H		; Bit  8 - Done
STECODE EQU	00FFH		; Error code:
UNK_CMD	EQU	0003H		;   Unknown command.
WRECODE EQU	0



;**	ERRNZ - generate assembly error if expr is not zero
;
;	Short hand of .errnz for people who cannot type period.
;
;	ENTRY:	expr = expression to test
;
;	EXIT:	if expr is not zero then generate assembly error

ERRNZ	MACRO	expr
.errnz	expr
ENDM

;
;**	Device Driver Request Packet Defintions
;


PktMax		equ	18
PktHead 	equ	13

Packet	struc
PktLen		db	?		; length in bytes of packet
PktUnit 	db	?		; subunit number of block device
PktCmd		db	?		; command code
PktStatus	dw	?		; status word
PktDOSLink	dd	?		; reserved
PktDevLink	dd	?		; device multiple-request link
PktData 	db	PktMax dup (?)	; data pertaing to specific packet
Packet		ends

CMDInit 	equ	0			; INIT command
InitcUnit	equ	byte ptr PktData	; number of units returned
InitpDevHlp	equ	dword ptr PktData+1	; pointer DevHlp entry point
InitpEnd	equ	dword ptr PktData+1	; pointer to first free byte
InitEcode	equ	word ptr PktData+1	; seperate names for InitpEnd
InitEdata	equ	word ptr PktData+3	; first and second words

InitpBPB	equ	dword ptr PktData+5	; pointer to BPBs
InitDrv 	equ	byte ptr PktData+9	; drive no. assigned to unit 0
						; for resident drivers only:
InitSysiData 	equ	byte ptr PktData+11	; SysInit's DGROUP selector

LENInit 	equ	10			; non-resident drivers
LENInitRes 	equ	(LENInit + 2)		; resident drivers
ERRNZ		<PktMax LT LENInit>
ERRNZ		<PktMax LT LENInitRes>

CMDMedChk	equ	1			; Media Check
MedChkmedia	equ	byte ptr PktData	; last media byte seen
MedChkflag	equ	byte ptr PktData+1	; -1=change 0=dont know 1=no change
MedChkpVID	equ	dword ptr PktData+2	; pointer to VID
LENMedChk	equ	6
ERRNZ		<PktMax LT LENMedChk>

CMDBldBPB	equ	2			; build BPB
BldBPBmedia	equ	byte ptr PktData	; media byte
BldBPBbuffer	equ	dword ptr PktData+1	; scratch buffer
BldBPBpBPB	equ	dword ptr PktData+5	; returned pointer to BPB
LENBldBPB	equ	9
ERRNZ		<PktMax LT LENBldBPB>

CMDIOCTLR	equ	3			; reserved for 3.x compatability
CMDINPUT	equ	4			; read data from device
CMDOUTPUT	equ	8			; write data to device
CMDOUTPUTV	equ	9			; write data and verify
CMDIOCTLW	equ	12			; reserved for 3.x compatability
IOmedia 	equ	byte ptr PktData	; media byte
IOpData 	equ	dword ptr PktData+1	; transfer address
IOcount 	equ	word ptr PktData+5	; count of bytes/sectors
IOstart 	equ	word ptr PktData+7	; starting sector (block)
IOpVID		equ	dword ptr PktData+9	; returned VID (block)
IOsknum 	equ	byte ptr PktData+7	; screen/kbd number (char)
IOsscod 	equ	word ptr PktData+8	; sub-screen code (char)
LENIO		equ	13
ERRNZ		<PktMax LT LENIO>

CMDNDR		equ	5			; non-destructive read
NDRbyte 	equ	byte ptr PktData	; byte read from device
LENNDR		equ	1
ERRNZ		<PktMax LT LENNDR>

CMDInputS	equ	6			; input status
CMDOutputS	equ	10			; output status
LENStatus	equ	0
ERRNZ		<PktMax LT LENStatus>

CMDInputF	equ	7			; input flush
CMDOutputF	equ	11			; output flush
LENFlush	equ	0
ERRNZ		<PktMax LT LENFlush>

CMDOpen 	equ	13			; device open
CMDClose	equ	14			; device close
LENOpnCls	equ	0
ERRNZ		<PktMax LT LENOpnCls>

CMDRemMed	equ	15			; is media removable
LENRemMed	equ	0
ERRNZ		<PktMax LT LENRemMed>

CMDStart EQU	98				; start console output
CMDStop EQU	99				; stop console output
CStpSKG EQU	BYTE PTR PktData+0		; Screen/Keyboard number
LenStp	EQU	1				; Size of request additions.
ERRNZ		<PktMax LT LenStp>

SupCatagory	EQU	82h			; Supported catagory
CMDGenIOCTL	EQU	16			; Generic IOCTL
GIOCategory	EQU	BYTE PTR PktData	; Category Code
GIOFunction	EQU	BYTE PTR PktData+1	; Function code
GIOParaPack	EQU	DWORD PTR PktData+2	; pointer to parameter packet
GIODataPack	EQU	DWORD PTR PktData+6	; pointer to data packet
LENGenIOCTL	EQU	10			; Packet is 10 bytes long
ERRNZ		<PktMax LT LENGenIOCTL>

; Used internally by the Disk Driver for Internal requests
IOC_Fun 	EQU	BYTE PTR PktData	; Internal function code
IOC_IOpData	EQU	DWORD PTR PktData+1	; Data transfer address
IOC_Count	EQU	WORD PTR PktData+5	; Sector Count
IOC_Cyln	EQU	WORD PTR PktData+7	; Cylinder number
IOC_Head	EQU	WORD PTR PktData+9	; Head number
IOC_StartSec	EQU	WORD PTR PktData+11	; Starting sector number
IOC_SecSiz	EQU	BYTE PTR PktData+13	; Sector size index
LenSpecPkt	EQU	15
ERRNZ		<PktMax LT LenSpecPkt>

CMDResMed	EQU	17			; reset media uncertain
LENResMed	equ	0
ERRNZ		<PktMax LT LENResMed>

CMDAddOnPrep	EQU	97			; Prepare for add on
Add_On_NewDD	EQU	DWORD PTR PktData
Add_On_SrvRout	EQU	DWORD PTR PktData
LenAddOn	EQU	4

CMDGetLogMap	EQU	18
CMDSetLogMap	EQU	19
Logical_Drive	EQU	BYTE PTR PktData; logical drive number
LenGetSetLog	EQU	1

