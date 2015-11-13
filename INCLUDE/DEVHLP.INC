;
; $Header:  \ibm16\devhlp.inc  2-2-93  jess fahland   $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;      Definitions for device driver helper functions.
;

DevHlp_SchedClock       EQU      0    ;  0  Called each timer tick
DevHlp_DevDone          EQU      1    ;  1  Device I/O complete
DevHlp_Yield            EQU      2    ;  2  yield CPU if resched set
DevHlp_TCYield          EQU      3    ;  3  yield to time critical task

DevHlp_ProcBlock        EQU      4    ;  4  Block on event
DevHlp_ProcRun          EQU      5    ;  5  Unblock process

DevHlp_SemRequest       EQU      6    ;  6  claim a semaphore
DevHlp_SemClear         EQU      7    ;  7  release a semaphore
DevHlp_SemHandle        EQU      8    ;  8  obtain a semaphore handle

DevHlp_PushRequest      EQU      9    ;  9  Push the request
DevHlp_PullRequest      EQU     10    ;  A  Pull next request from Q
DevHlp_PullParticular   EQU     11    ;  B  Pull a specific request
DevHlp_SortRequest      EQU     12    ;  C  Push request in sorted order

DevHlp_AllocReqPacket   EQU     13    ;  D  allocate request packet
DevHlp_FreeReqPacket    EQU     14    ;  E  free request packet

DevHlp_QueueInit        EQU     15    ;  F  Init/Clear char queue
DevHlp_QueueFlush       EQU     16    ; 10  flush queue
DevHlp_QueueWrite       EQU     17    ; 11  Put a char in the queue
DevHlp_QueueRead        EQU     18    ; 12  Get a char from the queue

DevHlp_Lock             EQU     19    ; 13  Lock segment
DevHlp_Unlock           EQU     20    ; 14  Unlock segment

DevHlp_PhysToVirt       EQU     21    ; 15  convert physical address to virtual
DevHlp_VirtToPhys       EQU     22    ; 16  convert virtual address to physical
DevHlp_PhysToUVirt      EQU     23    ; 17  convert physical to LDT

DevHlp_AllocPhys        EQU     24    ; 18  allocate physical memory
DevHlp_FreePhys         EQU     25    ; 19  free physical memory

DevHlp_SetROMVector     EQU     26    ; 1A  set a ROM service routine vector
DevHlp_SetIRQ           EQU     27    ; 1B  set an IRQ interrupt
DevHlp_UnSetIRQ         EQU     28    ; 1C  unset an IRQ interrupt

DevHlp_SetTimer         EQU     29    ; 1D  set timer request handler
DevHlp_ResetTimer       EQU     30    ; 1E  unset timer request handler

DevHlp_MonitorCreate    EQU     31    ; 1F  create a monitor
DevHlp_Register         EQU     32    ; 20  install a monitor
DevHlp_DeRegister       EQU     33    ; 21  remove a monitor
DevHlp_MonWrite         EQU     34    ; 22  pass data records to monitor
DevHlp_MonFlush         EQU     35    ; 23  remove all data from stream

DevHlp_GetDOSVar        EQU     36    ; 24  Return pointer to DOS variable
DevHlp_SendEvent        EQU     37    ; 25  an event occurred
DevHlp_ROMCritSection   EQU     38    ; 26  ROM Critical Section
DevHlp_VerifyAccess     EQU     39    ; 27  Verify access to memory
DevHlp_RAS              EQU     40    ; 28  Put info in RAS trace buffer

DevHlp_AllocGDTSelector EQU     45    ; 2D  Allocate GDT Selectors
DevHlp_PhysToGDTSelector EQU    46    ; 2E  Convert phys addr to GDT sel
DevHlp_RealToProt       EQU     47    ; 2F  Change from real to protected mode
DevHlp_ProtToReal       EQU     48    ; 30  Change from protected to real mode

DevHlp_EOI              EQU     49    ; 31  Send EOI to PIC
DevHlp_UnPhysToVirt     EQU     50    ; 32  mark completion of PhysToVirt
DevHlp_TickCount        EQU     51    ; 33  modify timer

