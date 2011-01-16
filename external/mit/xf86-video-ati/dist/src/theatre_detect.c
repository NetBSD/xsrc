/*************************************************************************************
 * 
 * Copyright (C) 2005 Bogdan D. bogdand@users.sourceforge.net
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, 
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE 
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the author shall not be used in advertising or 
 * otherwise to promote the sale, use or other dealings in this Software without prior written 
 * authorization from the author.
 *
 * $Log: theatre_detect.c,v $
 * Revision 1.1.1.1.2.1  2011/01/16 14:17:40  bouyer
 * Apply patch, requested by mrg in ticket #1524:
 * external/mit/xorg/server/drivers/xf86-video-radeon/Makefile		patch
 * xsrc/external/mit/xf86-video-ati/dist/COPYING				patch
 * xsrc/external/mit/xf86-video-ati/dist/ChangeLog				patch
 * xsrc/external/mit/xf86-video-ati/dist/Makefile.am			patch
 * xsrc/external/mit/xf86-video-ati/dist/Makefile.in			patch
 * xsrc/external/mit/xf86-video-ati/dist/README				patch
 * xsrc/external/mit/xf86-video-ati/dist/aclocal.m4			patch
 * xsrc/external/mit/xf86-video-ati/dist/compile				patch
 * xsrc/external/mit/xf86-video-ati/dist/config.guess			patch
 * xsrc/external/mit/xf86-video-ati/dist/config.h.in			patch
 * xsrc/external/mit/xf86-video-ati/dist/config.sub			patch
 * xsrc/external/mit/xf86-video-ati/dist/configure				patch
 * xsrc/external/mit/xf86-video-ati/dist/configure.ac			patch
 * xsrc/external/mit/xf86-video-ati/dist/depcomp				patch
 * xsrc/external/mit/xf86-video-ati/dist/install-sh			patch
 * xsrc/external/mit/xf86-video-ati/dist/ltmain.sh				patch
 * xsrc/external/mit/xf86-video-ati/dist/missing				patch
 * xsrc/external/mit/xf86-video-ati/dist/man/Makefile.am			patch
 * xsrc/external/mit/xf86-video-ati/dist/man/Makefile.in			patch
 * xsrc/external/mit/xf86-video-ati/dist/man/radeon.man			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/Makefile.am			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/Makefile.in			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/ati.c				patch
 * xsrc/external/mit/xf86-video-ati/dist/src/ati_pciids_gen.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/atipciids.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/atombios_crtc.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/atombios_output.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/drmmode_display.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/drmmode_display.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/legacy_crtc.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/legacy_output.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/local_xf86Rename.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/r600_exa.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/r600_reg_r6xx.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/r600_shader.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/r600_shader.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/r600_state.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/r600_textured_videofuncs.c	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/r6xx_accel.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_accel.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_accelfuncs.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_atombios.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_atombios.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_bios.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_chipinfo_gen.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_chipset_gen.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_commonfuncs.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_crtc.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_cursor.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_dga.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_dri.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_dri.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_dri2.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_dri2.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_driver.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_drm.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_dummy_bufmgr.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_exa.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_exa_funcs.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_exa_render.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_exa_shared.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_exa_shared.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_kms.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_legacy_memory.c	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_macros.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_modes.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_output.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_pci_chipset_gen.h	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_pci_device_match_gen.h	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_pm.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_probe.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_probe.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_reg.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_render.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_textured_video.c	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_textured_videofuncs.c	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_tv.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_tv.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_vbo.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_vbo.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_version.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_video.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_video.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_vip.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/simple_list.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/theatre200.c			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/theatre_detect.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/AtomBios/CD_Operations.c	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/AtomBios/includes/Decoder.h	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/AtomBios/includes/ObjectID.h	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/AtomBios/includes/atombios.h	patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Crtc.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Crtc.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Cursors.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86DiDGA.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86EdidModes.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Modes.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Modes.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86RandR12.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86RandR12.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Rename.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Rotate.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86cvt.c		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/parser/xf86Optrec.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/parser/xf86Parser.h		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/pcidb/ati_pciids.csv		patch
 * xsrc/external/mit/xf86-video-ati/dist/src/pcidb/parse_pci_ids.pl	patch
 * xsrc/external/mit/xf86-video-ati/include/config.h			patch
 * xsrc/external/mit/xf86-video-ati/dist/src/local_xf86Rename.h		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/radeon_dga.c			remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Crtc.c		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Crtc.h		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Cursors.c		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86DiDGA.c		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86EdidModes.c		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Modes.c		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Modes.h		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86RandR12.c		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86RandR12.h		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Rename.h		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86Rotate.c		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/modes/xf86cvt.c		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/parser/xf86Optrec.h		remove
 * xsrc/external/mit/xf86-video-ati/dist/src/parser/xf86Parser.h		remove
 *
 * add more support for hd3000/4000 and some for hd5000
 *
 * Revision 1.1.1.2  2010/11/20 10:00:26  mrg
 * initial import of xf86-video-ati-6.13.2
 *
 * Revision 1.4  2005/08/28 18:00:23  bogdand
 * Modified the licens type from GPL to a X/MIT one
 *
 * Revision 1.3  2005/07/11 02:29:45  ajax
 * Prep for modular builds by adding guarded #include "config.h" everywhere.
 *
 * Revision 1.2  2005/07/01 22:43:11  daniels
 * Change all misc.h and os.h references to <X11/foo.h>.
 *
 *
 ************************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include "xf86.h"
#include "generic_bus.h"
#include "theatre.h"
#include "theatre_reg.h"
#include "theatre_detect.h"

static Bool theatre_read(TheatrePtr t,uint32_t reg, uint32_t *data)
{
   if(t->theatre_num<0)return FALSE;
   return t->VIP->read(t->VIP, ((t->theatre_num & 0x3)<<14) | reg,4, (uint8_t *) data);
}

/* Unused code - reference */
#if 0
static Bool theatre_write(TheatrePtr t,uint32_t reg, uint32_t data)
{
   if(t->theatre_num<0)return FALSE;
   return t->VIP->write(t->VIP,((t->theatre_num & 0x03)<<14) | reg,4, (uint8_t *) &data);
}
#define RT_regw(reg,data)	theatre_write(t,(reg),(data))
#endif

