;
; $Header:   \ibm16\_misc.asm   2-2-93   jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
; 	This file contains misc support functions

.xlist
include segments.inc
include _misc.inc
include devhlp.inc
include ndis.inc
.list

_DATA segment

;*********************************************************
;* the following is a save area for the sgdt instruction *
;* used in the determination processor type              *
;*********************************************************

	gdtr	label	qword
	db	8		dup(0FFh)

	; satisfy a bogus C runtime library reference, all because of
	; movedata().

	__acrtused	dw	?
	public __acrtused

	extrn	_DevHlp:DWORD

_DATA ends

_TEXT segment

	assume CS:CGROUP, DS:DGROUP


;
; Function: _procType
;
; Inputs:
;	none
;
; Outputs:
;	returns one of the processor types PROC_8086, PROC_80286, PROC_80386,
;	or PROC_80486.
;
; Side Effects: none
;	The CY flag is cleared if this is an 80386 or 80486, signifying 32 bit
;	capability.
;
; WORD procType(void)
;
	public	_procType
_procType proc near
	ENTRY
	pushf
;
; check for 8086
;
	push	sp
	pop	ax
	cmp	sp, ax
	je	_check286
	mov	ax, PROC_8086
	stc
	jmp	_xProcType

_check286:
.286p
;
; test for 286/8086, return CY clear if 386, CY set otherwise.
;
	sgdt	fword ptr [gdtr]
	sar	byte ptr [gdtr+5],1
	jnc	_check386

	stc
	mov	ax, PROC_80286
	jmp	_xProcType

_check386:
ifdef OS2
.286p
else
.8086
endif
	clc
	mov	ax, PROC_80386

_xProcType:
	Popf			; Out with AX = Processor Type
	EXIT			;
	Ret			;
_procType endp



;-----------------------------------------------------------------------------
;
; Function: __bzero
;
; Inputs:
;	toZero - a far pointer to a block of memory
;	toZeroCnt - byte count
;
; Outputs:
;	none
;
; Side Effects:
;	This function zeroes out a block of memory.
;
; void _bzero( void far *toZero, unsigned short toZeroCnt);
;
	toZero		equ	dword ptr [bp+4]
	toZeroCnt	equ word ptr [bp+8]
__bzero	proc near
	public	__bzero
	ENTRY
	push	di

	; AX is stored into memory

	xor	ax, ax

	; get byte count, exit if 0

	mov	cx, toZeroCnt
	jcxz	_xBzero

	; get pointer to clear block

	les	di, toZero

	; save byte count for later retrieval, isolate the number of words to
	; zero.

	push	cx
	shr	cx, 1

	; do a single byte if number of words is zero

	jcxz	_doByte

	; fill the string with 0

	rep	stosw

_doByte:

	; check to see if there is an odd number of bytes

	pop	cx
	shr	cx, 1
	jnc	_xBzero

	; clear last byte

	mov	byte ptr es:[di], al

_xBzero:
	pop	di
	EXIT
	ret
__bzero	endp

;-----------------------------------------------------------------------------
;
; Function: __bcopy
;
; Inputs:
;	src		- far pointer to a block of memory
;	dest		- far pointer to a block of memory
;	numBytes	- byte count of the memory block referenced by 'src'
;
; Outputs:
;	none
;
; Side Effects:
;	This function copies a block of memory from 'src' to 'dest'. If the blocks
;	overlap, 'dest' must start after 'src'.
;
; void _bcopy( void far *src, void far *dest, WORD numBytes)
;
__bcopy	proc near
	public	__bcopy
	ENTRY

	; save the corrupted registers
	push	ds
	push	si
	push	di
	push	cx

	; after ENTRY, the stack looks like:
	;
	;	|---------------|
	;	|	numBytes	|
	;	|---------------| 12
	;	|	dest SEG	|
	;	|---------------| 10
	;	|	dest OFF	|
	;	|---------------|	8
	;	|	src SEG		|
	;	|---------------|	6
	;	|	src OFF		|
	;	|---------------|	4
	;	|	return addr	|
	;	|---------------|	2
	;	|	old BP		|
	;	|---------------| <-- BP

	numBytes	equ	word ptr [bp+12]
	dest		equ	dword ptr [bp+8]
	src		equ	dword ptr [bp+4]

	; setup for a byte string copy of numBytes from src to dest. quit if
	; count is 0.

	mov	cx,numBytes
	jcxz	_bcopyDone

	; if this is a 386 or 486, then use DWORD moves

	mov	dx, _ndisGen.procType

	; setup src and dest pointers

	lds	si, src
	les	di, dest

	; do the copy

	bcopy	ds

