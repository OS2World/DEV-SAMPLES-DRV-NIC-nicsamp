;
; $Header:   \ibm16\layout.inc   2-2-93  jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;       These are useful macros for the ASM
;
;.model small
include segments.inc
include devhlp.inc

MAKEPUB	equ	1

PANIC	macro	line
	push	ax
	mov	ax, line
	int	3
	pop	ax
endm

ifdef USE
.begin	macro	name
	IFDEF	MAKEPUB
	public	_&name
	ENDIF
_&name	proc	near
	push	bp
	mov	bp,sp
	endm
	
.end	macro	name
	pop	bp
	ret
_&name	endp
	endm
endif

loadmyds macro
	mov	ax,seg DGROUP
	mov	ds,ax
	endm

doffset macro	reg,item
	mov	reg, OFFSET DGROUP: item
	endm

coffset macro	reg,item
	mov	reg, OFFSET _TEXT: item
	endm

.ecode macro
	_TEXT ends
	endm

.edata macro
	_DATA ends
	endm


extern	macro	type,name
	IFIDN	<type>,<int>
EXTRN	_&name:word
	ENDIF
	IFIDN	<type>,<char>
EXTRN	_&name:byte
	ENDIF
	ENDM

