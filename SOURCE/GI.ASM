;
; $Header:   \ibm16\gi.asm  2-2-93  jess fahland $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; Description:
;       Generic interface routine.
;

include layout.inc
include os2def.inc
include bsedos.inc
include devhdr.inc
include ndis.inc
;include gi.inc
include _uprim.inc
include _buff.inc
include _hwbuff.inc
include _misc.inc

DMA_STACK_SIZE	equ	64
DMA_STACK_SETUP	MACRO
	GENERIC_STACK_SETUP	dmaStackBuffer, DMA_STACK_SIZE
ENDM

DMA_STACK_RESTORE	MACRO
	GENERIC_STACK_RESTORE dmaStackBuffer
ENDM

	AT_PLATFORM	equ	1

	extrn	_DevHdr:word
	extrn	_DevHlp:DWORD
        extrn   _MMIOAddress:dword
        extrn   _RAMAddress:dword
        extrn   _SRB_Offset:word
        extrn   _SSB_Offset:word
        extrn   _ARB_Offset:word
        extrn   _ASB_Offset:word
        extrn   _Init_Flag:word
        extrn   _IRQEnableReg:word
        extrn   _gi_state:byte
        extrn   _transmit_request:word

	extrn   _putChar:near
zap	macro	n
	push	ax
	push	dx
	mov	dx,300H + n
	in	al,dx
	pop	dx
	pop	ax
endm

	IFDEF	MAKEPUB

	public	_gi_nullints
	public	forkstate
	public	_gi_scb
	public	_gi_ssb
	public	adap_int
	public	sifdata,sifdatai,sifadr,sifint,sifinth,conbuf,irqarm
	public	shrmem
	public	old_con,new_con
	public	old_vec,savds
	public	conval
	public	_sif_int
	public	_dma_int
	public	is_sif
	public	all_done
	public	int_exit
	public	adapter_check
	public	ring_status
	public	command_status
	public	receive_status
	public	transmit_status

	ENDIF

	public	_SSflag
	public	_sysconfig
	public	_systype
	public	_gi_nullints
	public	forkstate
	public	_gi_scb
	public	_gi_ssb
	public	adap_int
	public	sifdata,sifdatai,sifadr,sifint,sifinth,conbuf,irqarm
	public	shrmem
	public	old_con,new_con
	public	old_vec,savds
	public	conval
	public	_sif_int
	public	_dma_int
	public	is_sif
	public	all_done
	public	int_exit
	public	adapter_check
	public	ring_status
	public	command_status
	public	receive_status
	public	transmit_status
	public 	_baseDgroup
        public  _GDTSelector

_SRBSTRUC struc

        SRB_Offset      dw      ?
        Page_Select     db      ?
                        db      ?
_SRBSTRUC ends

sin	macro	a,b
	in	a,b
	jmp	$+2
	endm
	
sout	macro	a,b
	out	a,b
	jmp	$+2
	endm

wout	macro
	out	dx,al
	inc	dx
	xchg	al,ah
	out 	dx,al
	dec	dx
	xchg	al,ah
	endm

win	macro
	in	al,dx
	inc	dx
	xchg	al,ah
	in	al,dx
	xchg	al,ah
	dec	dx
	endm


RRREVEN         equ     01E00h
RRRODD          equ     01E01h
WRBREVEN        equ     01E02h
WRBRODD         equ     01E03h
WWOREVEN        equ     01E04h
WWORODD         equ     01E05h
WWCREVEN        equ     01E06h
WWCRODD         equ     01E07h
ISRPEVEN        equ     01E08h
ISRPODD         equ     01E09h
ISRPODD_RESET   equ     01E29h
ISRPODD_SET     equ     01E49h
ISRAEVEN        equ     01E0Ah
ISRAODD         equ     01E0Bh
ISRAODD_RESET   equ     01E2Bh
ISRAODD_SET     equ     01E4Bh
TCREVEN         equ     01E0Ch
TCRODD          equ     01E0Dh
TVREVEN         equ     01E0Eh
TVRODD          equ     01E0Fh
SRPREVEN        equ     01E18h
SRPRODD         equ     01E19h

IMR		equ	21H
IACK	equ	20H
IEOI	equ	20H

IMR2	equ	0A1H
IACK2	equ	0A0H

FORKREQ 	equ	1
FORKED	equ	2

SDMA	equ	80H
SINTR	equ	40H
INTEN	equ	20H
SDDIR	equ	10H
BLKEN	equ	08H
DMAEN	equ	08H
SFTRST	equ	04H

; Individual bits in SIF_INT control/status register

IR_SRESET	equ 0ff00H
IR_INT_ADAP	equ 08000H
IR_ADAP_RST	equ 04000H
IR_SSB_CLR	equ 02000H
IR_EXECUTE	equ 01000H
IR_SCB_REQ	equ 00800H
IR_RCV_CONT	equ 00400H
IR_RCV_VALD	equ 00200H
IR_XMT_VALD	equ 00100H
IR_INT_SYS	equ 00080H
IR_INIT 	equ 00040H
IR_TEST 	equ 00020H
IR_ERROR	 equ 00010H


; Special pseudo-scb for NDIS

SCB_RCV_PENDING	equ	-2

SCB_INTCHECK	equ	-1
SCB_ADAPCHECK	equ	0
SCB_RINGSTATUS	equ	1
SCB_REJECT	equ	2
SCB_OPEN	equ	3
SCB_TRANSMIT	equ	4
SCB_RECEIVE	equ	6
SCB_CLOSE	equ	7
SCB_SETGROUP	equ	8
SCB_SETFUNC	equ	9
SCB_READLOG	equ	10
SCB_READADAP	equ	11
SCB_IMPLENABLE	equ	12

GIS_RESET	equ	0
GIS_INIT	equ	1
GIS_CLOSED	equ	2
GIS_OPEN	equ	3

; model bytes from PS/2 BIOS Tech Ref, System Identification
ATIDENTIFIER	equ	0FCH
PCIDENTIFIER	equ	0FFh
XTIDENTIFIER1	equ	0FBh
XTIDENTIFIER2	equ	0FEh

; from PS/2 BIOS Tech Ref for INT 15, Return System Config Params
SysConParms	struc
	SCLength	dw	?
	SCModel		db	?
	SCSubmodel	db	?
	SCBiosrev	db	?
	SCFeatures	db	?
	SCReserved	db	?,?,?,?
SysConParms	ends

; bits for 1st features byte of SysConParms
SC_MICROCHAN	equ	2
SC_TWOPICS	equ	40h

; bits of system status flag (_SSflag)

REAL_DMA		equ	1	; card does real DMA, i.e. not a p1340
TWO_PICS		equ	2	; machine has two PICs, i.e. AT bus or uchan
SHMEM_OK		equ	4	; OK to use shared mem on p1340, i.e.
					; machine is a slow PC or XT
CONBUF_EXISTS	equ	8	; OK to use CONBUF, i.e. not a p1840
EOI2		equ	10h	; interrupt is on 2nd PIC, must do 2 EOIs