_bcopyDone:

	pop	cx
	pop	di
	pop	si
	pop	ds
	EXIT
	ret
__bcopy	endp

;-----------------------------------------------------------------------------
;
; Function: __bcopy32
;
; Inputs:
;	src		- physical pointer to a block of memory
;	dest		- physical pointer to a block of memory
;	numBytes	- byte count of the memory block referenced by 'src'
;
; Outputs:
;	none
;
; Side Effects:
;	This function copies a block of memory from 'src' to 'dest'. If the blocks
;	overlap, 'dest' must start after 'src'.
;
; void _bcopy32( DWORD src, DWORD dest, WORD numBytes)
;

.386p
__bcopy32	proc near
	public	__bcopy32
	ENTRY

	; save the corrupted registers
	push	esi
	push	edi
	push	ecx

	; setup for a byte string copy of numBytes from src to dest. quit if
	; count is 0.

	movzx	ecx,numBytes
	jecxz	_bcopyDone

	; setup src and dest pointers

	mov	esi, src
	mov	edi, dest

;
; see if ES is already setup
;
	mov	ax, es
	mov	dx, _ndisGen.gdt0
	cmp	ax, dx
	je	_move32
	mov	es, dx

_move32:
	; do the copy

	bcopy32	es

_bcopy32Done:

	pop	ecx
	pop	edi
	pop	esi
	EXIT
	ret
__bcopy32	endp

ifdef OS2
.286p
else
.8086
endif

;-----------------------------------------------------------------------------
;
; Function: __wsBcopy
;
; Inputs:
;	src	- far pointer to a block of memory
;	dest	- far pointer to a block of memory
;	numWOrds- count of words to move
;
; Outputs:
;	none
;
; Side Effects:
;	copies numWords from src to dest, performing a byte swap in the process
;
; void _wsBcopy( void far *src, void far *dest, WORD numWords)
;
__wsBcopy	proc near
	public	__wsBcopy
	ENTRY

	; save the corrupted registers
	push	es
	push	ds
	push	si
	push	di
	push	cx

	; after ENTRY, the stack looks like:
	;
	;	|---------------|
	;	|	numBytes	|
	;	|---------------| 12
	;	|	dest SEG	|
	;	|---------------| 10
	;	|	dest OFF	|
	;	|---------------|	8
	;	|	src SEG		|
	;	|---------------|	6
	;	|	src OFF		|
	;	|---------------|	4
	;	|	return addr	|
	;	|---------------|	2
	;	|	old BP		|
	;	|---------------| <-- BP

	numWords	equ	word ptr [bp+12]

	; setup for a byte string copy of numBytes from src to dest

	mov	cx,numWords

	; bail; out if numWords is 0

	jcxz	_wsBcopyDone

	; setup src and dest pointers

	lds	si, src
	les	di, dest

	; set the direction to auto-increment

	cld

	; let her rock

_wsCopyLoop:

	lodsw
	xchg	ah, al
	stosw
	loop	_wsCopyLoop

_wsBcopyDone:

	pop	cx
	pop	di
	pop	si
	pop	ds
	pop	es
	EXIT
	ret
__wsBcopy	endp

