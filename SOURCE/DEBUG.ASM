;
; $Header:   \ibm16\debug.asm 2-2-93 jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;       These are the IBM token-ring debug print routines.
;	Debug Putchar for ring 3 debug printf's.

include segments.inc

;***	Pcall - Call local or external procedure.
;

Pcall	MACRO	name
	IF2
	IFNDEF	name
	EXTRN	name:NEAR
	ENDIF
	ENDIF
	call	name
	ENDM

;***	DefProc - Define subroutine entrypoint.
;

DefProc MACRO	name, scope
name	PROC	NEAR
	IFIDN	<scope>,<PUBLIC>
	PUBLIC	name
	ENDIF
	ENDM

;***	EndProc - Mark end of subroutine definition.
;

EndProc MACRO	name
	ret			; just in case he doesn't bother.
name	ENDP
	ENDM

IODELAY MACRO
	jmp	$+2
	jmp	$+2
	jmp	$+2
	jmp	$+2
	ENDM

COM1_PORT	equ	03f8H
COM2_PORT	equ	02f8H

COM_DAT         equ	00H
COM_IEN         equ	01H		; interrupt enable
COM_IER         equ	02H		; interrupt ID
COM_LCR         equ	03H		; line control registers
COM_MCR         equ	04H		; modem control register
COM_LSR         equ	05H		; line status register
COM_MSR         equ	06H		; modem status regiser
COM_DLL         equ	00H		; divisor latch least sig
COM_DLM         equ	01H		; divisor latch most sig

NMI_STATUS_PORT equ	61h		; IBM AT STATUS PORT
NMI_PORT	equ	70h		; IBM AT NMI disable/enable port

KEYBPORT	equ	64h
RESET		equ	0F0h		; Magic reset value (i.e. bit 0 == 0)

KBD_STATUS_PORT equ	64h		; 8042 com port
PORT_A		equ	60h		; 8042 data port

BUF_FULL_IN	equ	2		; 8042 input buffer busy bit
BUF_FULL_OUT	equ	1		; 8042 output buffer busy bit

CLOCK_RATEL	equ	1		; LSW of clock rate
CLOCK_RATEH	equ	0C200h		; MSW of clock rate
LC_DLAB         equ	10000000b	; divisor latch access bit

;**	Valid baud rates
;
BD_150		equ	150
BD_300		equ	300
BD_600		equ	600
BD_1200         equ	1200
BD_2400         equ	2400
BD_4800         equ	4800
BD_9600         equ	9600
BD_19200	equ	19200

_DATA	segment
Suspend dw	0		; !=0 means ^S in effect, use ^Q to resume.
ComInit dw	0		; !=0 means com port initialize


;	The following variable is used to auto-sense the COM port address
;	for debug messages.  If it is zero, then no messages will be written.
;	Otherwise, it will be equal to COM1_PORT or COM2_PORT.

ComBase		dw	0		; the base of the COM port installed.
c_InitDebugPort	dw	1

_DATA	ends

_TEXT    SEGMENT


;***	AutoSensePort - Determine COM port installed.
;
;1.	Functional description.
;	This routine is called to determine the correct COM port to write
;	messages to.  It sets ComBase to the correct PORT FWA.
;
;2.	Modification history.
;	S. E. Jones	89/02/09.	Original for 1.0.
;	L. E. Hattery	89/03/07.	Added IFDEF COM1_ONLY to force COM1.
;
;3.	WARNINGS.
;
;4.	ENTRY.
;	None.
;
;5.	EXIT.
;	None.
;
;6.	USES.
;	Flags.

	ASSUME	CS:_TEXT, DS:DGROUP
DefProc AutoSensePort, PUBLIC
	push	ax
	push	dx

;	If there is a ComBase defined already, then exit right away.

	cmp	ComBase, 0
	jne	AutoSensePort_Exit	; we have one. don't reset.

;	Assume there is no current COM port.  Try COM2 first, then COM1.
;	We do this by reading the input character on the port, and if it
;	is FFh, we assume the port is not there.

IFNDEF COM1_ONLY
	mov	dx, COM2_PORT		; (DX) = COM2 base address.
	in	al, dx			; (AL) = current char from COM port.
	IODELAY
	cmp	al, 0ffh		; is the port there?
	je	AutoSensePort_TryCom1	; nope. try com1.
	mov	ComBase, COM2_PORT
	jmp	short AutoSensePort_Exit
ENDIF ; COM1_ONLY

;	check for a COM1 port.

AutoSensePort_TryCom1:
	mov	dx, COM1_PORT		; (DX) = COM1 base address.
	in	al, dx			; (AL) = current char from COM port.
	IODELAY
	cmp	al, 0ffh		; is the port there?
	je	AutoSensePort_Exit	; nope.  still nothing attached.
	mov	ComBase, COM1_PORT
AutoSensePort_Exit:
	mov	ComInit, 1		; we've initialized this.
	pop	dx
	pop	ax
EndProc AutoSensePort

