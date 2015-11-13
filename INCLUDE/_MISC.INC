;
; $Header:   \bim16\_misc.inc  2-2-93  jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
; 	This file describes misc functions provided in _misc.asm.

;
; processor types.
;
SIXTEEN_BIT		equ	0001H
PROC_TYPE_MASK	equ	0030H

PROC_8086		equ	0000H OR SIXTEEN_BIT
PROC_80286		equ	0010H OR SIXTEEN_BIT
PROC_80386		equ	0020H
PROC_80486		equ	0040H

;
; debug definitions. if debug is not predefined as a MACRO, then it will
; be instantiated as a variable.
;
DBG_VERBOSE		equ	1
DBG_TERSE		equ	2
DBG_SPECIAL		equ	4

;
; debug Putchar MACRO
;
PUTCHAR	MACRO	letter
	LOCAL	noDebug
ifndef debug
	cmp		_debug,DBG_SPECIAL
	jne		noDebug
	push	ax
	mov		al, letter
	push	ax
	call 	_Putchar
	add		sp, 2
	pop		ax
endif
noDebug:
ENDM

ifndef debug
	EXTRN	_debug:WORD
	EXTRN	_Putchar:NEAR
endif

;
; The BP offset to the first parameter on the stack is always in the same
; place for a near call as well as for a far call.
;
NEAR_BP_OFFSET	equ	4
FAR_BP_OFFSET	equ	6

;
; define entry and exit macros to take best advantage of the CPU
;
ifdef OS2

ENTRY macro numBytes
IFB <numBytes>
	enter	0,0
ELSE
	enter	numBytes,0
ENDIF
endm

else

ENTRY	macro numBytes
	push	bp
	mov		bp, sp
IFNB	<numBytes>
	sub		sp, numBytes
ENDIF
endm

endif

EXIT macro
ifdef OS2
	leave
else
	mov	sp, bp
	pop	bp
endif
endm

;
; Generic PANIC MACRO. The input 'line' is meant to be used as a positional
; indicator in the source code.
;
PANIC	macro	line
ifndef NOPANIC
	push	ax
	mov	ax, line
	int	3
	pop	ax
endif
endm

ifdef DOS

;/* Defines for device open/close. 	*/

SIZE_0				equ 0H
NORM_FILE			equ 0H
EXIST_ONLY			equ 0001H
WO_DENY_RW_PRIV	 	equ 0091H
RW_DENY_NONE_PRIV	equ 00C2H
READ_ONLY			equ 0040H
RESERVED_DW_0		equ 0H

;
; convert a real mode segment:offset to a 32 bit physical address.
; ENTRY: dx - seg
;		 si - offset
; EXIT:	 dx:ax
;		CX trashed
SEGOFF_PHYS MACRO
	mov	ax, dx
	mov	cx, 4
	shl	ax, cl
	mov	cx, 12
	shr	dx, cl
	add	ax, si
	adc	dx, 0
ENDM
endif

;
; DOS stack setup MACRO. The input 'buffer' must be a memory variable that
; references a reasonable sized buffer (256 bytes at least). 'size' is the
; amount of memory that 'buffer' defines. This MACRO is NOT reentrant if
; the same 'buffer' memory variable is used.
;
SP_SAVE	equ	0
SS_SAVE	equ	2
GENERIC_STACK_SETUP MACRO buffer, size
ifdef DOS

	;
	; save trashed registers
	;
	push	ds
	push	ax
    mov		ax,DGROUP	; make local data addressable
	mov		ds,ax
	ASSUME	ds:DGROUP

	;
	; switch to the specified stack
	;
	mov		buffer+SP_SAVE, sp
	mov		buffer+SS_SAVE, ss
	mov		ax, ds
	mov		ss, ax
	mov		sp, offset buffer

	;
	; set SP to the top of the new stack
	;
	add		sp, size-2
	ASSUME	ss:DGROUP

	;
	; save the rest of the world
	;
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	push	es
	push	bp

	;
	; create a stack frame
	;
	mov		bp, sp

endif
ENDM

	;
	; Restore the stack to the values before GENERIC_STACK_SETUP was used.
	; This assumes a 1-1 usage between setup and restore.
	;