;-----------------------------------------------------------------------------
;
; Function: _physToGDT
;
; Inputs:
;	physAddr	- physical address to be converted
;	bLen		- byte length of the memory block
;	gdtSelector	- GDT selector to use
;
; Outputs:
;	OS/2 - returns a far virtual pointer to the memory block referenced.
;	DOS - returns a selector:offset address to the memory block referenced.
;
;	after entry, the stack looks like:
;
;		|---------------|
;		|	gdtSelector	|
;		|---------------| 10
;		|	bLen		|
;		|---------------| 8
;		|	phys HI		|
;		|---------------| 6
;		|	phys LO		|
;		|---------------| 4
;		|	ret addr	|
;		|---------------| 2
;		|	old BP		|
;		|---------------| <-- BP
;
; void far *physToGDT( ulong physAddr, ushort bLen, ushort gdtSelector )
;
	stackGdt	equ	word ptr [bp+10]
	bLen		equ	word ptr [bp+8]
	physAddrLo	equ	word ptr [bp+4]
	physAddrHi	equ	word ptr [bp+6]

_physToGDT	proc near
	public	_physToGDT
	ENTRY

	; SI, BX, and CX are used in the kernel call

	push	si
	push	bx
	push	cx

	; setup for call to PhysToGDTSelector

	mov	cx, bLen
	mov	bx, physAddrLo
	mov	ax, physAddrHi
	mov	si, stackGdt

ifdef OS2
	DEVHELP DevHlp_PhysToGDTSelector
else
;	uchar far *physToUVirt( ulong physAddr, ushort bLen )
	push	cx
	push	ax
	push	bx
	call	_physToUVirt
	add	sp, 6
endif
	jnc	_gdtOK

	; return NULL pointer

	xor	ax, ax
	xor	dx, dx
	jmp	short _xPhysToGDT

_gdtOK:
	
ifdef OS2
	; return a far pointer of the form dx:ax where the offset is 0

	xor	ax, ax
	mov	dx, si
endif

_xPhysToGDT:

	pop	cx
	pop	bx
	pop	si
	EXIT
	ret
_physToGDT	endp

;*****************************************************************************
;
; Function: LockMem - lock down memory
;
; Inputs:
;	p - far pointer to a block of memory
;
; Outputs:
;	returns a DWORD handle to use when unlocking, 0L if !SUCCESS.
;
; Side Effects:
;	This routine uses the lock memory segment
;	of the devices helper services. The routine is passed a far
;	pointer to the buffer to lock down. The function takes the
;	segment from the stack and passes that segment to the
;	lock routine.
;
; DWORD LockMem(void far *p);
;
public	_LockMem
_LockMem proc	near

	ENTRY

	; save the corrupted registers

	push	di			; save scratch regs.	push	di

	; after ENTRY, the stack looks like:
	;
	;	|---------------|	
	;	|   buff SEG 	  |	
	;	|---------------|	6
	;	|   buf  OFF 	  |	
	;	|---------------|	4
	;	|  return addr  |	
	;	|---------------|	2
	;	|   old BP      |	
	;	|---------------| <-- BP

	bufSeg		equ	word ptr [bp+6]

;		


	mov	ax,bufSeg		; set up segment addr.
	xor	bx,bx			; set short term, block till locked
ifdef OS2
	DEVHELP DevHlp_Lock
else
	clc				; indicate no errors, DOS always gets mem
endif

	jc	lock_err 		; exit if error
; return handle in form of dx:ax as a far pointer
	mov	dx,ax
	mov	ax,bx 			; save the lsw of lock handle
	clc				; indicate no errors
	jmp short lockExit

lock_err:
	stc				; show error encountered
	mov	ax,0			; set invalid parameter error code
	mov	dx,0
lockExit:

; return request handle in form of dx:ax	 High:Low

	pop	di			; restore scratch regs
	EXIT
	ret
_LockMem endp

;*****************************************************************************
;
; Function: UnlockMem
;
; Inputs:
;	requestHandle - a handle previously returned by LockMem()
;
; Outputs:
;	returns 0 if success, non-zero otherwise
;
; Side Effects:
;	This routine unlocks a segment which has been previously locked by
;	LockMem().
;
; WORD UnlockMem(far *requestHandle);
;
	public	_UnlockMem
_UnlockMem	proc	near
	ENTRY

	; save the corrupted registers

	push	di			; save scratch regs	push	di
	push	bx

	; after ENTRY, the stack looks like:
	;
	;	|---------------|	
	;	|   High handle |
	;	|---------------|	6
	;	|   Low handle  |
	;	|---------------|	4
	;	|  return addr  |
	;	|---------------|	2
	;	|   old BP      |
	;	|---------------| <-- BP

	HighHandle	equ	word ptr [bp+6]
	LowHandle	equ	word ptr [bp+4]

	mov	ax,word ptr HighHandle ; msw of lock handle
	mov	bx,word ptr LowHandle  ;  lsw of lock handle
