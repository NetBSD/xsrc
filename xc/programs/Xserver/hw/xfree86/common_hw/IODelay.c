/*******************************************************************************
  Stub for Alpha Linux
*******************************************************************************/

/* $XFree86: xc/programs/Xserver/hw/xfree86/common_hw/IODelay.c,v 3.0 1996/05/06 05:57:57 dawes Exp $ */
 
/* 
 *   All we really need is a delay of about 40ns for I/O recovery for just
 *   about any occasion, but we'll be more conservative here:  On a
 *   100-MHz CPU, produce at least a delay of 1,000ns.
 */ 
void
GlennsIODelay(count)
int count;
{
	usleep(1);
}
