;
; $Header:   \ibm16\opsys.asm  2-2-93  jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;       Operating System interface routines.  This file
;	contains the strategy routine for the MAC driver as
;       well as the simulated OS/2 DEVHELP functions for DOS.
;

include segments.inc
include ioctl.inc
include devhdr.inc
ifdef DOS
include devhlp.inc
include _misc.inc
endif

ifdef OS2

DevHeaderSeg	segment

; define the device header

	public	__DevHdr
	__DevHdr  SysDev  <-1,DEV_CHAR_DEV+DEVLEV_1+DEV_30,cgroup:Strategy,0>

DevHeaderSeg	ends

else

_TEXT segment

; define the device header

	public	__DevHdr
	__DevHdr  SysDev  <-1,DEV_CHAR_DEV+DEVLEV_1+DEV_GIOCTL+DEV_30,CGROUP:Strategy,CGROUP:DOSIntr>

_TEXT ends

endif

_DATA	segment

; define a far pointer to the device header and initialize at driver init
; time.

	public _DevHdr
	_DevHdr	dd ?

; define a far pointer for the Dev Help functions. it gets filled out at
; driver init time.

	public	_DevHlp
	_DevHlp	dd	?

; code ending location offsets and data ending location offsets go here.
; allocPhys() adds to the _DataEndOffset to increase the brk address.

	public	_CodeEndOffset
	public	_DataEndOffset

	_CodeEndOffset	dw	0
	_DataEndOffset	dw	0


ifdef DOS

OUTB	MACRO	ioReg
	mov	dx, ioReg
	out	dx, al
ENDM
	
PUBLIC_VAR	MACRO	var, val
	public	var
	var	dw	val
ENDM

;
; use a variable to store HW specific IO Base addresses and IVEC offsets.
; Platforms that use the same hardware, but at different addresses, can
; override the values at config time.
;
PUBLIC_VAR	_int1EOI,		INT_1EOIR
PUBLIC_VAR	_int1IMR,		INT_1IMR
PUBLIC_VAR	_int2EOI,		INT_2EOIR
PUBLIC_VAR	_int2IMR,		INT_2IMR
PUBLIC_VAR	_intBaseVec,	INT_BASE
PUBLIC_VAR	_altIntBaseVec,	INT_BASE_ALT


; define the DevHlp jump table for DOS. These functions simulate the OS/2
; DEVHELP functions and are searched by the DevHlp opcode.

	align	4
	public	_DevHlpJmpTab
_DevHlpJmpTab label byte
	DevHlpJmpTabDef	_DevHlp_EOI,			DevHlp_EOI
	DevHlpJmpTabDef	_DevHlp_NOP,			DevHlp_PhysToGDTSelector
	DevHlpJmpTabDef	_DevHlp_PhysToUVirt,	DevHlp_PhysToUVirt
	DevHlpJmpTabDef	_DevHlp_PhysToVirt,		DevHlp_PhysToVirt
	DevHlpJmpTabDef	_DevHlp_VirtToPhys,		DevHlp_VirtToPhys
	DevHlpJmpTabDef	_DevHlp_NOP,			DevHlp_UnPhysToVirt
	DevHlpJmpTabDef	_DevHlp_NOP,			DevHlp_AllocGDTSelector
	DevHlpJmpTabDef	_DevHlp_SetIRQ,			DevHlp_SetIRQ
	DevHlpJmpTabDef	_DevHlp_UnSetIRQ,		DevHlp_UnSetIRQ
	DevHlpJmpTabDef	_DevHlp_AllocPhys,		DevHlp_AllocPhys
	DevHlpJmpTabDef	_DevHlp_NOP,			DevHlp_FreePhys
;	DevHlpJmpTabDef	_DevHlp_SetTimer,		DevHlp_SetTimer
;	DevHlpJmpTabDef	_DevHlp_ResetTimer,		DevHlp_ResetTimer
	DevHlpJmpTabDef	0,						0