GENERIC_STACK_RESTORE	MACRO buffer
ifdef DOS

	; restore the world from the current stack

	pop	bp
	pop	es
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx

	; restore the stack to the original SS and SP

	mov	ss, buffer+SS_SAVE 
	mov	sp, buffer+SP_SAVE

	;
	; restore trashed registers
	;
	pop	ax
	pop	ds

endif
ENDM

STACK_EXTRN	MACRO
ifdef DOS
	extrn	stackSetupBuffer:WORD
endif
ENDM

STACK_SETUP_SIZE	equ	512
STACK_SETUP	MACRO
	GENERIC_STACK_SETUP	stackSetupBuffer, STACK_SETUP_SIZE
ENDM

STACK_RESTORE MACRO
	GENERIC_STACK_RESTORE stackSetupBuffer
ENDM

DOS_STACK_SIZE		equ	512
DOS_STACK_EXTRN	MACRO
ifdef DOS
	extrn	dosStackSetupBuffer:WORD
endif
ENDM

DOS_STACK_SETUP	MACRO
	GENERIC_STACK_SETUP	dosStackSetupBuffer, DOS_STACK_SIZE
ENDM

DOS_STACK_RESTORE	MACRO
	GENERIC_STACK_RESTORE	dosStackSetupBuffer
ENDM

;-----------------------------------------------------------------------------
; bcopy - copies n bytes from src to dest. this is an assembly function only.
;
; ENTRY:
;		ds:[si]	- source ptr
;		es:[di]	- destination ptr
;		cx		- byte count
;		dl		- low bit indicates indicates 386 (0) or 286 (1)
;
; EXIT:
;		cx		- 0
;		src		- end of source data
;		dest	- end of destionation data
;		dl		- trashed

bcopy	MACRO	srcSeg
	LOCAL	bcopyDone
	LOCAL	moveWords
	LOCAL	moveBytes

	; bcopy always copies to an increasing address

	cld

	; if this is a 386, then use DWORD moves

	test	dl, SIXTEEN_BIT
	jnz		moveWords 

	; use extended 386 instructions

	.386p

	; compute the leftover byte count and leave it in DX, then compute the 
	; DWORD count and leave it in CX.

	mov		dx, cx
	and		dx, 3
	sar		cx, 2

	; bail out if 0

	jcxz	moveBytes

	; move DWORDS

	rep		movs	dword ptr es:[di], dword ptr srcSeg:[si]

	; get the leftover bytes

	jmp		short moveBytes

	; switch back to regular 16 bit mode

ifdef OS2
	.286p
else
	.8086
endif

moveWords:

	; compute the leftover byte count and leave it in DX, then compute the
	; WORD count and leave it in CX.

	mov		dx, cx
	and		dx, 1
	sar		cx, 1

	; bail out if 0

	jcxz	moveBytes

	; let her rock

	rep		movs	word ptr es:[di], word ptr srcSeg:[si]

moveBytes:

	; restore CX and move the last bytes

	mov		cx, dx

	; bail out if 0

	jcxz	bcopyDone

	; move the last bytes

	rep		movs byte ptr es:[di], byte ptr srcSeg:[si]

bcopyDone:

ENDM

;-----------------------------------------------------------------------------
; bcopy32 - copies n bytes from src to dest. this is an assembly function 
;	only. it also assumes at least a 80386. this macro could be used to move
;	data in a flat 4 Gig address space.
;
; ENTRY:
;		srcSeg:[esi]	- source ptr
;		es:[edi]		- destination ptr
;		ecx				- byte count
;
; EXIT:
;		srcSeg:[esi]	- end of source data
;		es:[edi]		- end of destionation data
;		ecx				- 0

bcopy32	MACRO	srcSeg
	LOCAL	evenDwordCnt
	LOCAL	bcopyDone
	LOCAL	moveBytes

	; bcopy32 always copies to an increasing address

	cld

	; save ECX, and get the number of DWORDs to move

	push	ecx
	sar		ecx, 2

	; bail out if numBytes is 0

	jecxz	moveBytes

	; move DWORDS

	rep		movs	dword ptr es:[edi], dword ptr srcSeg:[esi]

moveBytes:

	; restore ECX and strip off the number of bytes to move

	pop		ecx
	and		ecx, 3

	; bail out if 0

	jecxz	bcopyDone

	; move DWORDS

	rep		movs	byte ptr es:[edi], byte ptr srcSeg:[esi]

bcopyDone:

ENDM