ifdef OS2
	DEVHELP DevHlp_UnLock
else
	clc				; indicate no errors, DOS always unlocks mem
endif
	jc	unlock_err 		; check, error if carry
	clc				; good
	xor	ax,ax			; clear status
	jmp short unlockExit
unlock_err:
	stc				; error status
	mov	ax,-1			
unlockExit:
	pop	bx			;
	pop	di			; restore index reg
	EXIT
	ret
_UnlockMem	endp



;-----------------------------------------------------------------------------
;
; Function: _physToUVirt
;
; Inputs:
;	physAddr - DWORD physical address
;	bLen	- lenght of the memory block referenced by physAddr
;
; Outputs:
;	OS/2 - returns an LDT virtual pointer to the memory block.
;	DOS - returns a segment:offset address to the memory block.
;
; Side Effects:
;	convert a physical address and return the far pointer. For OS/2, the pointer
;	is an LDT virtual pointer and only exists until the next system 
;	(DEVHELP) call.
;
;	after entry, the stack looks like:
;
;		|---------------|
;		|	bLen		|
;		|---------------| 8
;		|	phys HI		|
;		|---------------| 6
;		|	phys LO		|
;		|---------------| 4
;		|	ret addr	|
;		|---------------| 2
;		|	old BP		|
;		|---------------| <-- BP
;
; void far *physToUVirt( ulong physAddr, ushort bLen )
;

_physToUVirt	proc near
	public	_physToUVirt
	ENTRY

	; ES, BX, and CX are used in the kernel call

	push	es
	push	bx
	push	cx

	; setup for call to PhysToUVirt

	mov	cx, bLen
	mov	bx, physAddrLo
	mov	ax, physAddrHi

	; get read/write data selector

	mov	dh, 1
	DEVHELP DevHlp_PhysToUVirt
	jnc	_ldtOK

	; return NULL pointer

	xor	ax, ax
	mov	dx, ax
	jmp	short _xPhysToUVirt

_ldtOK:
	
	; return a far pointer of the form dx:ax.

	mov	ax, bx
	mov	dx, es

_xPhysToUVirt:

	pop	cx
	pop	bx
	pop	es
	EXIT
	ret
_physToUVirt	endp

;--------------------------------------------------------------------------
;
; Function: virtToPhys
;
; Inputs:
;	p - far pointer to a memory block
;
; Outputs:
;	returns a DWORD physical address referencing the memory block
;
; Side Effects:
;	OS/2 - convert a virtual address into a physical address
;	DOS - convert a segment:offset address into a physical address
;
; DWORD virtToPhys( void far *p )
;
	virtualPointer	equ dword ptr [bp+4]
	devHelpPtr		equ [bp-4]

_virtToPhys	proc near
	public	_virtToPhys
	ENTRY

	; make space for devHelpPtr

	sub	sp, 4

	; save other registers

	push	ds
	push	es
	push	si
	push	bx

	; save the device help address

	mov	ax, word ptr _DevHlp
	mov	word ptr devHelpPtr, ax
	mov	ax, word ptr _DevHlp+2
	mov	word ptr devHelpPtr+2, ax

	; setup for DEVHELP

	lds	si, virtualPointer

	; cannot use the MACRO in this case because DS has been used. must do
	; from the stack.

 	mov	dl, DevHlp_VirtToPhys
	call	dword ptr devHelpPtr

	jnc	_virtToPhysOk

	; return 0 for error

	xor	ax, ax
	xor	dx, dx
	jmp	short _xVirtToPhys

_virtToPhysOk:

	; success

	xchg	ax, dx
	xchg	ax, bx

_xVirtToPhys:

	pop	bx
	pop	si
	pop	es
	pop	ds
	EXIT
	ret
_virtToPhys	endp