; maintain a table of old IRQ vectors used for chaining interrupts. save
; enough room for 16 IRQ numbers and the 4 byte interrupt vector.

	public	_IRQTab
_IRQTab	db	16 * 4	dup(0)

;
; save SS and SP on entry to Driver Interrupt routine so that the driver
; runs on a local stack.


	public				dosStackSetupBuffer
	dosStackSetupBuffer	dw	DOS_STACK_SIZE	dup (0F0ADh)  	; DOS device stack
	public				stackSetupBuffer
	stackSetupBuffer	dw	STACK_SETUP_SIZE dup (0DEADh)  	; DOS device stack
	public				tos_intstack
	tos_intstack	label	word	 ; address of the top of the interrupt stack.

endif

_DATA	ends

IDATA segment

	extrn	_startInitData:WORD

IDATA ends

_TEXT	segment
	assume CS:CGROUP, DS:DGROUP

	Public Strategy
;-----------------------------------------------------------------------
;
; Function: Strategy
;
; Inputs:
;	es:bx - points to the driver request packet
;
; Outputs:
;	none
;
; Side Effects:
;	Handle device driver strategy calls from OS/2. 
;	INIT, OPEN, CLOSE and Generic are the only functions handled.
;	The INIT function call is only made once, by the operating
;	system.
;	The OPEN function is supported to allow other drivers
;	to see if the driver "exists". The generic function is supported
;	for general status and HW specific functions.
;	Status is returned in the request packet.
;
ifdef DOS

; remember the packet pointer

	packetPtr		dd	?

endif


Strategy	proc	far

ifdef DOS

; stash the packet pointer

	mov	word ptr cs:[packetPtr], bx
	mov	word ptr cs:[packetPtr+2], es
	ret

_DOSIntr:

endif

; get _DevHdr setup

	mov	word ptr _DevHdr, offset __DevHdr
ifdef OS2
	mov	word ptr _DevHdr+2, ds
else
	mov	word ptr _DevHdr+2, cs

; setup the segment values in the device driver header. if DOS, these 
; values are not setup on entry.

	mov	word ptr cs:[__DevHdr].SDevProtCS, cs
	mov	word ptr cs:[__DevHdr].SDevRealCS, cs
	mov	word ptr cs:[__DevHdr].SDevProtDS, ds
	mov	word ptr cs:[__DevHdr].SDevRealDS, ds

endif

; Make sure reps increment by clearing the direction flag
;
	cld


; an init packet?
;
	cmp	es:[bx].PktCmd, CMDInit
	je	strategy_init		; go do the init if asked

; An open/close?
;
	cmp	es:[bx].PktCmd, CMDOpen
	je	strategy_open
	cmp	es:[bx].PktCmd, CMDClose
	je	strategy_close

; handle Generic IOCTLs
;
	cmp	es:[bx].PktCmd, CMDGenIOCTL
	je	strategy_Generic
;
; flag this as a bad request (set done, error, unknown command)
; and return.
;
errExit:

	mov	es:[bx].PktStatus, STDON+STERR+UNK_CMD
;
	ret				; far

; Open/Close both return OK. (This just shows that we exist, used in
; determining the driver name when multiple adapters are used.)

strategy_open:
strategy_close:
	mov	es:[bx].PktStatus, STDON
	ret

; do device driver initialization
;
strategy_init:				
	extrn	_OSinit:near
	push	es			;
	push	bx			; OSinit(far *req_pkt)
	call	_OSinit		; will return with status set
					; in request packet.
	add	sp, 4
	ret

strategy_Generic:
	extrn	_GenIoctl:near
	cmp	es:[bx].GIOCategory,SupCatagory
	jne	errExit
	push	es			;
	push	bx			; GenIoctl(far *req_pkt)
	call	_GenIoctl		; will return with status set
					; in request packet.
	add	sp, 4			; will return with status set
	ret

Strategy	endp