;***	FlowControl - Do Flow Control (^S ^Q) Processing on Debug Terminal.
;
;1.	Functional description.
;	This routine suspends output if Suspend is set or if there is a ^S
;	submitted from the keyboard.  Output is resumed when a ^Q is received.
;	All other characters are dumped.
;
;2.	Modification history.
;	S. E. Jones	88/08/09.	Original for 1.0.
;
;3.	WARNINGS.
;
;4.	ENTRY.
;	None.
;
;5.	EXIT.
;	None.
;
;6.	USES.
;	Flags.

	ASSUME	CS:_TEXT, DS:DGROUP
DefProc FlowControl, PUBLIC
	push	ax		; save caller's regs.
	push	dx

FlowControl_Loop:
	mov	dx, ComBase
	add	dx, COM_LSR	; (DX) = FWA, COM 8250 line status register.
	in	al, dx
	IODELAY
	and	al, 1			; is there a char available?
	jz	FlowControl_CheckFlag	; nope. just check suspend flag.

;	Get incoming character.

	mov	dx, ComBase	; (DX) = FWA, COM1/COM2 8250 data port.
	in	al, dx
	IODELAY
	cmp	al, 19		; is this a ^S?
	jne	FlowControl_1	; nope.
	mov	Suspend, 1	; indicate we're suspended.
	jmp	FlowControl_Loop; wait for ^Q.

FlowControl_1:
	cmp	al, 17		; is this a ^Q?
	jne	FlowControl_Loop; nope. just eat the char and retry.
	mov	Suspend, 0	; we're not suspended any more.
	jmp	FlowControl_Loop; try to get more chars (probably won't.)

;	Now check Suspend flag.  If we're suspended, we just have to wait
;	until he types ^Q.

FlowControl_CheckFlag:
	cmp	Suspend, 0	; are we suspended?
	jne	FlowControl_Loop; yes. wait for ^Q.

FlowControl_Exit:
	pop	dx		; restore saved DX.
	pop	ax		; restore saved AX.
EndProc FlowControl

;***	Putchar - Display character on debugging terminal.
;
;1.	Functional description.
;	This routine displays the contents of [bp+4] as a character on the
;	remote debugging console.
;
;2.	Modification history.
;	S. E. Jones	88/08/08.	Original for 1.0.
;	S. E. Jones	89/02/16.	Changed Key_InitDebugPort to c_....
;	S. E. Jones	89/03/01.	Added finite loop to stop hung debug ports.
;	L. E. Hattery	89/03/09.	Fixed to push CX.
;
;3.	WARNINGS.
;
;4.	ENTRY.
;	AL	= char to display.
;
;5.	EXIT.
;	None.
;
;6.	USES.
;	Flags.

	outputChar	equ byte ptr [bp+4]

	ASSUME	CS:_TEXT, DS:DGROUP
DefProc _Putchar, PUBLIC
	push	bp
	mov	bp, sp
	push	dx			; we trash DX & CX
	push	cx
	push	ax			; save character to print out.
	xor	ah, ah
	mov	al, outputChar

	cmp	ComInit, 0		; has com port been initialized?
	jne	Putchar_loop		; yes, just do it.
	Pcall	AutoSensePort		; we need to see which port to init.
	cmp	ComBase, 0		; is it still undefined?
	jne	Putchar_Init		; no, so do init now.

Putchar_Disabled:
	pop	ax			; it is still undefined, so get out.
	pop	cx
	pop	dx
	jmp	short Putchar_Exit

Putchar_Init:
	IFDEF	DOS

;	If the DOS kernel debugger is running, then the baud, parity, and stop
;	bits have been setup already.  So we can look at the PROTOCOL.INI
;	param below to see if he wants the debug port initialized.

	cmp	c_InitDebugPort, 0	; does he want port initialization?
	je	Putchar_initialized	; no, just skip this part.

	mov	ax, 00e3h		; 9600 baud, no parity, 1 stop, 8 bits.
	mov	dx, 0			; default is COM1.
	cmp	ComBase, COM1_PORT	; is it COM1?
	je	@f			; if so, skip next line.
	inc	dx			; (DX)=1; meaning COM2.
@@:
	int	14h			; init com port.
	ENDIF	; (DOS)

Putchar_initialized:
	mov	ComInit, 1		; com port initialized.

Putchar_loop:
	cmp	ComBase, 0		; is COM port disabled?
	je	Putchar_Disabled	; yes.
	mov	cx, 1000		; try 1000 times.
Putchar_loop1:
	push	cx
	Pcall	FlowControl
	mov	dx, ComBase
	add	dx, COM_LSR		; (DX) = FWA, COM1/COM2 8250 line status reg.
	in	al, dx
	IODELAY
	pop	cx
	test	al, 020h		; is transmit buffer empty?
	jnz	Putchar_TBE		; yes.
	loop	Putchar_loop1
	mov	ComBase, 0		; disable COM port.
	jmp	Putchar_Disabled	; never write again.

;	Transmit buffer is empty.  Put the character to the console.

Putchar_TBE:
	pop	ax			; the char to write.
	mov	dx, ComBase
	add	dx, COM_DAT		; (DX) = FWA, 8250 data port.
	out	dx, al
	IODELAY
	pop	cx			; restore saved CX & DX.
	pop	dx
Putchar_Exit:
	mov	sp, bp
	pop	bp
EndProc _Putchar

_TEXT ENDS

	END