_DATA segment

	STACK_EXTRN

	even
_gi_scb 	dw		3 dup (0)


; We allocate 5 words for the ssb, the first word is the command, the next
; four words are status parameters. The hardware only uses three words for 
; reporting status, the extra word allows us to treat the adapter check
; information (4 bytes) as a psuedo-ssb (command = 0).
;
	even
_gi_ssb 	dw		5 dup (0)


extrn	_ssb_ring:word
extrn	_ssb_tail:word
extrn	_ssb_end:word

extrn	_realTimeTicks:word
extrn	_receivePending:word
interruptHooked	db	0

forkstate db	0

_gi_nullints	db	0

even


adap_int dw	adapter_check, all_done, ring_status, scb_clear
	dw	command_status, receive_status, transmit_status

; I/O addresses

sifdata 	dw	?
sifdatai 	dw	?
sifadr	dw	?
sifint	dw	?
sifinth 	dw	?
conbuf	dw	?
irqarm	dw	?

_GDTSelector    dw      ?
; I/O addresses for psuedo-dma registers

dmadata 	dw	?
dmalen	dw	?
dmaseg	dw	?
dmaoff	dw	?

; shared memory offset and segment

shrmem	dd	?

; interrupt request line number

_SSflag	dw	0
irqbit	db	?
irqmask 	db	?
intnum	dw	?		; vector number
intLine	dw	?		; 8259 interrupt line (0-ffH)
dmanum	db	?
	even
imrport 	dw	?

old_con 	db	?
new_con 	db	?


	even
old_vec 	dw	2 dup (?)
savds	dw	?


conval	db	?
_irqSet	db	0

_baseDgroup	dw 	2 dup (?)

;
; keep SC_MICROCHAN bit here if PS/2, set by IsThisAp1840NIC
;
	public	_SCFeatures
_SCFeatures	db	?


;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        public  logbufftop, logbuffend, logptr
;
;        logbufftop      label   byte
;        db      8000 dup (0)
;        logbuffend      label   byte
;
;        logptr  dw      offset logbufftop

	_DATA ends

	_TEXT segment

	assume CS:CGROUP, DS:DGROUP

extrn	_executor:near
extrn	_indicationsComplete:near
extrn   _gReqComplete:near
extrn   _gi_abortall:near
extrn   _gi_exit:near


;------------------------------------------------------------------------------;
;	_en_sif -
;
;	description -
; 		void en_sif(ushort iobase,
;					void far *shmem, 
;					ushort irqline, 
;					ushort drqline)

iobase  =	4
shmem   =	6
irqline =	10
drqline =	12

;
;	inputs:
;
;	outputs:
;
;
_en_sif	proc near
	public	_en_sif

	ENTRY

	push	si
	push	di

	cmp	interruptHooked,0
	jz	X1
	jmp	NEAR PTR L4B
X1:
	mov	interruptHooked,1

; Initialize adapter register pointers

	mov	ax,iobase[bp]
	mov	sifdata,ax
	add	ax,2
	mov	sifdatai,ax
	add	ax,2
	mov	sifadr,ax
	add	ax,2
	mov	sifint,ax
	add	ax,1
	mov	sifinth,ax
	add	ax,1
	mov	dmadata,ax
	add	ax,1
	mov	conbuf,ax
	add	ax,1
	mov	dmalen,ax
	add	ax,2
	mov	dmaseg,ax
	add	ax,2
	mov	dmaoff,ax
	add	ax,1
	mov	irqarm,ax

; Initialize shared memory pointer (Segment == 0 indicates no shared mem)

	mov	ax,shmem[bp]
	mov	word ptr shrmem,ax
	mov	ax,shmem+2[bp]
	mov	word ptr shrmem+2,ax

	test	_SSflag, SHMEM_OK
	jnz	ShareOk
	mov	word ptr shrmem,0
	mov	word ptr shrmem+2,0

ShareOk:

; Set DMA Request Line index number (expects 5, 6, or 7)

	mov	ax,drqline[bp]
	sub	ax,4
	mov	dmanum,al

; get irq line number

	mov	bx,irqline[bp]
	test	_SSflag, TWO_PICS
	jz	L2

	; if 2 PICs, then interrupt 2 is routed into 9
	cmp	bx,2
	jnz	L2
	mov	bx,9

L2:
   
	mov	word ptr intLine, bx

	; get an unshareable interrupt.
	mov	bx, word ptr intLine
	xor	bh, bh						; cleanup parameter
	xor	dh, dh						; no interrupt sharing
	mov	ax, offset _TEXT:_sif_int
	test	_SSflag, REAL_DMA			; Are we doing "real" DMA?
	jnz	RDMA_int
	mov	ax, offset _TEXT:_dma_int	; no, use pseudo dma handler
RDMA_int:
;
; if _irqSet, then don't try to set the interrupt vector again.
;
	cmp	_irqSet,0
	jnz	_gotSifIrq
	inc	_irqSet
	DEVHELP	DevHlp_SetIRQ
	jnc	_gotSifIrq
	PANIC	0
_gotSifIrq:

; Set CONBUF register for interrupt enable

	test	_SSflag, CONBUF_EXISTS
	jz	no_conbuf1
	mov	dx,conbuf
	sin	al,dx
	and	al,NOT BLKEN
	mov	old_con,al
	or	al,INTEN
	cmp	word ptr shrmem+2,0
	jz	N3
	or	al,BLKEN
N3:
	out	dx,al
	or	al,080H
	mov	new_con,al
no_conbuf1:

; zero null interrupt counter

	mov	_gi_nullints,0

;Enable interrupts

	sti

	test	_SSflag, REAL_DMA	; skip dma enable if p1340
	jz	L4B
	test	_SSflag, CONBUF_EXISTS	; skip dma enable if p1840
	jz	L4B
	test	byte ptr _SCFeatures, SC_MICROCHAN
	jnz	L4B

; SET AT DMA ENABLE FOR IBM AT CARD

	mov	dx,0D4H
	mov	al,4H
	add	al,dmanum
	out	dx,al

	MOV	DX,0D6H
	MOV	AL,0C0H
	add	al,dmanum
	sout	DX,AL

	MOV	DX,0D4H
	MOV	AL,dmanum
	sout	DX,AL

	mov	al,new_con
	or	al,DMAEN
	mov	new_con,al
	mov	dx,conbuf
	out	dx,al

	mov	dx,irqarm	; make sure the board can interrupt
	sin	al,dx

L4B:
	pop	di
	pop	si
	EXIT
	ret
_en_sif	endp

;------------------------------------------------------------------------------;
;	_dis_sif -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_dis_sif	proc near
	public	_dis_sif

	ENTRY

	cmp	interruptHooked,0
	jz	L5B
	mov	interruptHooked,0

; set interrupt mask register bit to disable our vector

	mov	dx,imrport
	sin	al,dx
	or	al,irqbit
	sout	dx,al

; restore old ISR

	cli

	mov	bx, word ptr intLine
;	DEVHELP	DevHlp_UnSetIRQ

