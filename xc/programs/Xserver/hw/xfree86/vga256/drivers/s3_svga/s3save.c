/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3save.c,v 1.1.2.1 1998/02/07 10:05:45 hohndel Exp $ */
/*
 *
 * Copyright 1995-1997 The XFree86 Project, Inc.
 *
 */
#include "X.h"
#include "input.h"
#include "screenint.h"

#include "compiler.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86_HWlib.h"
#include "xf86_PCI.h"
#include "vga.h"
#include "vgaPCI.h"

#ifdef XFreeXDGA
#include "X.h"
#include "Xproto.h"
#include "scrnintstr.h"
#include "servermd.h"
#define _XF86DGA_SERVER_
#include "extensions/xf86dgastr.h"
#endif

#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "s3reg.h"
#include "s3.h"

#define REG50_MASK	0x673b

/*
 *   S3Save --
 *
 *	Save the current video mode.
 *
 */

void*	S3Save(vgaS3Ptr save)
{
   int i;
   unsigned char CR5C;
  

#ifdef S3_DEBUG
   ErrorF("In S3Save()\n");
#endif
   
   S3BankZero();
   
   i = inb(0x3CC);

   /* save generic vga regs */
   save = vgaHWSave((vgaHWPtr)save, sizeof(vgaS3Rec));

   save->Clock = i;   /* ? MArk */

   /* now we need to save the special registers */

   /* save a good copy now as it gets corrupted later */
   if (DAC_IS_TI3025) {
 	  outb(vgaCRIndex, 0x5C);
	  CR5C = inb(vgaCRReg);
   }


   /* Save Ramdac registers */
   (s3Ramdacs[s3RamdacType].DacSave)(save);


   for (i = 0; i < 5; i++) {
	 outb(vgaCRIndex, 0x30 + i);
	 save->s3reg[i] = inb(vgaCRReg);
	 outb(vgaCRIndex, 0x38 + i);
	 save->s3reg[5 + i] = inb(vgaCRReg);
   }
      
   for (i = 0; i < 16; i++) {
	 outb(vgaCRIndex, 0x40 + i);
	 save->s3sysreg[i] = inb(vgaCRReg);
   }

   outb(vgaCRIndex, 0x45);
   inb(vgaCRReg);         /* reset color stack pointer */
   outb(vgaCRIndex, 0x4A);
   for (i = 0; i < 4; i++) {
	 save->ColorStack[i] = inb(vgaCRReg);
	 outb(vgaCRReg,save->ColorStack[i]);  /* advance stack pointer */
   }
      
   outb(vgaCRIndex, 0x45);
   inb(vgaCRReg);         /* reset color stack pointer */
   outb(vgaCRIndex, 0x4B);
   for (i = 4; i < 8; i++) {
	 save->ColorStack[i] = inb(vgaCRReg);
	 outb(vgaCRReg,save->ColorStack[i]);  /* advance stack pointer */
   }

   if (S3_801_928_SERIES(s3ChipId)) {
         for (i = 0; i < 16; i++) {
	     if (!((1 << i) & REG50_MASK))
	       continue;
	     outb(vgaCRIndex, 0x50 + i);
	     save->s3sysreg[i + 16] = inb(vgaCRReg);
         }
   }
    
   if (DAC_IS_TI3025)  /* restore 5C from above */
         save->s3sysreg[0x0C + 16] = CR5C;

   for (i = 32; i < (S3_x64_SERIES(s3ChipId) ? 46 : 
                      S3_805_I_SERIES(s3ChipId) ? 40 : 38); i++) {
	 outb(vgaCRIndex, 0x40 + i);
	 save->s3sysreg[i] = inb(vgaCRReg);
   }

   return ((void *) save);
}

/*
 *  S3Restore --
 *
 *  	Restore the given mode
 *
 */

void	S3Restore(vgaS3Ptr restore)
{
#ifdef S3_DEBUG
   ErrorF("In S3Restore()\n");
#endif  
}
