/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Debug.c,v 1.3 2000/09/26 15:57:08 tsi Exp $ */

#include <sys/time.h>
#include <unistd.h> 
#include "X.h"
#include "os.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "compiler.h"

void xf86Break1(void)
{
}

void xf86Break2(void)
{
}

void xf86Break3(void)
{
}

CARD32 xf86DummyVar1;
CARD32 xf86DummyVar2;
CARD32 xf86DummyVar3;

CARD8  xf86PeekFb8(CARD8  *p) { return *p; }
CARD16 xf86PeekFb16(CARD16 *p) { return *p; }
CARD32 xf86PeekFb32(CARD32 *p) { return *p; }
void xf86PokeFb8(CARD8  *p, CARD8  v) { *p = v; }
void xf86PokeFb16(CARD16 *p, CARD16 v) { *p = v; }
void xf86PokeFb32(CARD16 *p, CARD32 v) { *p = v; }

CARD8  xf86PeekMmio8(pointer Base, unsigned long Offset)
{
    return MMIO_IN8(Base,Offset);
}

CARD16 xf86PeekMmio16(pointer Base, unsigned long Offset)
{
    return MMIO_IN16(Base,Offset);
}

CARD32 xf86PeekMmio32(pointer Base, unsigned long Offset)
{
    return MMIO_IN32(Base,Offset);
}

void xf86PokeMmio8(pointer Base, unsigned long Offset, CARD8  v)
{
    MMIO_OUT8(Base,Offset,v);
}

void xf86PokeMmio16(pointer Base, unsigned long Offset, CARD16 v)
{
    MMIO_OUT16(Base,Offset,v);
}

void xf86PokeMmio32(pointer Base, unsigned long Offset, CARD32 v)
{
    MMIO_OUT32(Base,Offset,v);
}


void
xf86STimestamp(xf86TsPtr* timestamp)
{
    if (*timestamp) {
	gettimeofday((struct timeval*)*timestamp,NULL);
    } else {
	*timestamp = xnfalloc(sizeof(xf86TsRec));
	gettimeofday((struct timeval*)*timestamp,NULL);
    }
}

void
xf86SPTimestamp(xf86TsPtr* timestamp, char *str)
{
    if (*timestamp) {
	long diff;
	struct timeval ts;
	ts = **(struct timeval**)timestamp;
	gettimeofday((struct timeval*)*timestamp,NULL);
	if (ts.tv_usec > (*timestamp)->usec) 
	    diff = ((*timestamp)->sec - ts.tv_sec - 1) * 1000
		+ (ts.tv_usec - (*timestamp)->usec) / 1000;
	else
	    diff =  ((*timestamp)->sec - ts.tv_sec) * 1000
		+(- ts.tv_usec + (*timestamp)->usec) / 1000;
	ErrorF("%s Elapsed: %i\n",str,diff);
    } else {
	*timestamp = xnfalloc(sizeof(xf86TsRec));
	gettimeofday((struct timeval*)*timestamp,NULL);
    }
}