; tell board to stop interrupting

	test	_SSflag, CONBUF_EXISTS
	jz	no_conbuf3
	mov	dx,conbuf
	mov	al,old_con
	sout	dx,al
no_conbuf3:

; disable DMA

	test	_SSflag, REAL_DMA 	; skip dma disable if p1340
	jz	L1
	test	_SSflag, CONBUF_EXISTS	; skip dma disable if p1840
	jz	L1
 	test	byte ptr _SCFeatures, SC_MICROCHAN
	jnz	L1

	mov	dx,0D4H
	mov	al,4H
	add	al,dmanum
	sout	dx,al

L1:

	sti
L5B:
	EXIT
	ret
_dis_sif	endp

;------------------------------------------------------------------------------;
;	_ticks -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_ticks	proc near
	public	_ticks
	mov	ax, _realTimeTicks
	ret
_ticks	endp
	
;
; _sysconfig
;
; input: nothing
;
; output: sets bits of SSflag based on call to _systype
;

;------------------------------------------------------------------------------;
;	_sysconfig -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_sysconfig	proc near
	public	_sysconfig

	ENTRY

	push	ax
	push	dx
        push    ds
	mov 	_SSflag,0

        

;	call	_systype
;	cmp	ax, 2
;	ja	not_pcbus
;
;;	and	_SSflag, NOT REAL_DMA
;;	and	_SSflag, NOT TWO_PICS
;	or	_SSflag, CONBUF_EXISTS
;	cmp	ax, 1		      	; a PC or XT?
;	jne	done_sys_config	      	; no, jump
;	or	_SSflag, SHMEM_OK	; OK to use shared memory
;	jmp	done_sys_config
;
;not_pcbus:
;	or	_SSflag, REAL_DMA	; in case it turns out they're
;				      	; using a p1340 in an AT, this
;					; will be fixed later in bringup().
;	or	_SSflag, TWO_PICS
;;	and	_SSflag, NOT SHMEM_OK
;	cmp	ax, 3		; AT bus?
;	jne	done_sys_config	; no, jump
;	or	_SSflag, CONBUF_EXISTS
;

done_sys_config:
        pop     ds
	pop	dx
	pop	ax
	EXIT
	ret
_sysconfig	endp

;
; search for an p1840 adapter in the POS register space. If this not a PS/2,
; then the search will fail and the SC_MICROCHAN bit in _SCFeatures will be
; clear. if this is a PS/2 that contains an p1840, then SC_MICROCHAN will be
; set.
;
IsThisAp1840NIC	proc	near

	ENTRY	0
	push	ax
	push	bx
	push	cx

	; clear initially

	and	byte ptr _SCFeatures, NOT SC_MICROCHAN

	mov	cx, 8				; Number of slots, max.
	xor	bx, bx				; Holds current slot #

NextSlot:
	mov	al, bl
	or	al, 08h				; Bit 3 must be set

	out	96h, al				; Select the slot

	mov	dx, 101h
	in	al, dx				; Read POS1

	mov	ah, al
	dec	dx
	in	al, dx				; Read POS0

	push	ax					; close the POS register
	xor	al, al
	out	96h, al
	pop	ax

	cmp	ax, 0101h			; p1840 ID
	je	WeAreOnAPS2

	inc	bx				; Try next slot
	loop	NextSlot

_x1840NIC:

	pop	cx
	pop	bx
	pop	ax
	EXIT
	ret

WeAreOnAPS2:

	or	byte ptr _SCFeatures, SC_MICROCHAN
	jmp	short _x1840NIC

IsThisAp1840NIC	endp

;
; systype
;
; input: nothing
;
; output:
;	AX = 1, PC or XT with PC bus
;	AX = 2, PC bus, but not PC or XT (typically PS/2 model 25 or 30)
;	AX = 3, AT bus
;	AX = 4, microchannel bus
;
; notes:
;	1) return codes 3 and 4 imply two PICs.
;	2) return codes 1 and 2 differentiate slower and faster machines.
;		this is necessary to know whether to use shared memory on the p1340.
;
_systype	proc	near
	push	ds
	push	es
	push	bx

	push	cx
	push	di

	; if this is an OS/2 version, then we're either a 3 (AT class) or
	; a 4 (MCA).

	mov		ax, 0Fh
	mov		bx, 0FFFEh
	mov		cx, 10		;
	mov		dh, 1		; Leave result in es:di (cx is setup above)
	DEVHELP	DevHlp_PhysToVirt
	jnc		ptvOK
	PANIC	4
ptvOK:
	mov		ax, 3
	call		IsThisAp1840NIC
;	cmp		byte ptr es:[di], ATIDENTIFIER
;	jz		not_mchan
;	inc		ax

not_mchan:
	pop		di
	pop		cx

	; Next stop systype_ret


systype_ret:
	pop	bx
	pop	es
	pop	ds
	ret

_systype	endp

; Function to issue an scb request to the adapter

;------------------------------------------------------------------------------;
;	_scb_request -
;
;	description - void scb_request(WORD cmdCode, char *p)
;
;	inputs:
;
;	outputs:
;
;
_scb_request	proc near
	public	_scb_request

	ENTRY

	; wait for the adapter to set the command code to 0

SCBLOOP:
	mov	ax,word ptr _gi_scb
	test	ax,ax
	jnz	SCBLOOP

	; setup the command code in the right order

	mov	ax,4[bp]
	xchg	ah,al
	mov	word ptr _gi_scb,ax

	; compute physical address and put the resulting 24 bit address into
	; the command block.

	push	6[bp]
	call	_aform
	add	sp,2
	mov	word ptr _gi_scb+2,ax
	mov	word ptr _gi_scb+4,dx

	; yank the adapter interrupt

	mov	dx,sifint
	mov	ax,IR_INT_ADAP OR IR_EXECUTE OR IR_INT_SYS
	wout	dx,ax
	EXIT
	ret
_scb_request	endp



;------------------------------------------------------------------------------;
;	_scb_lrequest -
;
;	description - void scb_lrequest(WORD cmdCode, ulong)
;			  
;	inputs:
;
;	outputs:
;
;
_scb_lrequest	proc near
	public	_scb_lrequest

	ENTRY

	; wait for the adapter to set the command code to 0

SCBLLOOP:
	mov	ax,word ptr _gi_scb
	test	ax,ax
	jnz	SCBLLOOP

	; setup the command code in the right order

	mov	ax,4[bp]
	xchg	ah,al
	mov	word ptr _gi_scb,ax

	mov	ax, 6[bp]
	mov dx, 8[bp]
	mov	word ptr _gi_scb+2,ax
	mov	word ptr _gi_scb+4,dx

	; yank the adapter interrupt

	mov	dx,sifint
	mov	ax,IR_INT_ADAP OR IR_EXECUTE OR IR_INT_SYS
	wout	dx,ax
	EXIT
	ret
_scb_lrequest	endp

; *********************************************
; Interrupt routine to service SIF interrupts
; *********************************************

_sif_int proc	far

	cli

sif_int_i:

; see if our board requested SIF service

	mov	dx,sifint
	in	al,dx
	test	al,IR_INT_SYS
	jnz	is_sif

; null interrupt

	inc		_gi_nullints
	cmp		_gi_nullints,0FFH
	jz		internal_check

; let it go, haven't reached limit yet


	mov	ax, _MSC.MscInterrupt
	DEVHELP	DevHlp_EOI

	clc
	ret

L5Z:

	pop	dx
	pop	ax
	pop	ds
	iret


; too many bogus interrupts.

	public	internal_check
internal_check:

; Set this up as an adapter check, Adapter Inoperative.

    
	mov	word ptr _gi_ssb, SCB_INTCHECK
	cld

	mov	ax,ds
	mov	es,ax
	jmp	ring_status

; SIF is requested - save more registers

is_sif:

	cld

	mov	ax,ds
	mov	es,ax

	mov	dx,sifint	; read sif interrupt register
	win	ax, dx
	and	ax,0EH		; mask to extract interrupt reason
	mov	bx,ax
	jmp	adap_int[bx]	; jump to handle interrupt

; restore registers and exit

all_done:
	mov	ax,IR_INT_ADAP OR IR_SSB_CLR
	mov	dx,sifint
	wout	dx,ax

; EOI

	mov	ax, _MSC.MscInterrupt
	DEVHELP	DevHlp_EOI

	assume CS:CGROUP, DS:DGROUP
	test	_SSFlag, CONBUF_EXISTS
	jz	no_irqarm		;	so is irq re-arm register
	mov	dx,irqarm
	in	al,dx
no_irqarm:

	;
	; notify that work is requested
	;
	or	forkstate, FORKREQ

	;
	; check to see if it's already being processed
	;
	test	forkstate,FORKED
	jnz	int_exit


	; check to see if in real or protected mode. if real, then switch.
	; save the original machine status word on the stack for later
	; retrieval.
ifdef OS2
	smsw	ax
	push	ax
	shr	ax, 1

	; if CY set, then the proc is already in protected mode.

	jnc	_gotoProtectedMode

endif
_execute:

	STACK_SETUP

	;
	; force the fork state
	;
	mov	forkstate,FORKED

	;
	; the excutor always returns with interrupts disabled
	;
	call	_executor

	STACK_RESTORE

	;
	; allow the next interrupt to handle the work
	;
	mov	forkstate,0

	;
	; if the driver is not bound, then the protocol dispatch entry point will
	; not be valid.
	;
	test	byte ptr _MSS.MssStatus, MS_BOUND
	jz	chk_prot

	;
	; check for indications pending
	;
	cmp	_ndisGen.needIndCmplt, 0
	je	chk_prot

	;
	; an indication is needed
	;
	mov	byte ptr _ndisGen.needIndCmplt, 0
	push	word ptr _MCC.CcModuleID
	push	word ptr _protCC.CcDataSeg
	sti
	call	dword ptr _pldDsptchTble.PldIndComplete

chk_prot:


	; if the proc was in real mode, then restore the machine status word.

	pop	ax
	shr	ax, 1
	jnc	_gotoRealMode


int_exit:

	clc
	ret

	; jump to these code clauses if in real mode so that the normal OS/2
	; protected mode does not incur a branch hit.

_gotoProtectedMode:
	DEVHELP	DevHlp_RealToProt
	jmp	short _execute

_gotoRealMode:
	DEVHELP	DevHlp_ProtToReal
	jmp	short int_exit


adapter_check:

	mov	ax,05E0H
	mov	dx,sifadr
	wout	dx,ax
	doffset di, _gi_ssb
	mov		ax, SCB_ADAPCHECK
	stosw
	mov	dx,sifdatai
	mov	cx,4
AC_loop:	
	win	ax,dx
	stosw
	loop	AC_loop
	jmp	ring_status

; The NDIS driver uses the SCB_CLEAR interrupt to finish a rcv that has
; been held off because of the INDICATION OFF.
; If we don't have a rcv pending, just leave.

scb_clear:
	cmp	_receivePending, 0
	jne	finish_receive
	jmp	all_done			; Not a "special" SCB_CLEAR, exit.

finish_receive:
	or	forkstate, FORKREQ
	mov	_receivePending, 0
	mov	di, _ssb_tail
	mov	word ptr [di], SCB_RCV_PENDING
	add	di, 10
	jmp	short ssbCleanup

	
ring_status:
command_status:
receive_status:
transmit_status:
	or		forkstate,FORKREQ
	mov		di,_ssb_tail
	doffset 	si, _gi_ssb
	mov	cx,5
ssb_cpy:
	lodsw
	xchg	ah,al
	stosw
	loop	ssb_cpy
ssbCleanup:
	cmp		di,_ssb_end
	jnz		GI1
	doffset 	di,_ssb_ring
GI1:
	mov	_ssb_tail,di
	jmp	all_done

_sif_int endp

; Interrupt routine to service data transfer requests from the board

_dma_int proc	far

        int     3
	cli


; see if our board requested DMA service

	mov	dx,conbuf
	in	al,dx
	test	al,SDMA
	jnz	is_dma
	jmp	sif_int_i

; dma is requested - save more registers

is_dma:

;	inc	_dmacnt
	cld

	STACK_SETUP

; acknowledge to the board

	mov	al,new_con
	out	dx,al

; read transfer length into cx

	mov		dx,dmalen
	win		ax,dx
	xchg	ah,al
	mov		cx,ax

	mov		dx,conbuf
	sin		al,dx
	mov		conval,al

; read transfer address into es:di

RealAddressCalculation:

	push	cx
	mov		dx, dmaseg
	win		ax, dx		; AX <= high, middle bytes, 24 bit address
	mov		bh, al		; bh <= middle byte
	mov		al, ah		
	xor		ah, ah
	push	ax		; High word, phys addr, save for later.	
	mov		dx, dmaoff
	sin		al, dx		; AL <= low byte
	mov		bl, al		; bl <= low byte (BX is low word, phys addr)
	pop		ax		; high word, physical address.

; If the direction of the xfer is from the board to the PC, we must
; subtract one from the destination pointer (unless the xfer length is one).
; (I could not find the reason for this documented anywhere, just follow the 
; existing code and take it on faith. wbc)


	test	conval,SDDIR
	jz	addr_ok
	cmp	cx,1
	jz	addr_ok

; The physical address is in ax:bx

	sub		bx, 1
	jnc		addr_ok
	dec		ax

addr_ok:

	mov		dh, 1		; Leave result in es:di (cx is setup above)
	DEVHELP	DevHlp_PhysToVirt
	jnc		pv_OK

	PANIC	1

pv_OK:
	pop		cx

	test	conval,BLKEN
	jnz	mem_move
	jmp	io_move

mem_move:
	test	conval,SDDIR
	jz	move_from_pc
	
	push	ds
	push	si
	lds	si,shrmem

	rep	movsb

	pop	si
	pop	ds
	jmp	dma_done
	
