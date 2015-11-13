/*
 * File: $Header:   \ibm16\print.c  2-2-93  jess fahland  $
 *
 * Copyright (c) 1993 -- DWB Associates, Inc.  All Rights Reserved.
 *
 * Description:
 *		This module contains some string manipulation functions as well as
 *		the standard printf().
 */
#include <stdio.h>

#define	INCL_DOS
#define	INCL_DOSDEVICES

#include <os2.h>
#include <string.h>
#include "ndis.h"
#include "print.h"

#define STDOUT	1
#define UC unsigned char
#define UI unsigned short
#define US UI
#define PUTCHAR(c,num) DosWrite(STDOUT, \
								(char *)&(c), \
								1, \
								(unsigned far *)&(num))

/*
 * Function: strchr
 *
 * Inputs:
 *		s - pointer to an ASCIIZ string
 *		c - character to be matched.
 *
 * Ouputs:
 *		returns a pointer to the first occurence of 'c' in the string, NULL
 *		if not found.
 *
 * Side Effects:
 *		none
 *
 */
char * _cdecl 
strchr(register const char *s, register int c)
{
	while (*s && (*s != (char)c))
		s++;
	return( *s == (char)c ? s : (char *)NULL);
}

/*
 * Function: fStrcpy
 *
 * Inputs:
 *		src - far pointer to a source string
 *		dest - far pointer to a destination string.
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 *		This function copies an ASCIIZ string from src to dest, terminating
 *		after the NULL character is copied.
 *
 */
void
fStrcpy( char far *src, char far *dest)
{
	while (*dest++ = *src++)
		;
}

/*
 * Function: fStrncpy
 *
 * Inputs:
 *		src - far pointer to a source string
 *		dest - far pointer to a destination string.
 *		len - maximum number of bytes to copy.
 *
 * Outputs:
 *		none
 *
 * Side Effects:
 *
 *		This function copies an ASCIIZ string from src to dest, terminating
 *		after the NULL character is copied or the maximum number of bytes
 *		is reached. In the latter case, the NULL byte may not have been
 *		copied.
 *
 */
void
fStrncpy( char far *src, char far *dest, unsigned short len)
{
	while ((len--) && (*dest++ = *src++))
		;
}

/*
 * Function: fStrcmpi
 *
 * Inputs:
 *		str0 - far pointer to an ASCIIZ string.
 *		str1 - far pointer to an ASCIIZ string.
 *
 * Outputs:
 *		returns 0 if the 2 strings are the same, non-zero otherwise.
 *
 * Side Effects:
 *
 * 		This function is a case insensitive string compare. The strings must
 *		be the same length for success.
 *
 */
int
fStrcmpi(char far *str0, char far *str1)
{
	/*
	 * match all of the characters in the string.
	 */
	while(*str0 && *str1 && (ucase(*str0) == ucase(*str1))) {
		str0++;
		str1++;
	}

	/*
	 * check that both strings are at an end.
	 */
	if (ucase(*str0) == ucase(*str1)) {
		return (0);
	}

	/*
	 * the match failed
	 */
	return (1);
}

/*
 * Function: fStrncmpi
 *
 * Inputs:
 *		str0 - far pointer to an ASCIIZ string.
 *		str1 - far pointer to an ASCIIZ string.
 *		len - maximum number of characters to compare
 *
 * Outputs:
 *		returns 0 if the 2 strings are the same, non-zero otherwise.
 *
 * Side Effects:
 *
 * 		This function is a case insensitive string compare. Only the number
 *		of characters specified by len are compared.
 *
 */
int
fStrncmpi(char far *str0, char far *str1, unsigned short len)
{
	/*
	 * match all of the characters in the string.
	 */
	while(len && *str0 && *str1 && (ucase(*str0) == ucase(*str1))) {
		str0++;
		str1++;
		len--;
	}

	return (len);
}

void
putChar(UC c)
{
	unsigned num;
	c &= 0x7f;
	if (!ndisGen.ring0) {
		DosWrite(STDOUT,(char far *)&c,1,(unsigned far *)&(num));
	}
	else {
		Putchar(c);
	}
	if (c == '\n') {
		putChar('\r');
	}
}

