/*
 
Copyright (c) 1990, 1991, 1992  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991, 1992 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: strftime.c,v 1.3 94/04/17 20:59:51 rws Exp $
 */
/*
 * This is a stub for strftime which is used to provide the time in the TCC.
 */

#include <sys/types.h>
#include <sys/time.h>

int
strftime(s, maxsize, format, tm)
char	*s;
int 	maxsize;
char	*format;
struct	tm	*tm;
{
char	*cp;

#ifdef	lint
	cp = format;
#endif

	cp = asctime(tm);
	/*
	 * Copy exactly HH:MM:SS\0 (TSIZE chars) into s.
	 */
#define TSIZE 9
	strncpy(s, cp+11, maxsize);
	if (maxsize >= TSIZE) {
		s[TSIZE-1] = '\0';
		return(TSIZE);
	} else {
		return(maxsize);
	}
}

#ifdef notdef

/* A complete version that requires nls */

/****************************************************************************

NAME:  		strftime() - convert date and time to string    
		
MODULE CONTAINS:
       	strftime()

SYNOPSIS:
	#include <time.h>
	int strftime(s,maxsize,format,tm)
	char *s;
	int maxsize;
 	char *format;
	struct tm *tm;
	
DESCRIPTION:
	strftime() places characters into the array pointed to by s as 
	controlled by the string pointed to by format.

AUTHOR:		Geoff Clare/Phyfos Photi
DATE CREATED:	July 15th, 1988

****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <langinfo.h>
#include <nlstest.h>

#define CHKCOUNT(incr)          count += incr; \
				if (count+1 > maxsize) \
				{ \
					*bufp = '\0'; \
					return(0); \
				};

#define PUTVAL2(bufp,val)	CHKCOUNT(2); \
                                *bufp++ = '0' + ((val)/10) % 10; \
				*bufp++ = '0' + (val) % 10  

#define PUTVAL3(bufp,val)	CHKCOUNT(3); \
                                *bufp++ = '0' + ((val)/100) % 10; \
				*bufp++ = '0' + ((val)/10) % 10; \
				*bufp++ = '0' + (val) % 10

#define PUTINFO(bufp,item)	info = nl_langinfo(item); \
				if (info == NULL || *info == '\0') \
					  ++langerror; \
				else \
                                {	  CHKCOUNT(strlen(info)); \
			        	  (void) strcpy(bufp, info); \
					  bufp += strlen(info); \
				}

extern	char	*nl_langinfo();
extern	char	*strcpy();
extern	size_t	strlen();

size_t
strftime(buf,maxsize,fmt,tm)

struct 		tm *tm;
register char 	*fmt,*buf;
size_t 		maxsize;

{
	int		langerror = 0;
	int 		count     = 0;
	register char 	*info,*bufp;
	extern int 	lc_time;
	int 		len;
	int 		tznum;
 
	if (fmt == NULL || *fmt == '\0') 
	{	fmt = nl_langinfo(D_T_FMT);
		if (fmt == NULL || *fmt == '\0')
			++langerror;
	}

	for (bufp = buf; *fmt != '\0' && langerror == 0; fmt++) 
	{	
		if (*fmt == '%')
			switch (*++fmt) 
			{
			case '%' : CHKCOUNT(1);
			       	   *bufp++ =  '%';
				   break;
			case 'n' : CHKCOUNT(1);
				   *bufp++ = '\n'; 
				   break;
			case 't' : CHKCOUNT(1);
				   *bufp++ = '\t'; 
				   break;
			case 'm' : PUTVAL2(bufp, tm->tm_mon + 1); 
				   break;
			case 'd' : PUTVAL2(bufp, tm->tm_mday); 
				   break;
			case 'y' : PUTVAL2(bufp, tm->tm_year); 
				   break;
			case 'Y' : CHKCOUNT(4);
				   (void)sprintf(bufp,"%4d",1900 + tm->tm_year);
				   bufp += 4; 
				   break;	
			case 'D' : PUTVAL2(bufp, tm->tm_mon + 1); 
				   CHKCOUNT(1);
				   *bufp++ = '/';
				   PUTVAL2(bufp, tm->tm_mday); 
				   CHKCOUNT(1);
				   *bufp++ = '/';
				   PUTVAL2(bufp, tm->tm_year);
				   break;
			case 'H' : PUTVAL2(bufp, tm->tm_hour); 
				   break;
			case 'I' : PUTVAL2(bufp, (tm->tm_hour % 12 != 0) ?
				                 (tm->tm_hour % 12) : 12);
				   break;
			case 'M' : PUTVAL2(bufp, tm->tm_min);
				   break;
			case 'S' : PUTVAL2(bufp, tm->tm_sec); 
				   break;
			case 'T' : PUTVAL2(bufp, tm->tm_hour); 
				   CHKCOUNT(1);
				   *bufp++ = ':';
				   PUTVAL2(bufp, tm->tm_min);
				   CHKCOUNT(1);
				   *bufp++ = ':';
				   PUTVAL2(bufp, tm->tm_sec);
				   break;
			case 'j' : PUTVAL3(bufp, tm->tm_yday + 1);
				   break;
			case 'w' : CHKCOUNT(1);
				   *bufp++ = tm->tm_wday + '0'; 
				   break;
			case 'a' : PUTINFO(bufp, ABDAY_1 + tm->tm_wday);
				   break;
			case 'A' : PUTINFO(bufp, DAY_1 + tm->tm_wday);
				   break;
			case 'b' :                           /* fall-through */	
			case 'h' : PUTINFO(bufp, ABMON_1 + tm->tm_mon);
				   break;
			case 'B' : PUTINFO(bufp, MON_1 + tm->tm_mon);
				   break;
			case 'r' : PUTVAL2(bufp, (tm->tm_hour + 11) % 12 + 1);
				   CHKCOUNT(1);
				   *bufp++ = ':';
				   PUTVAL2(bufp, tm->tm_min);
				   CHKCOUNT(1);
				   *bufp++ = ':';
				   PUTVAL2(bufp, tm->tm_sec); 
				   *bufp++ = ' ';
				   if (tm->tm_hour < 12)
					*bufp++ = 'A';
				   else
					*bufp++ = 'P';
				   *bufp++ = 'M';
				   break;
			case 'p' : PUTINFO(bufp,
					tm->tm_hour < 12 ? AM_STR : PM_STR);
				   break;
			case 'c' : info = nl_langinfo(D_T_FMT);
				   if (info == NULL || *info == '\0')
					++langerror;
				   else
					if ((len = strftime(bufp,maxsize - count
                                                                ,info,tm)) == 0)
						return(0);
					else
					{	CHKCOUNT(len);
						bufp += len;
					}		
				   break;
			case 'U' : if ((tm->tm_yday - tm->tm_wday) < 0)
				   {
					PUTVAL2(bufp, 0);
				   }	
				   else
				   {
					PUTVAL2(bufp, (tm->tm_yday - 
					                   tm->tm_wday)/ 7 + 1);
				   }
				   break;
			case 'W' : if (tm->tm_yday - ((tm->tm_wday != 0) ?
                                                       tm->tm_wday - 1 : 6) < 0)
				   {
					PUTVAL2(bufp, 0);
				   }
				   else
				   {
					PUTVAL2(bufp, (tm->tm_yday -
                                         ((tm->tm_wday != 0) ? tm->tm_wday - 1 :
                                                                   6)) / 7 + 1);
             			   }
				   break;
			case 'x' : info = nl_langinfo(D_FMT);
				   if (info == NULL || *info == '\0')
					++langerror;
				   else
					if ((len = strftime(bufp,maxsize - count
                                                                ,info,tm)) == 0)
						return(0);
					else
					{	CHKCOUNT(len);
						bufp += len;
					}		
				   break;	    
			case 'X' : info = nl_langinfo(T_FMT);
				   if (info == NULL || *info == '\0')
					++langerror;
				   else
					if ((len = strftime(bufp,maxsize - count
                                                                ,info,tm)) == 0)
						return(0);
					else
					{	CHKCOUNT(len);
						bufp += len;
					}		
				   break;	    
			case 'Z' : tzset();
				   if (tm->tm_isdst == 0 || *tzname[1] == '\0')
					tznum = 0;
				   else
					tznum = 1;	
			           if (tzname[tznum] == NULL || *tzname[tznum]                                                                          == '\0')
					break;
				   CHKCOUNT(strlen(tzname[tznum]));
				   (void) strcpy(bufp,tzname[tznum]);
				   bufp += strlen(tzname[tznum]);
				   break;
			case '\0': CHKCOUNT(1);
			           *bufp++ = '%';
				   --fmt;
				   break;
			default :  CHKCOUNT(2);
				   *bufp++ = '%';
				   *bufp++ = *fmt;
				   break;
			}
		else
		{	CHKCOUNT(1);
			*bufp++ = *fmt;
		}
	}

	*bufp = '\0';

	if (langerror == 0)
		return(count);
	else
		return(0);
}

#endif