move_from_pc:

	push	ds
	push	si
	push	es
	mov	si,di
	les	di,shrmem
	pop	ds

	rep	movsb

	pop	si
	pop	ds
	jmp	dma_done

io_move:
	mov	dx,dmadata
	test	conval,SDDIR
	jz	io_from_pc

io_in_loop:
	in	al,dx
	stosb
	loop	io_in_loop
	jmp	dma_done
	
io_from_pc:
	push	si
	push	ds
	push	es
	pop	ds
	mov	si,di

io_out_loop:
	lodsb
	out	dx,al
	loop	io_out_loop
	pop	ds
	pop	si
	jmp	dma_done


; EOI, re-arm interrupt, restore registers and exit

dma_done:

; OS/2 only: Need to UnPhys

	DEVHELP	DevHlp_UnPhysToVirt

	mov	ax, _MSC.MscInterrupt
	DEVHELP	DevHlp_EOI

D5:
	; assume that if we were doing pseudo-dma then irqarm reg is there

	mov	dx,irqarm
	in	al,dx


	clc
	ret	

_dma_int endp

	public	__chkstk
	public	_disable_int
	public	_enable_int

;EXTRN	_chkq:near
;EXTRN	_death:near

__chkstk proc near
	pop	cx
	sub	sp,ax
	jmp	cx
__chkstk endp

;------------------------------------------------------------------------------;
;	_peekb -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_peekb	proc near
	public	_peekb

	ENTRY

	les	bx,4[bp]
	mov	al,es:[bx]
	EXIT
	ret
_peekb	endp

;------------------------------------------------------------------------------;
;	_inpw -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_inpw	proc near
	public	_inpw

	ENTRY
	mov	dx,4[bp]
	win	ax,dx
	EXIT
	ret
_inpw	endp
	
_inp	proc near
	public	_inp

	ENTRY
	xor	ah, ah
	mov	dx,4[bp]
	in	al,dx
	EXIT
	ret
_inp	endp

;------------------------------------------------------------------------------;
;	_outpw -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_outpw	proc near
	public	_outpw

	ENTRY
	mov	dx,4[bp]
	mov	ax,6[bp]
	wout	dx,ax
	EXIT
	ret
_outpw	endp

_outp	proc near
	public	_outp

	ENTRY
	mov	dx,4[bp]
	mov	al,6[bp]
	out	dx,al
	EXIT
	ret
_outp	endp

;------------------------------------------------------------------------------;
;	__dsreg -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
__dsreg	proc near
	public	__dsreg
	mov	ax,ds
	ret
__dsreg	endp

;------------------------------------------------------------------------------;
;	__csreg -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
__csreg	proc near
	public	__csreg
	mov	ax,cs
	ret
__csreg	endp

;------------------------------------------------------------------------------;
;	_lsw -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_lsw	proc near
	public	_lsw

	ENTRY
	mov	ax,4[bp]
	EXIT
	ret
_lsw	endp
	
;------------------------------------------------------------------------------;
;	_msw -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_msw	proc near
	public	_msw

	ENTRY
	mov	ax,6[bp]
	EXIT
	ret
_msw	endp

;------------------------------------------------------------------------------;
;	_lohi -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_lohi	proc near
	public	_lohi

	ENTRY
	mov	ax,4[bp]
	mov	dx,6[bp]
	EXIT
	ret
_lohi	endp
	
;------------------------------------------------------------------------------;
;	_swaps -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_swaps	proc near
	public	_swaps

	ENTRY
	mov	ax,4[bp]
	xchg	ah,al
	EXIT
	ret
_swaps	endp

;------------------------------------------------------------------------------;
;	_swapl -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_swapl	proc near
	public	_swapl

	ENTRY
	mov	dx,4[bp]
	mov	ax,6[bp]
	EXIT
	ret
_swapl	endp

;------------------------------------------------------------------------------;
;	_swaplb -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_swaplb	proc near
	public	_swaplb

	ENTRY
	mov	dx,4[bp]
	mov	ax,6[bp]
	xchg	ah,al
	xchg	dh,dl
	EXIT
	ret
_swaplb	endp


;********************************************************************
; Get the Dgroup physical base address, stash it away,
; this routine should be called at init time.
;
;
_getDgroupPhy	proc	near
	public	_getDgroupPhy

; Save registers
	push	si

; Devhelp VirtToPhys with DS:0

	xor 	si, si
	DEVHELP	DevHlp_VirtToPhys

; 32-bit physical address in AX:BX
	mov	[_baseDgroup+2], ax
	mov	[_baseDgroup], bx
	pop	si
	ret

_getDgroupPhy	endp

;********************************************************************
;	Calculate DMA Address, assume it must be an address for channels 5-7.
;	this implies that the incoming address must be word aligned.
;
;	assumes:
;		DS	has DGroup
;		AX	has the offset
;		BX	has the segment
;
;	returns:
;		AX	has the offset within 64K
;		BL	has the page register value
;


CalculateDMAAddress	proc	near

	push	dx
	push	cx

	mov	cl, 4
	rol	bx, cl
	mov	dx, bx
	and	dl, 0F0h
	and	bl, 00Fh
	add	ax, dx
	adc	bl, 0

	pop	cx
	pop	dx
	ret

CalculateDMAAddress	endp


;------------------------------------------------------------------------------;
;	_xform -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_xform	proc near
	public	_xform

	ENTRY
	mov	dx, [_baseDgroup+2]
	mov	ax, [_baseDgroup]
	add	ax, 4[bp]
	jnc	xform1
	inc	dx
xform1:
	xchg	ax,dx
	EXIT
	ret
_xform	endp

;------------------------------------------------------------------------------;
;	_aform -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_aform	proc near
	public	_aform

	ENTRY
	mov	dx, [_baseDgroup+2]
	mov	ax, [_baseDgroup]
	add	ax, 4[bp]
	adc	dx, 0
aform1:
	xchg	dl,ah
	xchg	dh,al

	EXIT
	ret
_aform	endp

;------------------------------------------------------------------------------;
; _disable_int - disbable interrupt using the cli instruction. return 0 if
;		interrupts were previously off, non-zero otherwise.
;
_disable_int proc near
	pushf
	pop	ax
	and	ax,0200H
	cli
	ret
_disable_int endp

;------------------------------------------------------------------------------;
; _enable_int - unconditionally enable interrupts
;
_enable_int proc near
	sti
	ret
_enable_int endp

ifdef NO_INLINE
;------------------------------------------------------------------------------;
;	_inp -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_inp	proc near
	public	_inp

	ENTRY
	mov	dx,4[bp]
	in	al,dx
	EXIT
	ret
_inp	endp

;------------------------------------------------------------------------------;
;	_outp -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_outp	proc near
	public	_outp

	ENTRY

	mov	dx,4[bp]
	mov	al,6[bp]
	out	dx,al
	EXIT
	ret
_outp	endp

endif