;-----------------------------------------------------------------------------
;
; Function: _allocGDTSelectors
;
; Inputs:
;	num - number of GDT selectors to allocate
;	dest - far pointer to the buffer where the GDT's will be placed.
;
; Outputs:
;	OS/2 - returns 0 if SUCCESS, non-zero otherwise.
;	DOS - always returns 0.
;
; Side Effects:
;	allocate an array of GDT selectors, return 0 if success, non-zero else.
;
;	after entry, the stack looks like:
;
;		|---------------|
;		|	*dest		|
;		|---------------| 6
;		|	num			|
;		|---------------| 4
;		|	ret addr	|
;		|---------------| 2
;		|	old BP		|
;		|---------------| <-- BP
;
; WORD allocGDTSelectors( ushort num, ushort *dest )
;
	numSelectors	equ	word ptr [bp+4]
	selectorDest	equ	word ptr [bp+6]

_allocGDTSelectors	proc near
	public	_allocGDTSelectors
	ENTRY

	; save regsiters

	push	es
	push	di
	push	cx

	; setup for kernel call

	mov	cx, numSelectors
	mov	di, ds
	mov	es, di
	mov	di, selectorDest

	DEVHELP	DevHlp_AllocGDTSelector

	sbb	ax, ax			; set success / failure status

	pop	cx
	pop	di
	pop	es
	EXIT
	ret
_allocGDTSelectors	endp

;-----------------------------------------------------------------------------
;
; Function: _setIrq
;
; Inputs:
;	irqFunc - far pointer to the interrupt handler function
;	irq - interrupt number to be hooked
;	share - flag specifying share abilities.
;
; Outputs:
;	returns 0 if success, non-zero otherwise.
;
; Side Effects:
;	define an interrupt handler. if share is 0, then the interrupt vector
;	is not to be shared.
;
; WORD setIrq( (void (far *irqFunc)(void)), ushort irq, uchar share);
;
	shareFlag	equ	byte ptr [bp+0ah]
	irqNum		equ	word ptr [bp+08h]
	funcOffset	equ	word ptr [bp+04h]

_setIrq	proc near
	public	_setIrq
	ENTRY
	push	bx

	; get the sharing flag, function offset, and IRQ number

	mov	dh, shareFlag
	mov	bx, irqNum
	mov	ax, funcOffset

	DEVHELP	DevHlp_SetIRQ

	sbb	ax, ax			; set success / failure status

	pop	bx
	EXIT
	ret
_setIrq	endp

;-----------------------------------------------------------------------------
;
; Function: _unSetIrq
;
; Inputs:
;	irq - interrupt vector number
;
; Outputs:
;	returns 0 if success, non-zero otherwise.
;
; Side Effects:
;	unhook an interrupt and restore the interrupt to it's previous vector.
;
; WORD unSetIrq( ushort irq );
;
	unSetIrqNum		equ	word ptr [bp+04h]

_unSetIrq	proc near
	public	_unSetIrq
	ENTRY
	push	bx

	; get the IRQ number

	mov	bx, unSetIrqNum

	DEVHELP	DevHlp_UnSetIRQ

	sbb	ax, ax			; set success / failure status

	pop	bx
	EXIT
	ret
_unSetIrq	endp

;-----------------------------------------------------------------------------
;
; Function: _eoi
;
; Inputs:
;	irq - interrupt number
;
; Outputs:
;	none
;
; Side Effects:
;	perform a specific EOI. The state of the interrupt flag is not affected.
;	The interrupt vector on the 8259 PIC gets a specific end-of-interrupt,
;	enabling further interrupts on this vector.
;
; void eoi( WORD irq )
;
	eoiIrq	equ	byte ptr [bp+4]
_eoi	proc near
	public _eoi
	ENTRY

	; get the IRQ number

	xor	ah, ah
	mov	al, eoiIrq

	DEVHELP	DevHlp_EOI

	EXIT
	ret
_eoi	endp