/*
 * Function: ucase
 *
 * Inputs:
 *		let - a single ASCII character
 *
 * Outputs:
 *		returns an upper case ASCII character
 *
 * Side Effects:
 *
 *		This function converts a lower case alphabetic character to upper 
 *		case. All other ASCII characters are returned unchanged.
 *
 */
char
ucase(char let)
{
	/*
	 * range check the incoming value, ignore if out of range.
	 */
	if (let >= 'a' && let <= 'z') {

		/*
		 * conver to upper case
		 */
		let -= ('a'-'A');
	}

	return (let);
}

/*
 * Function: fStrlen
 *
 * Inputs:
 *		p - far pointer to an ASCIIZ string.
 *
 * Outputs:
 *		returns the length of the input string, not including NULL.
 *
 * Side Effects:
 *
 *		This function counts the bytes of a string upto, and not including
 *		a '\0' character.
 *
 */
int
fStrlen(register char far *p)
{
	register WORD len = 0;

	/*
	 * while not 0...
	 */
	while (*p++)
		len++;
	return(len);
}

void
printData( UC far *p, US len )
{
	register US i;
	register US j;

	for (i=0; i<len; i+=16,p+=16) {
		US c;
		US maxLen = ((i+16) > len) ? len-i : 16;

		printf("%0x:%0x ",(WORD)((DWORD)p>>16),(WORD)p);
		for (j=0;j<16;j++) {
			if (j+i < len) {
				c = (US)*(p+j);
				if (c < 0x0010)
					printf("0");
				printf("%x ",c);
			}
			else
				printf("   ");
		}
		for (j=0;j<maxLen;j++) {
			c = (US)*(p+j);
			if (c < (US)' ' || c > (US)'~')
				printf(".");
			else
				printf("%c",c);
		}
		printf("\n");
	}
}

int _cdecl 
printf(const char *fmtStr, ... )
{
	US far *p = (US far *)&fmtStr;
	UC *fmt = (char *)fmtStr;
	short lead0;
	short fWidth;
	short charCnt;

	if (!fmt)
		return(0);

	while (*fmt) {

		charCnt	=
		fWidth	=
		lead0	= 0;

		if (*fmt != '%') {
			putChar(*fmt++);
			continue;
		}
		
		fmt++;
		p++;

formatLoop:
		switch (*fmt) {

			case '0':
				lead0++;
				fmt++;
				goto formatLoop;

			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				fWidth = (short)(*fmt - '9');
				/* no break */
			case '-':
				while (*fmt && ((*fmt >= '1' && *fmt <= '9') || *fmt == '-'))
					fmt++;
				goto formatLoop;

			case 'c':
				charCnt++;
				putChar((UC)*p);
				break;

			case 'd':
			{
				US i = 10000;
				US remainder;
				US oneDigitSeen = 0;
				US num = *p;
				for (; i; i /= 10) {
					remainder = num / i;
					if (remainder || oneDigitSeen || i==1) {
						charCnt++;
						putChar((UC)'0'+(UC)remainder);
						num -= (remainder*i);
						oneDigitSeen++;
					}
					else if (lead0) {
						charCnt++;
						putChar((UC)'0');
					}
				}
				break;
			}

			case 's':
			{
				UC *s = (UC *)*p;
				while (*s) {
				 	charCnt++;
					putChar(*s++);
				}
				break;
			}

			case 'x':
			{
				US i = 0x1000;
				US remainder;
				US oneDigitSeen = 0;
				US num = *p;
				for (; i; i/= 0x10) {
					remainder = num / i;
					if (remainder || oneDigitSeen || i==1) {
						charCnt++;
						putChar( (UC)remainder + (((UC)remainder < 0x0a) ? '0' : ('a'-10)));
						num -= (remainder*i);
						oneDigitSeen++;
					}
					else if (lead0) {
						charCnt++;
						putChar((UC)'0');
					}
				}
				break;
			}

			case '\0':
				return(0);

			default:
				putChar(*(fmt-1));
				putChar(*fmt);
				charCnt = fWidth;
				p--;
				break;
		}
		for (;charCnt<fWidth;charCnt++)
			putChar((UC)' ');
		fmt++;
	}
	return(0);
}


