/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/i810/os-support/linux/version-test.c,v 1.1.2.2 1999/11/18 19:06:21 hohndel Exp $ */
#include <linux/version.h>

int main(int argc, char **argv)
{
   printf(UTS_RELEASE"\n");
   return(0);
}