;*****************************************************************************
;
; Function: __startInitData
;
; Inputs:
;	none
;
; Outputs:
;	return a far pointer to the start of initilization data.
;
; Side Effects:
;	none
;
__startInitData	proc near
	public	__startInitData
	mov	ax, offset DGROUP:_startInitData
	mov	dx, ds
	ret
__startInitData	endp

ifdef DOS

;*****************************************************************************
;
; Function: DOSIntr
;
; Inputs:
;	none
;
; Ouputs:
;	none
;
; Side Effects:
;	This is the DOS device driver interrupt entry point. It retrieves the
;	request packet stored in cs:[packetPtr] and jumps into the Strategy 
;	routine code that is common to DOS and OS/2.
;
;
DOSIntr	proc far
	public	DOSIntr
	DOS_STACK_SETUP

	; i know that the packet pointer has already been stashed

	les	bx, dword ptr cs:[packetPtr]

	; when Strategy returns, I want it to come back to me
	
	push	cs
	mov	ax, offset _DOSIntrReturn
	push	ax

	jmp	_DOSIntr

_DOSIntrReturn:

	DOS_STACK_RESTORE
	ret
DOSIntr	endp


;*****************************************************************************
;
; Function: __DevHlp
;
; Inputs:
;	AL - device help function code
;	as well as a variety of other registers, function code dependent
;
; Outputs:
;	returns the status of the function code handler, or CY set if none.
;
; Side Effects:
;	This is the interface function for the DOS simulation of OS/2 device
;	helper routines. It searches the _DevHlpJmpTab table for an opcode
;	match, and then invokes the associated function.
;
;	DOS only OS/2 emulation routines. The Device Helper entry point is not
;	passed in via the device driver header in DOS, rather it must be 
;	hardcoded and supplied by the DOS driver. At this point, not all of the 
;	OS/2 DevHlp functions are supported, only those that make sense and those
;	that ease the porting effort. The DevHlp entry point (__DevHlp) compares
;	the incoming function in DL and calls the appropriate handler. If the
;	handler is not found, then PANIC is executed. Following is the current 
;	list of DevHlp functions supported:
;
;		DevHlp_EOI
;		DevHlp_PhysToGDTSelector
;		DevHlp_PhysToUVirt
;		DevHlp_VirtToPhys
;		DevHlp_AllocGDTSelector
;		DevHlp_SetIRQ
;		DevHlp_UnSetIRQ
;		DevHlp_AllocPhys
;		DevHlp_FreePhys
;		DevHlp_SetTimer
;		DevHlp_ResetTimer
;
;

	funcPtr	equ	word ptr [bp-2]
__DevHlp	proc far
	public __DevHlp

	; standard prolog, make space for 1 word on the stack

	push	bp
	mov	bp, sp
	sub	sp, 2

	; save the registers used

	push	es
	push	di

	; setup to interogate the jump table

	mov	di, _DATA
	mov	es, di
	mov	di, offset _DATA:_DevHlpJmpTab

	; determine which DevHlp function to call.

_funcCodeLoop:

	; if the function pointer from the table is NULL, then PANIC

	cmp	word ptr es:[di].DevHlpFuncPtr, 0
	je	_panic

_nextCode:

	cmp	dl, byte ptr es:[di].DevHlpFuncCode
	je	_foundFuncCode
	add	di, size DevHlpJmpTabStruc
	jmp	short _funcCodeLoop


_foundFuncCode:

	; save the function pointer, restore the register set, and call the
	; indicated function.

	push	word ptr es:[di].DevHlpFuncPtr
	pop	funcPtr
	pop	di
	pop	es
	call	funcPtr

	; standard epilog

_xDevHlp:

	mov	sp, bp
	pop	bp
	ret

_panic:
;	PANIC	31
	push	ax
	mov	ax, 31
	int	3
	pop	ax
	jmp	short _xDevHlp

__DevHlp	endp

;*****************************************************************************
;
;	DevHlp_EOI
;
_DevHlp_EOI	proc near
	public	_DevHlp_EOI

	push	dx
	mov	dl, al

	mov	al,NONSPEC_EOI		  ;	non-specific EOI

	; if on the slave, then do it first for level sensitive machines (MCA).

	cmp	dl,8
	jb	_doMasterEoi
	OUTB	_int2EOI