#define RT_regr(reg,data)	theatre_read(t,(reg),(data))
#define VIP_TYPE      "ATI VIP BUS"


_X_EXPORT TheatrePtr DetectTheatre(GENERIC_BUS_Ptr b)
{
   TheatrePtr t;  
   int i;
   uint32_t val;
   char s[20];
   
   b->ioctl(b,GB_IOCTL_GET_TYPE,20,s);
   if(strcmp(VIP_TYPE, s)){
   xf86DrvMsg(b->scrnIndex, X_ERROR, "DetectTheatre must be called with bus of type \"%s\", not \"%s\"\n",
          VIP_TYPE, s);
   return NULL;
   }
   
   t = calloc(1,sizeof(TheatreRec));
   t->VIP = b;
   t->theatre_num = -1;
   t->mode=MODE_UNINITIALIZED;

   b->read(b, VIP_VIP_VENDOR_DEVICE_ID, 4, (uint8_t *)&val);
   for(i=0;i<4;i++)
   {
	if(b->read(b, ((i & 0x03)<<14) | VIP_VIP_VENDOR_DEVICE_ID, 4, (uint8_t *)&val))
        {
	  if(val)xf86DrvMsg(b->scrnIndex, X_INFO,
			    "Device %d on VIP bus ids as 0x%08x\n", i,
			    (unsigned)val);
	  if(t->theatre_num>=0)continue; /* already found one instance */
	  switch(val){
	  	case RT100_ATI_ID:
	           t->theatre_num=i;
		   t->theatre_id=RT100_ATI_ID;
		   break;
		case RT200_ATI_ID:
	           t->theatre_num=i;
		   t->theatre_id=RT200_ATI_ID;
		   break;
                }
	} else {
	  xf86DrvMsg(b->scrnIndex, X_INFO, "No response from device %d on VIP bus\n",i);	
	}
   }
   if(t->theatre_num>=0)xf86DrvMsg(b->scrnIndex, X_INFO,
				   "Detected Rage Theatre as device %d on VIP bus with id 0x%08x\n",
				   t->theatre_num, (unsigned)t->theatre_id);

   if(t->theatre_num < 0)
   {
   free(t);
   return NULL;
   }

   RT_regr(VIP_VIP_REVISION_ID, &val);
   xf86DrvMsg(b->scrnIndex, X_INFO, "Detected Rage Theatre revision %8.8X\n",
	      (unsigned)val);

#if 0
DumpRageTheatreRegsByName(t);
#endif
	
   return t;
}