;------------------------------------------------------------------------------;
;	__movedata -
;
;	description - void _movedata( void far *dest, void far *src, 
;							ushort byteCnt );
;
;	inputs:
;
;	outputs:
;
;
__movedata	proc near
	public	__movedata

	ENTRY

	push	si
	push	di
	push	ds
	cld

	mov	cx,12[bp]
	jcxz	MDRET
	mov	ds,4[bp]
	mov	si,6[bp]
	mov	es,8[bp]
	mov	di,10[bp]
	mov	dx,di
	shr	dx,1
	jnc	MD1
	movsb
	dec	cx
MD1:
	mov	dx,cx
	shr	cx,1
	rep	movsw
	shr	dx,1
	jnc	MDRET
	movsb
MDRET:
	pop	ds
	pop	di
	pop	si
	EXIT
	ret
__movedata	endp

;------------------------------------------------------------------------------;
;	_memset -
;
;	description - void memset(void far *, uchar val, ushort numBytes);
;
;	inputs:
;
;	outputs:
;
;
__memset	proc near
	public	__memset

	ENTRY

	push	di
	push	es
	mov	ax,6[bp]
	mov	es,ax
	mov	di,4[bp]
	mov	bx,di
	mov	cx,0aH[bp]
	jcxz	MSRET
	mov	al,8[bp]
	mov	ah,al
	mov	dx,di
	shr	dx,1
	jnc	MS1
	stosb
	dec	cx
MS1:	mov	dx,cx
	shr	cx,1
	rep	stosw
	shr	dx,1
	jnc	MSRET
	stosb
MSRET:
	mov	ax,bx
	pop	es
	pop	di
	EXIT
	ret
__memset	endp

;------------------------------------------------------------------------------;
;	_memcpy -
;
;	description -
;
;	inputs:
;
;	outputs:
;
;
_memcpy	proc near
	public	_memcpy

	ENTRY

	push	di
	push	si
	push	ds
	pop	es
	mov	di,4[bp]
	mov	si,6[bp]
	mov	dx,di
	mov	cx,8[bp]
	cmp	di,si
	jbe	MC2
	mov	ax,si
	add	ax,cx
	cmp	di,ax
	jnb	MC2
	add	si,cx
	add	di,cx
	dec	si
	dec	di
	std
MC1:	rep	movsb
	cld
	jmp	MCRET
MC2:	mov	ax,di
	or	ax,si
	shr	ax,1
	jnc	MC3
	mov	ax,di
	xor	ax,si
	shr	ax,1
	jc	MC1
	movsb
	dec	cx
MC3:	mov	bx,cx
	shr	cx,1
	rep	movsw
	shr	bx,1
	jnc	MCRET
	mov	al,[si]
	mov	es:[di],al
MCRET:	mov	ax,dx
	pop	si
	pop	di
	EXIT
	ret
_memcpy	endp

;
;	Wait 15 - 30 MicroSeconds. Based on the Int 8 timer source.
;
;	In: CX = cycles to wait.
;	Out: Nothing.
;

	Public	_Fixed_Wait
_Fixed_Wait	Proc	Near
	ENTRY
	Mov	CX, [BP+4]
	Xor	AX,AX
FW1:
    	In	AL,61H			; Read the Refresh timer port.
    	And	AL,10H			; Strip off extraneous bits.
    	Cmp	AL,AH			; See if same as saved value.
    	Je	FW1				; Brif not.
    	Mov	AH,AL			; Save new setting.
    	Loop	FW1			; Dec count during refresh.
	EXIT
    	Ret
_Fixed_Wait	ENDP



;------------------------------------------------------------------------------;
;	_FillDHB
;
;	description - Transfers data from transmit buffer descriptor to
;                     DHB for a transmit command.
;
;	inputs:      Address offset of DHB
;
;	outputs:
;
;
DHB_Offset      equ     4[bp]
BuffDesc        equ     6[bp]
tmpDevHlp       equ     dword ptr [bp-4]

_FillDHB        proc    near
	        public	_FillDHB

	ENTRY   4

        push    es
        push    ds
        push    di
        push    si
        push    cx
        push    dx
;
; Load pointers to immediate data and DHB offset
;
;        push    '3'
;        call    _putChar
;        add     sp,2

        mov     si,BuffDesc
        les     di,dword ptr _RAMAddress
        mov     di,DHB_Offset

        mov     cx,ds:[si].TxImmedLen
        jcxz    No_Immediate_Data
        push    ds
        push    si
        lds     si,ds:[si].TxImmedPtr
;
; Now ES:DI points to our transmit buffer and DS:SI points to the
; imediate data.
;
        rep     movsb                   ; Copy immediate data.

        pop     si
        pop     ds

No_Immediate_Data:
        mov     cx,ds:[si].TxDataCount  ; Any data blocks?
        jcxz    NoMoreData              ; Nope.
;
; For each data block copy the data to the transmit buffer and keep track
; of the total number of bytes.
;
DataCountLoop:
        add     si,TxDataBlk            ; Point to next data block.
        push    cx                      ; CX -> # of data blocks.
        mov     cx,ds:[si].TxDataLen
        jcxz    NoDataInBlock
        push    ds
        push    si

        cmp     ds:[si].TxPtrType,0     ; Check if physical or virtual
        jne     PtrNotPhysical
;
; Must convert physical address into virtual.
;
        pushf
        push    cx                              ; Save DataLen
        mov     ax,word ptr _DevHlp+2
        mov     [bp-2],ax
        mov     ax,word ptr _DevHlp
        mov     [bp-4],ax
        mov     ax,word ptr ds:[si].TxDataPtr+2
        mov     bx,word ptr ds:[si].TxDataPtr
        mov     dh,0
        mov     dl,DevHlp_PhysToVirt
        call    tmpDevHlp
        jnc     CvrtOK
;
; An error occured, clean up stack & panic
;
        pop     cx
        popf
        pop     si
        pop     ds
        pop     cx
        EXIT
        int     3

CvrtOK:
        pop     cx

        rep     movsb                           ; Transfer data
        mov     dl,DevHlp_UnPhysToVirt          ; Release virtual address
        call    tmpDevHlp
        popf                                    ;Restore interrupt flag
        jmp     short CopyDataDone

PtrNotPhysical:

        lds     si,ds:[si].TxDataPtr            ; Load pointer
        rep     movsb                           ; Transfer data
CopyDataDone:
        pop     si
        pop     ds
NoDataInBlock:
        pop     cx

        loop    DataCountLoop

NoMoreData:

;        push    '4'
;        call    _putChar
;        add     sp,2

        pop     dx
        pop     cx
        pop     si
        pop     di
        pop     ds
        pop     es

        EXIT
        ret
_FillDHB        endp

;-----------------------------------------------------------------------
;
; Function: _IntHandler
;
; Inputs:
;	none
;
; Outputs:
;	none (clears the carry flag)
;
; Description:
;       Interrupt handler
;
;
_IntHandler     proc    far
                public  _IntHandler

        cli
;        int     3
;        STACK_SETUP
                
        les     bx,dword ptr _MMIOAddress
        mov     bx,ISRPODD
        mov     al,byte ptr es:[bx]
        and     ax,007ch