_doMasterEoi:

	; send an EOI to the master

	OUTB	_int1EOI

	pop	dx
	clc
	ret
_DevHlp_EOI	endp

;*****************************************************************************
;
;	NOP function, just clear carry and return
;
_DevHlp_NOP	proc near
	public	_DevHlp_NOP
	clc
	ret
_DevHlp_NOP	endp

;*****************************************************************************
;
;	_DevHlp_PhysToUVirt - return incoming address in es:bx
;
_DevHlp_PhysToUVirt	proc near
	public	_DevHlp_PhysToUVirt

	; save low 4 bits

	mov	dx, bx
	and	dx, 0FH

	; get number paragraphs 

	mov	cl, 4
	shr	bx, cl
	mov	cl, 12
	sal	ax, cl
	or	bx, ax
	
	; return the pointer in es:bx

	mov	es, bx
	mov	bx, dx

	clc
	ret
_DevHlp_PhysToUVirt	endp

;*****************************************************************************
;
;	_DevHlp_PhysToVirt - return incoming address in AX:BX according to DH
;		DH - 0 --> DS:SI
;		DH - 1 --> ES:DI
;
_DevHlp_PhysToVirt	proc near
	public	_DevHlp_PhysToVirt
	push	dx

	; call PhysToVirt to do the work, result comes back in ES:BX

	push	es
	push	cx
	push	dx
	call	_DevHlp_PhysToUVirt
	pop	dx
	pop	cx

	; return the pointer in ds:si if DH==0

	cmp	dh, 0
	jne	_esdi
	mov	ax, es
	mov	ds, ax
	mov	si, bx
	pop	es
	jmp	short _xPhysToVirt

_esdi:
	add	sp, 2
	mov	di, bx

_xPhysToVirt:
	pop	dx
	clc
	ret
_DevHlp_PhysToVirt	endp
;*****************************************************************************
;
;	_DevHlp_VirtToPhys - return a virtual address in ax:bx
;	entry: ds:si
;	exit: ax:bx
;
_DevHlp_VirtToPhys	proc near
	public	_DevHlp_VirtToPhys

	mov	bx, ds
	mov	ax, ds
	mov	cx, 4
	shl	bx, cl
	mov	cx, 12
	shr	ax, cl
	add	bx, si
	adc	ax, 0
	clc
	ret
_DevHlp_VirtToPhys	endp

;*****************************************************************************
;
;	_DevHlp_SetIRQ - chain off the IRQ if sharing is indicated, else just
;		set it if 0. if not 0, then return CY set.
	saveIrq	equ	word ptr [bp-2]
_DevHlp_SetIRQ	proc near
	public	_DevHlp_SetIRQ
	push	bp
	mov	bp, sp
	sub	sp, 2

	push	si
	push	di
	disableIntr

	; save the IRQ

	mov	saveIrq, bx

	; set default interrupt vector byte offset

	mov	cx, _intBaseVec

	xor	bh, bh
	mov	di, bx
	mov	si, bx

	; get IRQ table pointer

	shl	si, 1
	shl	si, 1
	add	si, offset _IRQTab

	; is the IRQ in the master PIC?

	cmp	di,7
	jbe	inita_based

	; no, it's on the slave

	mov	cx,_altIntBaseVec
	sub	di,8

inita_based:

	shl	di,1
	shl	di,1					; INT_BASE + (int_level*4)
	add	di,cx
	xor	cx, cx
	mov	es, cx

	; if already set, then error.

	mov	cx, word ptr [si]
	or	cx, word ptr [si+2]
	jnz	_SetIRQErr

	push	word ptr es:[di+2]
	push	word ptr es:[di]
	pop	word ptr [si]
	pop	word ptr [si+2]

_setIVEC:

	mov	word ptr es:[di], ax
	mov	word ptr es:[di+2], cs

	push	saveIrq
	call	__unMaskIntr
	add	sp, 2