;-----------------------------------------------------------------------------
;
; Function: _allocPhys
;
; Inputs:
;	size - number of bytes to allocate
;	below - flag specifying if memory must be allcoated above or below the
;		1 Mb boundary.
;
; Outputs:
;	returns a DWORD physical address of the memory allocated.
;
; Side Effects:
;	OS/2 - allocate physical memory from the operating system heap.
;	DOS - allocate memory starting from the current _brk address.
;
; DWORD allocPhys( DWORD size, BYTE below );
;
	below	equ	byte ptr [bp+8]
	sizeHi	equ	word ptr [bp+6]
	sizeLo	equ	word ptr [bp+4]

_allocPhys	proc near
	public	_allocPhys
	ENTRY
	push	bx

	; setup for DEVHELP

	mov	bx,	sizeLo
	mov	ax, sizeHi
	mov	dh, below

	DEVHELP	DevHlp_AllocPhys
	jnc	_allocOk

	; return 0 as error

	xor	ax, ax
	xor	dx, dx
	jmp	short _xAllocPhys

_allocOk:

	; return physical address in DX:AX

	xchg	dx, ax
	xchg	bx, ax

_xAllocPhys:

	pop	bx
	EXIT	
	ret
_allocPhys	endp

;-----------------------------------------------------------------------------
; Function: _freePhys
;
; Inputs:
;	addr - physical address of the memory to be freed
;
; Outputs:
;	none
;
; Side Effects:
;	free physical memory previously allocated by allocPhys(). For DOS, this 
;	memory must be freed in a LIFO fashion.
;
; void freePhys( DWORD addr )
;
	addrLo	equ	word ptr [bp+4]
	addrHi	equ	word ptr [bp+6]

_freePhys	proc near
	public	_freePhys
	ENTRY
	push	bx

	; setup for DEVHELP

	mov	ax, addrHi
	mov	bx, addrLo

	DEVHELP	DevHlp_FreePhys

	; return no error

	pop	bx
	EXIT
	ret
_freePhys	endp

;-----------------------------------------------------------------------------
;
; Function: __brk
;
; Inputs:
;	errorCode - the positional indicator of where _brk() was invoked.
;
; Outputs:
;	none
;
; Side Effects:
;	This functions sets AX to the error code and executes an INT 3. This causes
;	the kernel debugger to break, giving the developer debug access to the 
;	driver. if NOPANIC is defined, the INT 3 is not executed.
;
; void _brk( WORD errorCode )
;
	errorCode equ [bp+4]
__brk proc near
	public __brk
	ENTRY

	push	ax
	mov	ax, errorCode
ifndef NOPANIC
	int	3
endif
	pop	ax

	EXIT
	ret

__brk endp


;--------------------------------------------------------------------------
;
; Function: _enableI
;
; Inputs:
;	none
;
; Outputs:
;	none
;
; Side Effects:
;	This function unconditionally enables interrupts. This is a C callable
;	function usually used when providing exclusion.
;
; void _enableI(void)
;
 	public	__enableI
__enableI proc	near
	sti	
	ret
__enableI endp

;--------------------------------------------------------------------------
;
; Function: __disableI
;
; Inputs:
;	none
;
; Outputs:
;	returns the previous state of the interrupt flag, 0 if clear, non-zero
;	otherwise.
;
; Side Effects:
;	disable interrupts and return previous state of the interrupt flag bit.
;
; WORD _disableI(void);
;
		public	__disableI
__disableI proc	near
	pushf
	cli	
	pop	ax
;	and	ax, 0200H
	ret
__disableI endp


;--------------------------------------------------------------------------
;
; Function: __xchg
;
; Inputs:
;	mem - near pointer to a WORD
;	val - new vlaue
;
; Outputs:
;	returns previous value of memory word pointed to by 'mem'
;
; Side Effects:
;	This function can be used when providing semaphoring or exclusion because
;	it reads and sets a memory location in one uninterruptable instruction.
;
; WORD _xchg(WORD *mem, WORD val);
;
	val	equ	word ptr [bp+6]
	memPtr	equ word ptr [bp+4]

	public	__xchg
__xchg	proc near
	ENTRY
	push	si

	; setup for the exchange

	mov	si, memPtr
	mov	ax, val

	; do the swap, return the previous value in AX

	xchg	ax, word ptr [si]

	pop	si
	EXIT
	ret
__xchg	endp


_TEXT ends

end

