;
; $Header:   \ibm16\_init.asm   2-2-93   jess fahland  $
;
; Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
;
; description:
; 	This file represents the break between real code and
;       init code and data.   

.xlist
include segments.inc
.list

	assume CS:CGROUP, DS:DGROUP

IDATA segment

	public	_startInitData
	_startInitData	label	word

IDATA ends

_TEXT segment

	public	_startInitCode
_startInitCode	proc near
	ret
_startInitCode	endp

_TEXT ends

end