DevHlp_GetLIDEntry      EQU     52    ; 34  Obtain Logical ID
DevHlp_FreeLIDEntry     EQU     53    ; 35  Release Logical ID
DevHlp_ABIOSCall        EQU     54    ; 36  Call ABIOS
DevHlp_ABIOSCommonEntry EQU     55    ; 37  Invoke Common Entry Point
DevHlp_GetDeviceBlock   EQU     56    ; 38  Get ABIOS Device Block


DosVar_SysINFOSeg	equ	1     ; get pointer to System Info Seg
DosVar_LocINFOSeg	equ	2     ; get pointer to Local Info Seg
DosVar_ComPIDs  	equ	3     ; owners of both com1 and com2
DosVar_VectorS		equ	4     ; Reserved
DosVar_VectorReboot	equ	5     ; Vector to reboot the DOS
DosVar_VectorM		equ	6     ; Reserved
DosVar_YieldFlag	equ	7     ; Indicator for performing Yields
DosVar_TCYieldFlag	equ	8     ; Indicator for performing TCYields



;
; invoke the Device Helper function in a generic fashion.
;
DEVHELP	MACRO func
; 	extrn	_DevHlp:dword
 	mov		dl, func
  	call	dword ptr _DATA:_DevHlp
endm

ifdef DOS

DevHlpJmpTabStruc	STRUC
	DevHlpFuncCode	dw	?
	DevHlpFuncPtr	dw	?
DevHlpJmpTabStruc	ENDS

DevHlpJmpTabDef	MACRO func, code
	dw code
	dw offset CGROUP:func
;	DevHlpJmpTabStruc <func,code>
ENDM

;------ Interrupt Matters ---------------------------------------------------;
;
; 8259A Programmable Interrupt Controller (PIC)
;
INT_1EOIR       equ     20h             ; end of interrupt register (master PIC)
INT_1IMR        equ     21h             ; interrupt mask register (master PIC)
INT_2EOIR       equ     0A0h            ; end of interrupt register (slave PIC)
INT_2IMR        equ     0A1h            ; interrupt mask register (slave PIC)
NONSPEC_EOI     equ     20h             ; non-specific end of interrupt command
INT_BASE		equ		20H
INT_BASE_ALT	equ		1C0H

endif

;
; Disable and restore interrupts based on the value of IF at the time 
; interrupts were disabled. Popping into a register and testing for IF
; is a work around for some steps of the 80286. Those steps could
; potentially not restore IF correctly. disableIntr and restoreIntr are
; meant to be used in tightly nested pairs, i.e., :
;
;	disableIntr
;	disableIntr
;	...
;	restoreIntr	DX
;	restoreIntr	DX
;
disableIntr	MACRO
	pushf
	cli
ENDM

;
; Restore interrupts using the supplied register, otherwise default to AX.
;
restoreIntr	MACRO reg
	local	exit
IFB <reg>
	pop		ax
	test	ax, 200H
ELSE
	pop		reg
	test	reg, 200H
ENDIF
	jz	exit
	sti
exit:
ENDM

event_SM_Mou            equ     0       ; event was Session Manager-mouse
    event_MUF           equ     0       ; (OBSOLETE) event was muf key
event_CtrlBrk           equ     1       ; event was Ctrl-Break
event_CtrlC             equ     2       ; event was Ctrl-C
event_CtrlScrLk         equ     3       ; event was Ctrl-ScrollLock
event_CtrlPrtSc         equ     4       ; event was Ctrl-PrtSc
event_ShftPrtSc         equ     5       ; event was Shift-PrtSc
event_SM_Kbd            equ     6       ; event was Session Manager -keyboard
event_count             equ     7       ; @@ Number of possible events

;       Character Queue structure
;
;       QueueInit must be called before any other queue manipulation
;       subroutine.  The Qsize field must be initialized before
;       calling QueueInit.

CharQueue STRUC
        Qsize   DW      ?               ; Size of queue in bytes
        Qchrout DW      ?               ; Index of next char out
        Qcount  DW      ?               ; Count of characters in the queue
        Qbase   DB      ?               ; Queue buffer
CharQueue ENDS

RASrouter struc
        pretrace_addr   dw      0
        preminor        dw      0
        posttrace_addr  dw      0
        postminor       dw      0
RASrouter ends