;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    ax
;        push    1
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

        test    al,040h                 ; Adapter check?
        jz      NoCheck
        call    ProcessCheck
NoCheck:
        test    al,020h                 ; Is it an SRB response?
        jz      NoSRB
        call    ProcessSRB
NoSRB:
        test    al,08h                  ; Is it an ARB command?
        jz      NoARB
        call    ProcessARB
NoARB:
        test    al,04h                  ; Is it an SSB response?
        jz      NoSSB
        call    ProcessSSB
NoSSB:
        test    al,010h                 ; Is it an ASB free?
        jz      NoASB
        call    ProcessASB
NoASB:

all_done_ISR:
        not     al                      ; Set up mask

        les     bx,dword ptr _MMIOAddress
        mov     bx,ISRPODD_RESET
        mov     byte ptr es:[bx],al     ; Reset ISRP bits.

;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    0
;        push    0ffh
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

        mov     ax,[_MSC].MscInterrupt  ; EOI our interrupt number.
        xor     ah,ah
        DEVHELP DevHlp_EOI
;
; Re-arm interrupts on the card.
;
        mov     dx,_IRQEnableReg
        mov     al,1
        out     dx,al
;
; notify that work is requested
;
	or	forkstate, FORKREQ

;
; check to see if it's already being processed
;
	test	forkstate,FORKED
	jnz	intr_exit
;
; check to see if in real or protected mode. if real, then switch.
; save the original machine status word on the stack for later
; retrieval.
;
;	smsw	ax 
	push	ax
	shr	ax, 1

; if CY set, then the proc is already in protected mode.

	jnc	DoProtectedMode

_execute_it:

	STACK_SETUP

;
; force the fork state
;
	mov	forkstate,FORKED

;
; the excutor always returns with interrupts disabled
;
	call	_executor

	STACK_RESTORE

;
; allow the next interrupt to handle the work
;
	mov	forkstate,0

;
; if the driver is not bound, then the protocol dispatch entry point will
; not be valid.
;
	test	byte ptr _MSS.MssStatus, MS_BOUND
	jz	chk_protected

;
; check for indications pending
;
	cmp	_ndisGen.needIndCmplt, 0
	je	chk_protected

;
; an indication is needed
;
	mov	byte ptr _ndisGen.needIndCmplt, 0
	push	word ptr _MCC.CcModuleID
	push	word ptr _protCC.CcDataSeg
	sti
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    0
;        push    033h
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

	call	dword ptr _pldDsptchTble.PldIndComplete

;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    0
;        push    034h
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

chk_protected:
;
; if the proc was in real mode, then restore the machine status word.
;
	pop	ax
	shr	ax, 1
	jnc	DoRealMode

intr_exit:
;        push    'I'
;        call    _putChar
;        add     sp,2
;        push    0ah
;        call    _putChar
;        add     sp,2

;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    0
;        push    2
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

	clc
        sti
	ret

;
; This code causes a switch to protected mode and back if needed while
; running OS2.
;
DoProtectedMode:
        DEVHELP DevHlp_RealToProt
	jmp	short _execute_it

DoRealMode:
        DEVHELP DevHlp_ProtToReal
        jmp     short intr_exit

_IntHandler     endp
;
;--------------------------------------------------------------------------
;--------------------------------------------------------------------------
;

;
; An adapter check has occured.  The reason code can be found in the
; shared ram at an offset found in the WWCR register pair in MMIO area.
;

ProcessCheck    proc    near
        push    ax
;
; Get offset of reason code.
;
        les     bx,dword ptr _MMIOAddress
        mov     ah,byte ptr es:[bx + WWCREVEN]
        mov     al,byte ptr es:[bx + WWCRODD]
;
; Get reason code from shared RAM.
;
        les     bx,dword ptr _RAMAddress
        mov     bx,ax
        mov     al,es:[bx]
        mov     ah,es:[bx+1]
;
; Put reason code in ssb queue.
;
        or      forkstate,FORKREQ
        mov     di,_ssb_tail
        mov     word ptr ds:[di],SCB_INTCHECK
        mov     word ptr ds:[di+2],ax

        add     di,10
        jmp     ssbIntCleanup

        ret
ProcessCheck    endp

;
; Valid ARB commands can be receive data, ring status change, request for
; transmit data, or DLC status change
;
ProcessARB      proc    near
        push    ax
;
; First check if ARB_Offset is valid.
;
        mov     ax,_ARB_Offset
        cmp     ax,0ffffh
        jne     ARB_OK
        jmp     ARB_Done

ARB_OK:
;
; If valid then extract command and status
;
        les     bx,dword ptr _RAMAddress
        mov     bx,ax
        mov     al,byte ptr es:[bx]             ; Get command
        xor     ah,ah
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    ax
;        push    5
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

        or      forkstate,FORKREQ
        mov     di,_ssb_tail
        mov     word ptr ds:[di],ax
        mov     ch,byte ptr es:[bx+6]           ; Get rcv buffer offset
        mov     cl,byte ptr es:[bx+7]
        mov     word ptr ds:[di+2],cx
        cmp     ax,0084h
        je      ARB_Continue
        cmp     ax,0081h
        je      ARB_Receive
        cmp     ax,0082h
        jne     ARB_Continue
;
; We have a transmit request.
;
;        push    bx
;        push    '1'
;        call    _putChar
;        add     sp,2
;        pop     bx


;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    0
;        push    0ddh
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

        mov     _transmit_request,0
        mov     cl,byte ptr es:[bx+1]           ; Get command correlator
        xor     ch,ch
        mov     word ptr ds:[di+4],cx
        mov     ch,byte ptr es:[bx+4]           ; Get station ID
        mov     cl,byte ptr es:[bx+5]
        mov     word ptr ds:[di+6],cx
        jmp     ARB_Continue

ARB_Receive:
;
; This is a receive data command, extract info from ARB
;
;        push    bx
;        push    'R'
;        call    _putChar
;        add     sp,2
;        pop     bx

;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    0
;        push    0bbh
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

        mov     word ptr _ndisGen.currBuff,cx
        mov     cx,word ptr _RAMAddress + 2
        mov     word ptr _ndisGen.currBuff + 2,cx
        mov     ch,byte ptr es:[bx+10]          ; Get frame length
        mov     cl,byte ptr es:[bx+11]
        mov     word ptr ds:[di+4],cx
        xor     ch,ch
        mov     cl,byte ptr es:[bx+12]          ; Get NCB type
        mov     word ptr ds:[di+6],cx
        mov     ch,byte ptr es:[bx+4]
        mov     cl,byte ptr es:[bx+5]           ; Get station ID
        mov     word ptr ds:[di+8],cx

ARB_Continue:
        add     di,10

        les     bx,dword ptr _MMIOAddress
        mov     al,02h
        mov     es:[bx+ISRAODD_SET],al
        jmp     ssbIntCleanup

ARB_Done:
        les     bx,dword ptr _MMIOAddress
        mov     al,02h
        mov     es:[bx+ISRAODD_SET],al

        pop     ax
        ret
ProcessARB      endp