_xSetIRQ:

	restoreIntr
	clc

	pop	di
	pop	si
	mov	sp, bp
	pop	bp
	ret

_SetIRQErr:

	restoreIntr
	stc
	jmp	short _xSetIRQ

_DevHlp_SetIRQ	endp

;*****************************************************************************
;
;	_DevHlp_UnSetIRQ - restore the interrupt vector
;
_DevHlp_UnSetIRQ	proc near
	public	_DevHlp_UnSetIRQ
	push	bp
	mov	bp, sp
	sub	sp, 2

	push	si
	push	di

	; save the IRQ

	mov	saveIrq, bx

	; set default interrupt vector byte offset

	mov	cx, _intBaseVec

	xor	bh, bh
	mov	di, bx
	mov	si, bx

	; get IRQ table pointer

	shl	si, 1
	shl	si, 1
	add	si, offset _IRQTab

	; is the IRQ in the master PIC?

	cmp	di,7
	jbe	initaBased

	; no, it's on the slave

	mov	cx,_altIntBaseVec
	sub	di,8

initaBased:

	shl	di,1
	shl	di,1					; INT_BASE + (int_level*4)
	add	di,cx
	xor	cx, cx
	mov	es, cx

	; restore the original IRQ vector

	disableIntr
	push	word ptr [si+2]
	push	word ptr [si]
	pop	word ptr es:[di]
	pop	word ptr es:[di+2]
	mov	word ptr [si], 0
	mov	word ptr [si+2], 0

	restoreIntr
	clc

	pop	di
	pop	si
	mov	sp, bp
	pop	bp
	ret

_DevHlp_UnSetIRQ	endp

;*****************************************************************************
;
;	__MaskIntr - mask an interrupt level
;	
;	void _MaskIntr( BYTE irq )
;
	irqLevel	equ byte ptr [bp+4]
__maskIntr	proc near
	public	__maskIntr
	push	bp
	mov	bp, sp

	; check the IRQ level. if 7 or less, then it is a master interrupt 
	; controller level.

	mov	cl, irqLevel
	cmp	cl, 7
	jbe	_master

	; isolate the slave IRQ and load the port address

	sub	cl, 8
	mov	dx, _int2IMR
	jmp	short _maskIRQ

_master:

	; load the port address for the master

	mov	dx, _int1IMR

_maskIRQ:

	; construct the mask

	and	cl, 7H
	mov	bl, 1
	shl	bl, cl

	; get the previous mask

	disableIntr
	in	al, dx

	; OR in the mask bit

	or	al, bl

	; put it back out to the PIC

	out	dx, al
	restoreIntr

	mov	sp, bp
	pop	bp
	ret
__maskIntr	endp

;*****************************************************************************
;
;	__unMaskIntr - unmask an interrupt level
;	
;	void _UnMaskIntr( BYTE irq )
;
__unMaskIntr	proc near
	public	__unMaskIntr
	push	bp
	mov	bp, sp

	; check the IRQ level. if 7 or less, then it is a master interrupt 
	; controller level.

	mov	cl, irqLevel
	cmp	cl, 7
	jbe	_umaster

	; isolate the slave IRQ and load the port address

	sub	cl, 8
	mov	dx, _int2IMR
	jmp	short _umaskIRQ

_umaster:

	; load the port address for the master

	mov	dx, _int1IMR

_umaskIRQ:

	; construct the mask

	and	cl, 7H
	mov	bl, 1
	shl	bl, cl
	not	bl

	; get the previous mask

	disableIntr
	in	al, dx

	; AND the mask bit

	and	al, bl

	; put it back out to the PIC

	out	dx, al
	restoreIntr

	mov	sp, bp
	pop	bp
	ret
__unMaskIntr	endp

