/*
 * file accel.h
 *
 * header file for accel.c
 */
/* $XFree86$ */

#ifndef __ACCEL_H__
#define __ACCEL_H__
 


/*
 * includes
 */

#include "vtypes.h"



/*
 * function prototypes
 */

void RENDITIONAccelPreInit(ScrnInfoPtr pScreenInfo);
void RENDITIONAccelXAAInit(ScreenPtr pScreen);
void RENDITIONAccelNone(ScrnInfoPtr pScreenInfo);
int RENDITIONInitUcode(ScrnInfoPtr pScreenInfo);
int RENDITIONLoadUcode(ScrnInfoPtr pScreenInfo);

void RENDITIONDumpUcode(ScrnInfoPtr pScreenInfo);
void RENDITIONDrawSomething(ScrnInfoPtr pScreenInfo);



#endif /* #ifdef __ACCEL_H__ */

/*
 * end of file accel.h
 */
