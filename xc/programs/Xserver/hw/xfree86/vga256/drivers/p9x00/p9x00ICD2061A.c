/*
 * Copyright 1996-1997  Joerg Knura (knura@imst.de)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * JOERG KNURA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00ICD2061A.c,v 1.1.2.1 1998/08/25 10:54:21 hohndel Exp $ */

#include "X.h"
#include "Xmd.h"
#include "p9x00Access.h"
#include "p9x00ICD2061A.h"

#define C_DATA	(CARD8)8
#define C_CLK	(CARD8)4
#define C_BOTH	(CARD8)0xC
#define C_NONE	(CARD8)0


static CARD32 clock_range[13]={
  51000000,
  53200000,
  58500000,
  60700000,
  64400000,
  66800000,
  73500000,
  75600000,
  80900000,
  83200000,
  91500000,
  100000000,
  120000000
};

CARD32 icd2061CalcClock(long clock)
{
  long p_counter;
  long q_counter;
  long m=0;
  long range_index=0;
  long diff=0x7FFFFFFF;
  long actual_diff;
  long best_p;
  long best_q;

  while (clock<50000000L) {
    clock<<=1;
    ++m;
  }
  if (clock<=120000000) {
    while (clock>clock_range[range_index])
      ++range_index;
    for (p_counter=3;p_counter<=130;++p_counter) {
      for (q_counter=2;q_counter<=129;++q_counter) {
        actual_diff=clock-(14318180<<1)*p_counter/q_counter;
        if (actual_diff<0L)
	  actual_diff=-actual_diff;
        if (actual_diff<diff) {
          diff=actual_diff;
          best_p=p_counter;
          best_q=q_counter;
        }
      }
    }
    best_q-=2;
    best_p-=3;
    return (CARD32)((range_index<<17)|(best_p<<10)|(m<<7)|best_q);
  }
  return 0L;
}


void icd2061WriteClock ( unsigned long control_word)
     {
     unsigned long index;


  for ( index = 0 ; index < 5 ; index ++ )
       {
       P9X00_WRITEICD(C_DATA) ; /* data held high */
       P9X00_WRITEICD(C_BOTH) ; /* clock goes hi while data stays hi */
       }

  P9X00_WRITEICD(C_NONE); /* let them both go low */
  P9X00_WRITEICD(C_CLK);  /* clock goes high */
  P9X00_WRITEICD(C_NONE); /* start bit = 0 */
  P9X00_WRITEICD(C_CLK);  /* clock the start bit */

  /* shift in the 24 bit clock control word */
  for ( index = 1 ; index < 0x1000000 ; index <<= 1 )
      if (control_word & index )
	 {
	 P9X00_WRITEICD(C_CLK);  /* with clock still high, data = ! bit */
	 P9X00_WRITEICD(C_NONE); /* lower clock, dont change data */
	 P9X00_WRITEICD(C_DATA); /* data = bit */
	 P9X00_WRITEICD(C_BOTH); /* clock the bit */
	 }
      else
	 {
	 P9X00_WRITEICD(C_BOTH); /* with clock still high, data = ! bit */
	 P9X00_WRITEICD(C_DATA); /* lower clock, dont change data */
	 P9X00_WRITEICD(C_NONE); /* data = bit */
	 P9X00_WRITEICD(C_CLK);  /* clock the bit */
	 }

  P9X00_WRITEICD ( C_BOTH ) ; /* clock still high, bring data high */
  P9X00_WRITEICD ( C_DATA ) ; /* toggle clock low */
  P9X00_WRITEICD ( C_BOTH ) ; /* toggle clock hi */
  }




