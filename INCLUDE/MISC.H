/*
 * file: $Header:   \ibm16\misc.h  2-2-93  jess fahland $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * description:
 * 	This file describes misc functions provided in misc.c.
 */
#ifndef MISC_H
#define MISC_H
#include "ndis.h"
#include "devhdr.h"

/*
 * debug definitions. if debug is not predefined as a MACRO, then it will
 * be instantiated as a variable.
 */
#define DBG_VERBOSE	0x0001
#define DBG_TERSE	0x0002
#define DBG_SPECIAL	0x0004

#ifndef debug
	extern WORD debug;
#endif

/*
 * shorthand
 */
#define uchar	BYTE
#define ushort	WORD
#define ulong	DWORD

/*
 * debug breakpoints and panic routines. _brk() will exec an INT 3 if
 * NOPANIC is not defined. panic() prints an error code with the appropriate
 * message and then calls _brk().
 */
#if (_MSC_VER >= 600)
#define _brk() _asm { _asm int 3 }
#else
extern void _brk(void);
#endif
extern void panic(WORD);

/*
 * define operating system environment variables.
 */
#ifdef DOS
#	define dos 1
#	define os2 0
#else
#	define os2 1
#	define dos 0
#endif

/*
 * Defines for device open/close.
 */
#define SIZE_0				0L
#define NORM_FILE			0
#define EXIST_ONLY			0x0001
#define RESERVED_DW_0		0L

#ifdef OS2
#define WO_DENY_RW_PRIV	 	0x0091
#define RW_DENY_NONE_PRIV	0x00C2
#define READ_ONLY			0x0040
#else
#define WO_DENY_RW_PRIV	 	O_WRONLY
#define RW_DENY_NONE_PRIV	O_RDWR
#define READ_ONLY			O_RDONLY
#endif

/*
 * copy data from source to destination. This function will take advantage
 * of the 80386 if OS/2, 16 bit string moves otherwise.
 */
extern void _bcopy(	void far *,		/* source */
					void far *,		/* destination */
					unsigned short);/* number of bytes to copy */
extern void _bcopy32( DWORD src, DWORD dest, WORD numBytes);

/*
 * zero out a data area
 */
extern void _bzero( void far *,		/* area to blank */
					unsigned short);/* number of bytes to 0 out */

/*
 * copy data from here to there while performing a word swap. This is most
 * useful for a TI or Motorola architecture.
 */
extern void _wsBcopy(	void far *,		/* source */
						void far *,		/* destination */
						unsigned short);/* number of words to copy */

/*
 * build far pointers to _TEXT and _DATA at runtime. FNDATA converts a near
 * pointer to a far pointer without any compiler complaints. FFDATA converts
 * a far data pointer to a far data pointer with the correct DS value.
 */
#define FCODE(p) (((DWORD)DevHdr->SDevProtCS<<16)|((DWORD)(p)&0xffff))
#define FNDATA(p) (((DWORD)DevHdr->SDevProtDS<<16)|((DWORD)((WORD)(p))))
#define FFDATA(p) (((DWORD)DevHdr->SDevProtDS<<16)|((DWORD)(p)&0xffff))

/*
 * convert a physical address into a GDT virtual address
 */
extern BYTE far *physToGDT( DWORD physAddr, WORD bLen, WORD gdt );

/*
 * get a ring 3 pointer to physical memory.
 */
extern BYTE far *physToUVirt( DWORD physAddr, WORD bLen );

/*
 * allocate an array of GDT selectors, returns SUCCESS or !SUCCESS
 */
extern WORD allocGDTSelectors( WORD num, WORD *dest );

/*
 * convert a virtual address into a 32 bit physical
 */
extern DWORD virtToPhys( void far * );

/*
 * convert a virtual address into a 32 bit physical address in IBM/Motorola
 * format. returns SUCCESS or !SUCCESS.
 */
extern WORD virtToPhysMoto( void far *, DWORD far * );

/*
 * allocate/free a fixed size block of memory, return 0 if error. below 
 * states that the memory must be allocated below the 1Mb boundary.
 */
extern DWORD allocPhys( DWORD sizeBytes , BYTE below );
extern void freePhys( DWORD );

/*
 * return the processor type.
 */
#define	SIXTEEN_BIT		0x0001
#define	PROC_TYPE_MASK	0x0030

#define PROC_8086		0x0000 | SIXTEEN_BIT
#define PROC_80286		0x0010 | SIXTEEN_BIT
#define PROC_80386		0x0020
#define PROC_80486		0x0040

extern WORD procType(void);

/*
 * set/unset an IRQ
 */
#define IRQ_SHARED		1
#define IRQ_NOTSHARED	0

extern WORD setIrq( void (far *)(void), WORD /* irq */, BYTE /* share */);
extern WORD unSetIrq( WORD /* irq */ );

/*
 * perform an EOI on a specific interrupt
 */
extern void eoi(WORD /* irq */);

/*
 * swap a word/long into Motorola format
 */
extern DWORD _lSwap(DWORD);
extern WORD _wSwap(WORD);

/*
 * word swap a variable in-line, this will not work with constants.
 */
#define wSwap(a) (((a)<<8)|((a)>>8))

/*
 * word or dword swap a constant
 */
#define WSWAP(w) (((WORD)(w)<<8)|((WORD)(w)>>8))
#define LSWAP(l) _lSwap((l))

/*
 * enable/disable interrupts to the processor. disableI() returns the 
 * previous state of the interrupt flag bit. if 0, then ints were disabled, 
 * otherwise interrupts were enabled before the call.
 */
extern void _enableI(void);
extern WORD _disableI(void);

/*
 * output bytes and words with a spin loop afterwards
 */
extern WORD oSpin;	/* how much spin? */
extern void _outw(WORD,WORD);
extern void _outb(WORD,BYTE);

/*
 * input bytes and words with a spin loop afterwards
 */
extern WORD iSpin;	/* how much spin? */
extern WORD _inw(WORD);
extern BYTE _inb(WORD);

/*
 * place a constant into memory and read the previous value in one
 * instruction. can be used for semaphoring.
 */
WORD _xchg(WORD *, WORD);

/*
 * a handy MACRO to hide interrupt exclusion busy work.
 */
//#if (_MSC_VER >= 600)
//#define DISABLEI(f)	_asm { \
//	_asm pushf 		\
//	_asm cli		\
//	_asm pop f		\
//}
//#define RESTOREI(f) if ((f) & 0x0200){ _asm { _asm sti } }
//#else
#define DISABLEI(f)	(f)=_disableI();
#define RESTOREI(f) if ((f) & 0x0200) _enableI();
//#endif

#endif
