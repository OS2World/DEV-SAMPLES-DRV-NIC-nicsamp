;
; $Header:  \ibm16\gi.inc 2-2-93 jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
;    This is the Generic Call Block definition for ASM.
;    This structure must match the GCB defined in giexec.h.
;

GCB	struc

gcb_link	dd	?
gcb_upcall	dd	?
gcb_status	dw	?
gcb_node	db	6 dup(?)
gcb_size	dw	?
gcb_pointer dd	?
gcb_flags	db	?

GCB	ends
