/*
 * File: $Header:   \ibm16\print.h  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * Description:
 *     These definitions provide prototypes for the print module PRINT.C
 */
#ifndef PRINT_H
#define PRINT_H

extern void fStrcpy( char far *, char far *);
extern void fStrncpy( char far *, char far *, unsigned short );
extern int fStrcmpi(char far *, char far *);
extern int fStrncmpi(char far *, char far *, unsigned short );

extern int fStrlen(char far *);
extern char ucase(char);



extern void Putchar(unsigned char);
extern void putChar(unsigned char );
extern void printData( unsigned char far *, unsigned short );

#endif