;*****************************************************************************
;
;	_DevHlp_AllocPhys - allocate physical memory at the start of the init data 
;		segment.
;
_DevHlp_AllocPhys	proc near
	public	_DevHlp_AllocPhys

	push	si

	; find the end of the current data (break address)

	mov	cx, bx

	; compute the break address

	call	__startInitData
	mov	si, ax
	push	ds
	push	cx
	call	_DevHlp_VirtToPhys
	pop	cx
	pop	ds
	add	bx, word ptr _DataEndOffset
	adc	ax, 0
	add	word ptr _DataEndOffset, cx
	pop	si
	clc
	ret
_DevHlp_AllocPhys	endp

;*****************************************************************************
;
; Function: DOSOPEN
;
; Inputs:
;	see the description of side effects
;
; Ouputs:
;	DOS file handle
;
; Side Effects:
;	This function simulates the OS/2 FAMAPI DOSOPEN. see the OS/2 documentation
;	for more details.
;
;	unsigned far pascal DOSOPEN (
;		char far *,				/* File path name */
;		unsigned far *,			/* New file's handle */
;		unsigned far *,			/* Action taken - 1=file existed, */
;								/* 2=file was created */
;		unsigned long,			/* File primary allocation */
;		unsigned,				/* File attributes */
;		unsigned,				/* Open function type */
;		unsigned,				/* Open mode of the file */
;		unsigned long );		/* Reserved (must be zero) */
;
dosOpenRsvd	equ	dword ptr [bp+6]
dosOpenMode	equ	word ptr [bp+10]
dosOpenType	equ	word ptr [bp+12]
dosOpenAttr	equ	word ptr [bp+14]
dosOpenPrim	equ	word ptr [bp+16]
dosOpenAct	equ	dword ptr [bp+20]
dosOpenHand	equ	dword ptr [bp+24]
dosOpenName	equ	dword ptr [bp+28]

DOSOPEN	proc far
	public	DOSOPEN
	push	bp
	mov	bp, sp
	push	si
	push	di
	push	ds

	mov	ax, dosOpenMode
	mov	ah, 3dH
	lds	dx, dosOpenName
	int	21H
	jc	_DOSOPENErr

	lds	si,dosOpenAct
	push	dosOpenType
	pop	word ptr ds:[si]
	lds	si, dosOpenHand
	mov	word ptr [si], ax
	xor	ax, ax

_xDOSOPEN:
	pop	ds
	pop	di
	pop	si
	mov	sp, bp
	pop	bp
	ret	24

_DOSOPENErr:
	mov	ax, -1
	jmp	short _xDOSOPEN

DOSOPEN	endp

;*****************************************************************************
;
; Function: DOSCLOSE
;
; Inputs:
;	see description of Side Effects
;
; Outputs:
;	returns 0 if success, non-zero otherwise.
;
; Side Effects:
;	This function simulates the OS/2 FAMAPI DOSCLOSE function.
;
;	unsigned far pascal DOSCLOSE (unsigned );			/* File handle */
;

dosCloseHand	equ word ptr [bp+6]
DOSCLOSE	proc far
	public	DOSCLOSE
	push	bp
	mov	bp, sp
	push	si
	push	di
	push	ds

	mov	bx, dosCloseHand
	mov	ah, 3eH
	int	21H
	jc	_DOSCLOSE_Err
	xor	ax, ax

_xDOSCLOSE:

	pop	ds
	pop	di
	pop	si
	mov	sp, bp
	pop	bp
	ret	2

_DOSCLOSE_Err:
	mov	ax, 1
	jmp	short _xDOSCLOSE

DOSCLOSE	endp

