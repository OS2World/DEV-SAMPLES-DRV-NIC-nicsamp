;
; $Header:   \ibm16\ticker.asm  2-2-93  jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;       These are the timer interrupt handling routines.
;

.xlist
include segments.inc
include _misc.inc
include devhlp.inc
include ndis.inc
;include _hw.inc
.list

; Global information segment for DosGetInfoSeg
GINFOSEG struc
    gis_time                dd  ?
    gis_msecs               dd  ?
    gis_hour                db  ?
    gis_minutes             db  ?
    gis_seconds             db  ?
    gis_hundredths          db  ?
    gis_timezone            dw  ?
    gis_cusecTimerInterval  dw  330
    gis_day                 db  ?
    gis_month               db  ?
    gis_year                dw  ?
    gis_weekday             db  ?
    gis_uchMajorVersion     db  ?
    gis_uchMinorVersion     db  ?
    gis_chRevisionLetter    db  ?
    gis_sgCurrent           db  ?
    gis_sgMax               db  ?
    gis_cHugeShift          db  ?
    gis_fProtectModeOnly    db  ?
    gis_pidForeground       dw  ?
    gis_fDynamicSched       db  ?
    gis_csecMaxWait         db  ?
    gis_cmsecMinSlice       dw  ?
    gis_cmsecMaxSlice       dw  ?
    gis_bootdrive           dw  ?
    gis_amecRAS             db  32 dup (?)
    gis_csgWindowableVioMax db  ?
    gis_csgPMMax            db  ?
GINFOSEG ends

bios_data       segment at 40H
                org     06cH
low_time        dw      ?
bios_data       ends

GET_LOW_TIME	MACRO
	mov	ax,bios_data
	mov	es,ax
	assume	es:bios_data
	mov	bx, offset low_time
ENDM

	extrn	_DevHlp:DWORD

_DATA segment

	public	_realTimeTicks
_realTimeTicks  dw      0
_lastTick		dw		0

_timerInterval	dw	0

	_infoSeg		dd	0
	_dummy			dd	0

_DATA ends

_TEXT segment

	assume CS:CGROUP, DS:DGROUP

;------------------------------------------------------------------------------;
;	__ticker - timer interrupt handler
;
__ticker	proc	far
	public	__ticker
	sti
time:
	inc	word ptr _realTimeTicks
	clc
	ret
__ticker	endp

;------------------------------------------------------------------------------;
;	_enableTicker - get a timer interrupt handler
;
;
__enableTicker	proc near
	public	__enableTicker

	ENTRY
	push	es
	push	si
	push	cx

ifdef OS2
	; put the info seg selector hear and use it as a long address

	lea	ax, _infoSeg+2
	push	ds
	push	ax

	; don't need the other selector

	lea	ax, _dummy
	push	ds
	push	ax

	; get our info seg

	extrn	DOSGETINFOSEG:far
	call	DOSGETINFOSEG

	; extract the timer interval

	les	si, _infoSeg
	mov	ax, es:[si].gis_cusecTimerInterval

	; reduce the interval to milli-seconds

	xor	dx, dx
	mov	cx, 10
	div	cx
	mov	word ptr _timerInterval, ax

	; hook the timer interrupt

	mov	ax, offset __ticker
	DEVHELP	DevHlp_SetTimer
	jnc	done
	PANIC	2
else

	; DOS timer runs 18.2 times per second

	mov	word ptr _timerInterval, 55	; 55 milli-seconds per timer tick

endif

done:

	pop	cx
	pop	si
	pop	es
	EXIT
	ret
__enableTicker	endp

;------------------------------------------------------------------------------;
;	_disableTicker - unhook the timer interrupt
;
;
__disableTicker	proc near
	public	__disableTicker

	ENTRY

ifdef OS2
	mov	ax, offset __ticker
	DEVHELP	DevHlp_ResetTimer
	jnc	done1
	PANIC	3
endif

done1:

	EXIT
	ret
__disableTicker	endp

; get the elapsed time in milli-seconds since initTime() was called.

__getTime	proc near
	public	__getTime
	ENTRY

ifdef OS2
	push	si

	; disable interrupts 

	disableIntr

	; compute the number of msecs since last __initTime function call

	mov	ax, word ptr _realTimeTicks
	mul	word ptr _timerInterval

	; restore interrupt state

	restoreIntr	si

	; restore SI

	pop	si
else

	; the DOS timer interrupt increments a WORD at 4:6C every ~55MS

	GET_LOW_TIME
	mov	ax, word ptr _lastTick

	; if the timer WORD has changed, then increment the tick count

	cmp	ax, word ptr es:[bx]
	je	_noChange
	inc	word ptr _realTimeTicks
	mov	ax, word ptr es:[bx]
	mov	word ptr _lastTick, ax
_noChange:
	mov	ax, word ptr _realTimeTicks
	mul	word ptr _timerInterval
endif

	EXIT
	ret
__getTime	endp

; initialize the time keeper to 0

__initTime proc near
	public	__initTime
	mov	word ptr _realTimeTicks, 0
ifdef DOS
	GET_LOW_TIME
	mov	ax, word ptr es:[bx]
	mov	word ptr _lastTick, ax
endif
	xor	ax,ax
	xor	dx,dx
	ret
__initTime endp

_TEXT ends

end