ProcessSSB      proc    near
        push    ax

        mov     ax,_SSB_Offset                  ; Make sure SSB valid
        cmp     ax,0ffffh
        je      SSB_Done

        les     bx,dword ptr _RAMAddress
        mov     bx,ax
        mov     al,byte ptr es:[bx]             ; Get command
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        xor     ah,ah
;        mov     cx,ax
;        mov     al,byte ptr es:[bx+2]
;        push    0
;        push    ax
;        mov     ax,cx
;        push    ax
;        push    6
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

        cmp     al,0ah                          ; Was it a transmit?
        jne     SSB_Done                        ; Nope, continue
;
; It was a transmit command so extract info from SSB
;
;        push    bx
;        push    '6'
;        call    _putChar
;        add     sp,2
;        pop     bx

        or      forkstate,FORKREQ
        mov     di,_ssb_tail
        mov     ax,000fh
        mov     word ptr ds:[di],ax             ; Command
        mov     al,byte ptr es:[bx+2]
        mov     word ptr ds:[di+2],ax           ; Return code
        mov     al,byte ptr es:[bx+6]
        mov     word ptr ds:[di+4],ax           ; Error FS code
        mov     al,byte ptr es:[bx+1]
        mov     word ptr ds:[di+6],ax           ; Command correlator
        mov     ah,byte ptr es:[bx+4]
        mov     al,byte ptr es:[bx+5]
        mov     word ptr ds:[di+8],ax           ; Station ID.
        add     di,10
        jmp     ssbIntCleanup

SSB_Done:
        pop     ax
        ret
ProcessSSB      endp


;-----------------------------------------------------------------------
;
; Function: ProcessSRB
;
; Inputs:
;	none
;
; Outputs:
;	none 
;
; Description:
;       The adapter has recognized an SRB request and has set the return code
;       in the SRB.
;
ProcessSRB      proc    near

        push    ax
;
; If SRB has not been set up yet then we are still initializing, so just
; exit.
;
        mov     ax,[_SRB_Offset]
        cmp     ax,0ffffh
        jne     SRB_Command

        mov     _Init_Flag,1            ; Flag that init is done.
        jmp     SRB_Exit

SRB_Command:
;
; We have a valid SRB command so process it.  The SRB offset is in AX.
;
        les     bx,dword ptr _RAMAddress
        mov     bx,ax
        mov     al,byte ptr es:[bx]     ; get command code
        xor     ah,ah
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    ax
;        push    7
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

        mov     di,_ssb_tail
        mov     word ptr ds:[di],ax
;
; If command was OPEN, then get other offsets out of SRB
;
        cmp     ax,0003h
        jne     Not_OpenCommand
        call    ExtractOffsets
        jmp     SRB_Continue
Not_OpenCommand:
        cmp     ax,0
        jne     Not_IntCommand

	cmp	_receivePending, 0
	jne	Do_RcvPending
	jmp	SRB_Exit	        ; Not a "special" SCB_INTERRUPT, exit.
Do_RcvPending:

	or	forkstate, FORKREQ
	mov	_receivePending, 0
        xor     ax,ax
	jmp	short ssbIntCleanup
        
Not_IntCommand:
        mov     al,byte ptr es:[bx+2]
SRB_Continue:
        or      forkstate,FORKREQ
        mov     word ptr ds:[di+2],ax
        add     di,10
ssbIntCleanup:
        cmp     di,_ssb_end
        jnz     NoChange_ssb
        doffset di,_ssb_ring
NoChange_ssb:
        mov     _ssb_tail,di

SRB_Exit:
        pop     ax

        ret

ProcessSRB      endp


ProcessASB      proc    near
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;        push    0
;        push    0
;        push    0
;        push    8
;        call    _Logit
;        add     sp,8
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

        ret
ProcessASB      endp



;-----------------------------------------------------------------------
;
; Function: ExtractOffsets
;
; Inputs:
;	ES:BX - points to SRB in shared RAM.
;
; Outputs:
;	return code in AX
;
; Description:
;       If an OPEN command is processed, the offsets of the ARB, ASB
;       SSB, and the new SRB are in shared RAM.  This routine gets
;       these offsets and puts them in global variables.
;
;
ExtractOffsets  proc    near
;
; Check the return code.  If open failed, just exit.
;
        mov     al,byte ptr es:[bx+2]
        cmp     al,0
        je      Open_Successful
;
; The open command failed, so return error code.
;
Offsets_Exit:
        ret

Open_Successful:
;
; Get ASB offset in shared RAM.
;
        mov     ah,byte ptr es:[bx + 8]
        mov     al,byte ptr es:[bx + 9]
        mov     _ASB_Offset,ax
;
; Get new SRB offset.
;
        mov     ah,byte ptr es:[bx + 10]
        mov     al,byte ptr es:[bx + 11]
        mov     _SRB_Offset,ax
;
; Get ARB offset.
;
        mov     ah,byte ptr es:[bx + 12]
        mov     al,byte ptr es:[bx + 13]
        mov     _ARB_Offset,ax
;
; Get SSB offset.
;
        mov     ah,byte ptr es:[bx + 14]
        mov     al,byte ptr es:[bx + 15]
        mov     _SSB_Offset,ax
;
; Return SUCESS
;
        xor     ax,ax
        jmp     Offsets_Exit

ExtractOffsets  endp

_stopit proc    near
        public  _stopit

        int     3
        ret

_stopit endp


;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD
;
; format of log buffer:
;       1 byte    ->  identifier
;       3 bytes   ->  additional data
;       total  =  4 bytes


;myid    equ     [bp+4]
;data1   equ     [bp+6]
;data2   equ     [bp+8]
;data3   equ     [bp+10]
;
;_Logit  proc    near
;        public  _Logit
;
;        jmp     rightout
;
;        push    bp
;        mov     bp,sp
;
;
;        pushf
;        cli
;        push    ax
;        push    cx
;        push    dx
;        push    di
;
;;        int     3
;; interrupts off
;;
;;
;; CX -> high order part,  DX -> low order part.  Store in buffer 
;; as DWORD.
;;
;        mov     di,logptr
;
;        mov     ax,myid
;        mov     byte ptr ds:[di],al
;        inc     di
;        mov     ax,data1
;        mov     byte ptr ds:[di],al
;        inc     di
;        mov     ax,data2
;        mov     byte ptr ds:[di],al
;        inc     di
;        mov     ax,data3
;        mov     byte ptr ds:[di],al
;        inc     di
;;
;; If at end of buffer wrap around
;;
;        cmp     di,offset logbuffend
;        jne     ptrok
;        mov     di,offset logbufftop
;ptrok:
;        mov     logptr,di
;
;        pop     di
;        pop     dx
;        pop     cx
;        pop     ax
;
;        popf
;
;        pop     bp
;rightout:
;
;        ret
;
;_Logit   endp
;DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD

tmp1    struc

mybp    dw      ?
retaddr dw      ?

addr1   dw      ?
addr2   dw      ?

tmp1    ends


	_TEXT ends

	end