;*****************************************************************************
;
; Function: DOSREAD
;
; Inputs:
;	See the description of Side Effects
;
; Outputs:
;	returns 0 if success, non-zero otherwise.
;
; Side Effects:
;	This function simulates the OS/2 FAMAPI DOSREAD function.
;
;	unsigned far pascal DOSREAD (
;			unsigned,			/* File handle */
;			char far *,			/* Address of user buffer */
;			unsigned,			/* Buffer length */
;			unsigned far * );	/* Bytes read */
;
dosReadRead	equ	dword	ptr [bp+6]
dosReadLen	equ	word	ptr [bp+10]
dosReadAddr	equ	dword	ptr [bp+12]
dosReadHand	equ	word	ptr [bp+16]
DOSREAD	proc far
	public	DOSREAD
	push	bp
	mov	bp, sp
	push	ds
	push	di
	push	si

	mov	bx, dosReadHand
	mov	cx, dosReadLen
	lds	dx, dosReadAddr
	mov	ah, 3fH
	int	21H
	jc	_DOSREAD_Err

	lds	bx, dosReadRead
	mov	word ptr ds:[bx], ax
	xor	ax, ax

_xDOSREAD:
	pop	si
	pop	di
	pop	ds
	mov	sp, bp
	pop	bp
	ret	12

_DOSREAD_Err:
	mov	ax, 1
	jmp	short _xDOSREAD
	
DOSREAD	endp

;*****************************************************************************
;
;
; Function: DOSWRITE
;
; Inputs:
;	See the description of Side Effects
;
; Outputs:
;	returns 0 if success, non-zero otherwise.
;
; Side Effects:
;	This function simulates the OS/2 FAMAPI DOSWRITE function.
;
;	unsigned far pascal DOSWRITE (
;			unsigned,			/* File handle */
;			char far *,			/* Address of user buffer */
;			unsigned,			/* Buffer length */
;			unsigned far * );	/* Bytes written */
;
dosWriteWritten	equ	dword ptr [bp+6]
dosWriteLength	equ	word  ptr [bp+10]
dosWriteAddr	equ	dword ptr [bp+12]
dosWriteHand	equ	word  ptr [bp+16]
DOSWRITE	proc far
	public	DOSWRITE
	push	bp
	mov	bp, sp
	push	si
	push	di
	push	ds

	mov	bx, dosWriteHand
	lds	dx, dosWriteAddr
	mov	cx, dosWriteLength
	mov	ah, 40H
	int	21H
	jc	_DOSWRITE_Err

	lds	bx, dosWriteWritten
	mov	word ptr ds:[bx], ax
	xor	ax, ax

_xDOSWRITE:

	pop	ds
	pop	di
	pop	si
	mov	sp, bp
	pop	bp
	ret	12

_DOSWRITE_Err:
	mov	ax, 1
	jmp	short _xDOSWRITE

DOSWRITE	endp

;*****************************************************************************
;
;
; Function: DOSDEVIOCTL
;
; Inputs:
;	See the description of Side Effects
;
; Outputs:
;	returns 0 if success, non-zero otherwise.
;
; Side Effects:
;	This function simulates the OS/2 FAMAPI DOSDEVIOCTL function.
;
;	unsigned far pascal DOSDEVIOCTL (
;			char far *,			/* Data area */
;			char far *,			/* Command-specific argument list */
;			unsigned,			/* Device-specific function code */
;			unsigned,			/* request block size */
;			unsigned );			/* Device handle returned by Open */
;
dosIoctlHand	equ	word ptr [bp+6]
dosIoctlCat		equ	word ptr [bp+8]
dosIoctlCode	equ	byte ptr [bp+10]
dosIoctlArgp	equ	dword ptr [bp+12]
dosIoctlDatap	equ	dword ptr [bp+16]
DOSDEVIOCTL	proc far
	public	DOSDEVIOCTL
	push	bp
	mov	bp, sp
	push	si
	push	di
	push	ds

	mov	ax, 4402H
	mov	bx, dosIoctlHand
	mov	cx, dosIoctlCat
	lds	dx, dosIoctlArgp
	int	21H
	jc	_DOSDEVIOCTL_Err
	xor	ax, ax

_xDOSDEVIOCTL:

	pop	ds
	pop	di
	pop	si
	mov	sp, bp
	pop	bp
	ret	14

_DOSDEVIOCTL_Err:

	mov	ax, 1
	jmp	short _xDOSDEVIOCTL

DOSDEVIOCTL	endp


endif

_TEXT	ends

end
