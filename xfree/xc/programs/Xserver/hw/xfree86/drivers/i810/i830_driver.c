/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/i810/i830_driver.c,v 1.7 2002/01/08 18:59:29 dawes Exp $ */
/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.


**************************************************************************/

/* Author: Jeff Hartmann <jhartmann@valinux.com>
 */

/* Heavily based on the VESA driver written by:
 * Paulo CÅÈsar Pereira de Andrade <pcpa@conectiva.com.br>
 */

/*
 * Changes:
 *
 *    23/08/2001 Abraham van der Merwe <abraham@2d3d.co.za>
 *        - Fixed display timing bug (mode information for some
 *          modes were not initialized correctly)
 *        - Added workarounds for GTT corruptions (I don't adjust
 *          the pitches for 1280x and 1600x modes so we don't
 *          need extra memory)
 *        - The code will now default to 60Hz if LFP is connected
 *        - Added different refresh rate setting code to work
 *          around 0x4f02 BIOS bug
 *        - BIOS workaround for some mode sets (I use legacy BIOS
 *          calls for setting those)
 *        - Removed 0x4f04, 0x01 (save state) BIOS call which causes
 *          LFP to malfunction (do some house keeping and restore
 *          modes ourselves instead - not perfect, but at least the
 *          LFP is working now)
 *        - Several other smaller bug fixes
 *
 *    06/09/2001 Abraham van der Merwe <abraham@2d3d.co.za>
 *        - Preliminary local memory support (works without agpgart)
 *        - DGA fixes (the code were still using i810 mode sets, etc.)
 *        - agpgart fixes
 *
 *    18/09/2001
 *        - Proper local memory support (should work correctly now
 *          with/without agpgart module)
 *        - more agpgart fixes
 *        - got rid of incorrect GTT adjustments
 *
 *    09/10/2001
 *        - Changed the DPRINTF() variadic macro to an ANSI C compatible
 *          version
 *
 *    10/10/2001
 *        - Fixed DPRINTF_stub(). I forgot the __...__ macros in there
 *          instead of the function arguments :P
 *        - Added a workaround for the 1600x1200 bug (Text mode corrupts
 *          when you exit from any 1600x1200 mode and 1280x1024@85Hz. I
 *          suspect this is a BIOS bug (hence the 1280x1024@85Hz case).
 *          For now I'm switching to 800x600@60Hz then to 80x25 text mode
 *          and then restoring the registers - very ugly indeed.
 *
 *    15/10/2001
 *        - Improved 1600x1200 mode set workaround. The previous workaround
 *          was causing mode set problems later on.
 *
 *    18/10/2001
 *        - Fixed a bug in I830BIOSLeaveVT() which caused a bug when you
 *          switched VT's
 */

#include "xf86.h"
#include "xf86_ansic.h"
#include "xf86_OSproc.h"
#include "xf86Resources.h"
#include "xf86RAC.h"
#include "xf86cmap.h"
#include "compiler.h"
#include "mibstore.h"
#include "vgaHW.h"
#include "mipointer.h"
#include "micmap.h"

#include "fb.h"
#include "miscstruct.h"
#include "xf86xv.h"
#include "Xv.h"
#include "vbe.h"

#include "i810.h"

#ifdef XF86DRI
#include "dri.h"
#endif

#define BIT(x) (1 << (x))
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define NB_OF(x) (sizeof (x) / sizeof (*x))

static SymTabRec I830BIOSChipsets[] = {
   { PCI_CHIP_I830_M,	  "i830"},
   { -1, NULL }
};

static PciChipsets I830BIOSPciChipsets[] = {
   { PCI_CHIP_I830_M,	  PCI_CHIP_I830_M,	       RES_SHARED_VGA },
   { -1, -1, RES_UNDEFINED }
};

static OptionInfoRec I830BIOSOptions[] = {
   { OPTION_NOACCEL, "NoAccel", OPTV_BOOLEAN, {0}, FALSE },
   { OPTION_SW_CURSOR, "SWcursor", OPTV_BOOLEAN, {0}, FALSE },
   { OPTION_CACHE_LINES, "CacheLines", OPTV_INTEGER, {0}, FALSE},
   { OPTION_DRI, "DRI", OPTV_BOOLEAN, {0}, TRUE},
   { OPTION_STRETCH, "Stretch", OPTV_BOOLEAN, {0}, FALSE},
   { OPTION_CENTER, "Center", OPTV_BOOLEAN, {0}, FALSE},
   { -1, NULL, OPTV_NONE, {0}, FALSE}
};


static VBEInfoBlock *I830VESAGetVBEInfo(ScrnInfoPtr pScrn);
static Bool I830BIOSGetRec(ScrnInfoPtr pScrn);
static void I830BIOSFreeRec(ScrnInfoPtr pScrn);
static void SaveFonts(ScrnInfoPtr pScrn);
static void RestoreFonts(ScrnInfoPtr pScrn);
static Bool I830VESASetMode(ScrnInfoPtr pScrn, DisplayModePtr pMode);
static void I830DisplayPowerManagementSet(ScrnInfoPtr pScrn,
					  int PowerManagementMode,
					  int flags);

#ifdef I830DEBUG
void DPRINTF_stub (const char *filename,int line,const char *function,const char *fmt, ...) {
	va_list ap;
	fprintf (stderr,
			 "\n##############################################\n"
			 "*** In function %s, on line %d, in file %s ***\n",
			 function,line,filename);
	va_start (ap,fmt);
	vfprintf (stderr,fmt,ap);
	va_end (ap);
	fprintf (stderr,
			 "##############################################\n\n");
	fflush (stderr);
}
#else	/* #ifdef I830DEBUG */
void DPRINTF_stub (const char *filename,int line,const char *function,const char *fmt, ...) {
	/* do nothing */
}
#endif	/* #ifdef I830DEBUG */

const OptionInfoRec *
I830BIOSAvailableOptions(int chipid, int busid)
{
   int i;

   for(i = 0; I830BIOSPciChipsets[i].PCIid > 0; i++) {
      if(chipid == I830BIOSPciChipsets[i].PCIid) return I830BIOSOptions;
   }
   return NULL;
}

static VBEInfoBlock *
I830VESAGetVBEInfo(ScrnInfoPtr pScrn)
{
   I810Ptr pI810;
   VESAPtr pVesa;
   VBEInfoBlock *block = NULL;
   int i, pStr, pModes;
   char *str;
   CARD16 major, minor, *modes;

   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;

   bzero(pVesa->block, sizeof(VBEInfoBlock));

   pVesa->block[0] = 'V';
   pVesa->block[1] = 'B';
   pVesa->block[2] = 'E';
   pVesa->block[3] = '2';

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x4f00;
   pVesa->pInt->es = SEG_ADDR(pVesa->page);
   pVesa->pInt->di = SEG_OFF(pVesa->page);
   xf86ExecX86int10_wrapper(pVesa->pInt, pScrn);

   if(pVesa->pInt->ax != 0x4f) return NULL;

   block = xcalloc(sizeof(VBEInfoBlock), 1);
   block->VESASignature[0] = pVesa->block[0];
   block->VESASignature[1] = pVesa->block[1];
   block->VESASignature[2] = pVesa->block[2];
   block->VESASignature[3] = pVesa->block[3];

   block->VESAVersion = *(CARD16*)(pVesa->block + 4);
   major = (unsigned)block->VESAVersion >> 8;
   minor = block->VESAVersion & 0xff;

   pStr = *(CARD32*)(pVesa->block + 6);
   str = xf86int10Addr(pVesa->pInt, FARP(pStr));
   block->OEMStringPtr = strdup(str);

   block->Capabilities[0] = pVesa->block[10];
   block->Capabilities[1] = pVesa->block[11];
   block->Capabilities[2] = pVesa->block[12];
   block->Capabilities[3] = pVesa->block[13];

   pModes = *(CARD32*)(pVesa->block + 14);
   modes = xf86int10Addr(pVesa->pInt, FARP(pModes));
   i = 0;
   while(modes[i] != 0xffff) i++;
   block->VideoModePtr = xalloc(sizeof(CARD16) * i + 1);
   memcpy(block->VideoModePtr, modes, sizeof(CARD16) * i);
   block->VideoModePtr[i] = 0xffff;

   block->TotalMemory = *(CARD16*)(pVesa->block + 18);

   if(major < 2) memcpy(&block->OemSoftwareRev, pVesa->block + 20, 236);
   else {
      block->OemSoftwareRev = *(CARD16*)(pVesa->block + 20);
      pStr = *(CARD32*)(pVesa->block + 22);
      str = xf86int10Addr(pVesa->pInt, FARP(pStr));
      block->OemVendorNamePtr = strdup(str);
      pStr = *(CARD32*)(pVesa->block + 26);
      str = xf86int10Addr(pVesa->pInt, FARP(pStr));
      block->OemProductNamePtr = strdup(str);
      pStr = *(CARD32*)(pVesa->block + 30);
      str = xf86int10Addr(pVesa->pInt, FARP(pStr));
      block->OemProductRevPtr = strdup(str);
      memcpy(&block->Reserved, pVesa->block + 34, 222);
      memcpy(&block->OemData, pVesa->block + 256, 256);
   }

   return block;
}

ModeInfoBlock * I830VESAGetModeInfo (ScrnInfoPtr pScrn,int mode)
{
   I810Ptr pI810;
   VESAPtr pVesa;
   ModeInfoBlock *block = NULL;

   pI810 = I810PTR (pScrn);
   pVesa = pI810->vesa;

   bzero (pVesa->block,sizeof (ModeInfoBlock));

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x4f01;
   pVesa->pInt->cx = mode;
   pVesa->pInt->es = SEG_ADDR (pVesa->page);
   pVesa->pInt->di = SEG_OFF (pVesa->page);
   xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
   if (pVesa->pInt->ax != 0x4f) return (NULL);

   block = xcalloc (sizeof (ModeInfoBlock),1);

   block->ModeAttributes = *(CARD16 *) pVesa->block;
   block->WinAAttributes = pVesa->block[2];
   block->WinBAttributes = pVesa->block[3];
   block->WinGranularity = *(CARD16 *) (pVesa->block + 4);
   block->WinSize = *(CARD16 *) (pVesa->block + 6);
   block->WinASegment = *(CARD16 *) (pVesa->block + 8);
   block->WinBSegment = *(CARD16 *) (pVesa->block + 10);
   block->WinFuncPtr = *(CARD32 *) (pVesa->block + 12);
   block->BytesPerScanline = *(CARD16 *) (pVesa->block + 16);

   /* mandatory information for VBE 1.2 and above */
   block->XResolution = *(CARD16 *) (pVesa->block + 18);
   block->YResolution = *(CARD16 *) (pVesa->block + 20);
   block->XCharSize = pVesa->block[22];
   block->YCharSize = pVesa->block[23];
   block->NumberOfPlanes = pVesa->block[24];
   block->BitsPerPixel = pVesa->block[25];
   block->NumberOfBanks = pVesa->block[26];
   block->MemoryModel = pVesa->block[27];
   block->BankSize = pVesa->block[28];
   block->NumberOfImages = pVesa->block[29];
   block->Reserved = pVesa->block[30];

   /* Direct color fields (required for direct/6 and YUV/7 memory models) */
   block->RedMaskSize = pVesa->block[31];
   block->RedFieldPosition = pVesa->block[32];
   block->GreenMaskSize = pVesa->block[33];
   block->GreenFieldPosition = pVesa->block[34];
   block->BlueMaskSize = pVesa->block[35];
   block->BlueFieldPosition = pVesa->block[36];
   block->RsvdMaskSize = pVesa->block[37];
   block->RsvdFieldPosition = pVesa->block[38];
   block->DirectColorModeInfo = pVesa->block[39];

   /* Mandatory information for VBE 2.0 and above */
   if(pVesa->major >= 2)
	 {
		block->PhysBasePtr = *(CARD32 *) (pVesa->block + 40);
		block->Reserved32 = *(CARD32 *) (pVesa->block + 44);
		block->Reserved16 = *(CARD16 *) (pVesa->block + 48);

		/* Mandatory information for VBE 3.0 and above */
		if (pVesa->major >= 3)
		  {
			 block->LinBytesPerScanLine = *(CARD16 *) (pVesa->block + 50);
			 block->BnkNumberOfImagePages = pVesa->block[52];
			 block->LinNumberOfImagePages = pVesa->block[53];
			 block->LinRedMaskSize = pVesa->block[54];
			 block->LinRedFieldPosition = pVesa->block[55];
			 block->LinGreenMaskSize = pVesa->block[56];
			 block->LinGreenFieldPosition = pVesa->block[57];
			 block->LinBlueMaskSize = pVesa->block[58];
			 block->LinBlueFieldPosition = pVesa->block[59];
			 block->LinRsvdMaskSize = pVesa->block[60];
			 block->LinRsvdFieldPosition = pVesa->block[61];
			 block->MaxPixelClock = *(CARD32 *) (pVesa->block + 62);
			 memcpy (&block->Reserved2,pVesa->block + 66,188);
		  }
		else memcpy (&block->LinBytesPerScanLine,pVesa->block + 50,206);
	 }
   else memcpy (&block->PhysBasePtr,pVesa->block + 40,216);

   return (block);
}

static Bool
I830BIOSGetRec(ScrnInfoPtr pScrn)
{
   I810Ptr pI810;

   if(pScrn->driverPrivate) return TRUE;
   pI810 = pScrn->driverPrivate = xnfcalloc(sizeof(I810Rec), 1);
   pI810->vesa = xcalloc(sizeof(VESARec), 1);
   return TRUE;
}

static void
I830BIOSFreeRec(ScrnInfoPtr pScrn)
{
   I810Ptr pI810;
   VESAPtr pVesa;
   DisplayModePtr mode;

   if(!pScrn) return;
   if(!pScrn->driverPrivate) return;

   pI810 = I810PTR(pScrn);
   mode = pScrn->modes;
   pVesa = pI810->vesa;

   if (mode) {
      do {
	 if(mode->Private) {
	    ModeInfoData *data = (ModeInfoData*)mode->Private;

	    if(data->block) xfree(data->block);
	    xfree(data);
	    mode->Private = NULL;
	 }
	 mode = mode->next;
      } while(mode && mode != pScrn->modes);
   }

   if(pVesa->monitor) xfree(pVesa->monitor);
   if(pVesa->vbeInfo) xfree(pVesa->vbeInfo);
   if(pVesa->pal) xfree(pVesa->pal);
   if(pVesa->savedPal) xfree(pVesa->savedPal);
   if(pVesa->fonts) xfree(pVesa->fonts);
   xfree(pScrn->driverPrivate);
   pScrn->driverPrivate=0;
}

void
I830BIOSProbeDDC(ScrnInfoPtr pScrn, int index)
{
   vbeInfoPtr pVbe;
   
   if (xf86LoadSubModule(pScrn, "vbe")) {
      pVbe = VBEInit(NULL,index);
      ConfiguredMonitor = vbeDoEDID(pVbe, NULL);
   }
}

void
I830VESAFreeModeInfo(ModeInfoBlock *block)
{
   xfree(block);
}

static Bool
I830DetectDisplayDevice(ScrnInfoPtr pScrn)
{
   I810Ptr pI810;
   VESAPtr pVesa;

   pI810 = I810PTR (pScrn);
   pVesa = pI810->vesa;

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x5f64;
   pVesa->pInt->bx = 0x0100;
   xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);

   if (pVesa->pInt->ax != 0x005f)
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"Failed to detect active display devices\n");
		return (FALSE);
	 }

   pI810->configured_device = pVesa->pInt->cx;

#define PIPEA_CRT_ACTIVE	0x01
#define PIPEA_LCD_ACTIVE	0x08
   /* Anything on Pipe A? */
   if (pVesa->pInt->cx & 0xff)
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"Currently active displays on Pipe A:\n");

		if (pVesa->pInt->cx & PIPEA_CRT_ACTIVE)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    CRT\n");

		if (pVesa->pInt->cx & 0x02)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    TV child device\n");

		if (pVesa->pInt->cx & 0x04)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    DFP child device\n");

		if (pVesa->pInt->cx & PIPEA_LCD_ACTIVE)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    LFP (Local Flat Panel) child device\n");

		if (pVesa->pInt->cx & 0xf0)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    Some unknown display devices may also be present\n");
	 }

#define PIPEB_CRT_ACTIVE	0x0100
#define PIPEB_LCD_ACTIVE	0x0800
   /* Anything on Pipe B? */
   if (pVesa->pInt->cx & 0xff00)
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"Currently active displays on Pipe B:\n");

		if (pVesa->pInt->cx & PIPEB_CRT_ACTIVE)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    CRT\n");

		if (pVesa->pInt->cx & 0x0200)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    TV child device\n");

		if (pVesa->pInt->cx & 0x0400)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    DFP child device\n");

		if (pVesa->pInt->cx & PIPEB_LCD_ACTIVE)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    LFP (Local Flat Panel) child device\n");

		if (pVesa->pInt->cx & 0xf000)
		  xf86DrvMsg (pScrn->scrnIndex,X_INFO,"    Some unknown display devices may also be present\n");
	 }

   return (TRUE);
}

static int I830DetectMemory (ScrnInfoPtr pScrn)
{
   I810Ptr pI810;
   VESAPtr pVesa;
   PCITAG bridge;
   CARD16 gmch_ctrl;
   int memsize;

   pI810 = I810PTR (pScrn);
   pVesa = pI810->vesa;

   bridge = pciTag (0,0,0);	/* This is always the host bridge */
   gmch_ctrl = pciReadWord (bridge,I830_GMCH_CTRL);

   switch (gmch_ctrl & I830_GMCH_GMS_MASK)
	 {
	  case I830_GMCH_GMS_STOLEN_512:
		memsize = KB (512);
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"detected %dK stolen memory.\n",memsize / 1024);
		break;
	  case I830_GMCH_GMS_STOLEN_1024:
		memsize = MB (1);
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"detected %dK stolen memory.\n",memsize / 1024);
		break;
	  case I830_GMCH_GMS_STOLEN_8192:
		memsize = MB (8);
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"detected %dK stolen memory.\n",memsize / 1024);
		break;
	  case I830_GMCH_GMS_LOCAL:
		/* I'd like to use the VGA controller registers here, but MMIOBase isn't
		 * yet, so for now, we'll just use the BIOS instead... */
		pVesa->pInt->num = 0x10;
		pVesa->pInt->ax = 0x5f10;
		xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
		memsize = pVesa->pInt->cx * KB (1);
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"detected %dK local memory.\n",memsize / 1024);
		break;
	  default:
		/* not that this is possible, but anyway (: */
		memsize = 0;
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"no video memory detected.\n");
	 }

   return (memsize);
}

Bool I830BIOSPreInit (ScrnInfoPtr pScrn,int flags)
{
   vgaHWPtr hwp;
   I810Ptr pI810;
   MessageType from;
   rgb defaultWeight = { 0, 0, 0 };
   VESAPtr pVesa;
   VBEInfoBlock *vbe;
   DisplayModePtr pMode,tmp;
   ModeInfoBlock *mode;
   ModeInfoData *data = NULL;
   vbeInfoPtr pVbe;
   pointer pVbeModule,pDDCModule;
   int mem;
   int flags24;
   int i = 0;

   if(pScrn->numEntities != 1) return (FALSE);

   /* The vgahw module should be loaded here when needed */
   if (!xf86LoadSubModule (pScrn,"vgahw")) return (FALSE);
   xf86LoaderReqSymLists (I810vgahwSymbols,NULL);

    /* Load int10 module */
   if (!xf86LoadSubModule (pScrn,"int10")) return (FALSE);
   xf86LoaderReqSymLists (I810int10Symbols,NULL);

   /* Allocate a vgaHWRec */
   if (!vgaHWGetHWRec (pScrn)) return (FALSE);

   /* Allocate driverPrivate */
   if (!I830BIOSGetRec (pScrn)) return (FALSE);

   pI810 = I810PTR (pScrn);
   pI810->pEnt = xf86GetEntityInfo (pScrn->entityList[0]);

   if (pI810->pEnt->location.type != BUS_PCI) return (FALSE);

   if (flags & PROBE_DETECT)
	 {
		I830BIOSProbeDDC (pScrn,pI810->pEnt->index);
		return (TRUE);
	 }

   pI810->PciInfo = xf86GetPciInfoForEntity (pI810->pEnt->index);
   pI810->PciTag = pciTag (pI810->PciInfo->bus,pI810->PciInfo->device,pI810->PciInfo->func);

   if (xf86RegisterResources (pI810->pEnt->index,0,ResNone))
	 return (FALSE);

   pScrn->racMemFlags = RAC_FB | RAC_COLORMAP;
   pScrn->monitor = pScrn->confScreen->monitor;
   pScrn->progClock = TRUE;
   pScrn->rgbBits = 8;

   flags24 = Support32bppFb | PreferConvert24to32 | SupportConvert24to32;

   if (!xf86SetDepthBpp (pScrn,8,8,8,flags24)) return (FALSE);
   switch (pScrn->depth)
	 {
      case 8:
      case 15:
      case 16:
      case 24:
		break;
      default:
		xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Given depth (%d) is not supported by I830 driver\n",pScrn->depth);
		return (FALSE);
	 }
   xf86PrintDepthBpp (pScrn);

   if (!xf86SetWeight (pScrn,defaultWeight,defaultWeight))
	 return (FALSE);
   if (!xf86SetDefaultVisual (pScrn,-1))
	 return (FALSE);

   hwp = VGAHWPTR (pScrn);
   pI810->cpp = pScrn->bitsPerPixel / 8;

   /* Process the options */
   xf86CollectOptions (pScrn,NULL);
   xf86ProcessOptions (pScrn->scrnIndex,pScrn->options,I830BIOSOptions);

   /* We have to use PIO to probe, because we haven't mapped yet */
   I810SetPIOAccess (pI810);

   pVesa = pI810->vesa;

   /* Initialize Vesa record */

   if ((pVesa->pInt = xf86InitInt10(pI810->pEnt->index)) == NULL)
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Int10 initialization failed.\n");
		return (FALSE);
	 }
   if ((pVesa->block = xf86Int10AllocPages (pVesa->pInt,1,&pVesa->page)) == NULL)
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Cannot allocate one scratch page in real mode memory.\n");
		return (FALSE);
	 }

   /* Set the Chipset and ChipRev, allowing config file entries to override. */
   if (pI810->pEnt->device->chipset && *pI810->pEnt->device->chipset)
	 {
		pScrn->chipset = pI810->pEnt->device->chipset;
		from = X_CONFIG;
	 }
   else if (pI810->pEnt->device->chipID >= 0)
	 {
		pScrn->chipset = (char *) xf86TokenToString (I830BIOSChipsets,pI810->pEnt->device->chipID);
		from = X_CONFIG;
		xf86DrvMsg (pScrn->scrnIndex,X_CONFIG,"ChipID override: 0x%04X\n",pI810->pEnt->device->chipID);
	 }
   else
	 {
		from = X_PROBED;
		pScrn->chipset = (char *) xf86TokenToString (I830BIOSChipsets,pI810->PciInfo->chipType);
	 }

   if (pI810->pEnt->device->chipRev >= 0)
	 xf86DrvMsg (pScrn->scrnIndex,X_CONFIG,"ChipRev override: %d\n",pI810->pEnt->device->chipRev);

   xf86DrvMsg (pScrn->scrnIndex,from,"Chipset: \"%s\"\n",(pScrn->chipset != NULL) ? pScrn->chipset : "Unknown i810");

   if (pI810->pEnt->device->MemBase != 0)
	 {
		pI810->LinearAddr = pI810->pEnt->device->MemBase;
		from = X_CONFIG;
	 }
   else
	 {
		if (pI810->PciInfo->memBase[1] != 0)
		  {
			 pI810->LinearAddr = pI810->PciInfo->memBase[0] & 0xFF000000;
			 from = X_PROBED;
		  }
		else
		  {
			 xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"No valid FB address in PCI config space\n");
			 I830BIOSFreeRec (pScrn);
			 return (FALSE);
		  }
	 }

   xf86DrvMsg (pScrn->scrnIndex,from,"Linear framebuffer at 0x%lX\n",(unsigned long) pI810->LinearAddr);

   if (pI810->pEnt->device->IOBase != 0)
	 {
		pI810->MMIOAddr = pI810->pEnt->device->IOBase;
		from = X_CONFIG;
	 }
   else
	 {
		if (pI810->PciInfo->memBase[1])
		  {
			 pI810->MMIOAddr = pI810->PciInfo->memBase[1] & 0xFFF80000;
			 from = X_PROBED;
		  }
		else
		  {
			 xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"No valid MMIO address in PCI config space\n");
			 I830BIOSFreeRec (pScrn);
			 return (FALSE);
		  }
	 }

   xf86DrvMsg (pScrn->scrnIndex,from,"IO registers at addr 0x%lX\n",(unsigned long) pI810->MMIOAddr);

	 {
		PCITAG bridge;
		CARD16 gmch_ctrl;

		bridge = pciTag (0,0,0); /* This is always the host bridge */
		gmch_ctrl = pciReadWord (bridge,I830_GMCH_CTRL);
		if ((gmch_ctrl & I830_GMCH_MEM_MASK) == I830_GMCH_MEM_128M)
		  {
			 pI810->FbMapSize = 0x7000000;
			 pI810->DepthOffset = 0x7000000;
			 pI810->BackOffset = 0x7800000;
		  }
		else
		  {
			 pI810->FbMapSize = 0x3000000;
			 pI810->DepthOffset = 0x3000000;
			 pI810->BackOffset = 0x3800000;
		  }
	 }

   /* FIXME: Get rid of StolenSize/StolenOnly */
   pI810->StolenSize = I830DetectMemory (pScrn);

   /* Default to stolen/local memory size or 8MB whichever is bigger */
   if (!pI810->pEnt->device->videoRam)
	 {
		from = X_DEFAULT;
		pScrn->videoRam = pI810->StolenSize / 1024;
		if (pScrn->videoRam < 8192) pScrn->videoRam = 8192;
	 }
   else
	 {
		from = X_DEFAULT;
		pScrn->videoRam = pI810->pEnt->device->videoRam;
	 }

   mem = I810CheckAvailableMemory (pScrn);
   pI810->StolenOnly = FALSE;

   DPRINTF (PFX,
			"Available memory: %dk\n"
			"Requested memory: %dk (see XF86Config-4)\n",
			mem,pScrn->videoRam);

   if (mem < 0)
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_WARNING,
					"/dev/agpgart is either not available, or no memory "
					"is available\nfor allocation. Using stolen memory only.\n");
		pScrn->videoRam = pI810->StolenSize / 1024;
		pI810->StolenOnly = TRUE;
	 }

   xf86DrvMsg (pScrn->scrnIndex,from,"Will alloc AGP framebuffer: %d kByte\n",pScrn->videoRam);

   /*
	* If the driver can do gamma correction, it should call xf86SetGamma() here.
    */

	 {
		Gamma zeros = { 0.0, 0.0, 0.0 };

		if (!xf86SetGamma (pScrn,zeros)) return (FALSE);
	 }

   if (!I830DetectDisplayDevice (pScrn))
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Couldn't detect display devices.\n");
		return (FALSE);
	 }

   vbe = I830VESAGetVBEInfo(pScrn);
   pVesa->major = (unsigned)(vbe->VESAVersion >> 8);
   pVesa->minor = vbe->VESAVersion & 0xff;
   pVesa->vbeInfo = vbe;

   /* Load vbe module */
   if((pVbeModule = xf86LoadSubModule(pScrn, "vbe")) == NULL) return FALSE;
   xf86LoaderReqSymLists(I810vbeSymbols, NULL);
   if((pVbe = VBEInit(pVesa->pInt, pVesa->pEnt->index)) == NULL) return FALSE;

   /* Load ddc module */
   if((pDDCModule = xf86LoadSubModule(pScrn, "ddc")) == NULL) return FALSE;
   xf86LoaderReqSymLists(I810ddcSymbols, NULL);

   if((pVesa->monitor = vbeDoEDID(pVbe, pDDCModule)) != NULL) {
      xf86PrintEDID(pVesa->monitor);
   }

   /* unload modules */
   xf86UnloadSubModule(pVbeModule);
   xf86UnloadSubModule(pDDCModule);
   /* Set display resolution */
   xf86SetDpi(pScrn, 0, 0);

#define VIDEO_BIOS_SCRATCH 0x18

   /* Enable hot keys by writing the proper value to GR18 */
   {
      CARD8 gr18;
      gr18 = pI810->readControl(pI810, GRX, VIDEO_BIOS_SCRATCH);
      gr18 &= ~0x80; /* Clear Hot key bit so that Video BIOS performs
		      * the hot key servicing */
      pI810->writeControl(pI810, GRX, VIDEO_BIOS_SCRATCH, gr18);
   }

   if((pScrn->monitor->DDC = pVesa->monitor) != NULL)
     xf86SetDDCproperties(pScrn, pVesa->monitor);

   /* New way of processing video modes */
   for (i = 0; i <= 0x7f; i++)
	 {
		pVesa->pInt->num = 0x10;
		pVesa->pInt->ax = 0x5f28;
		pVesa->pInt->bx = 0x8000 | i;
		pVesa->pInt->cx = 0x8000; /* Current display device */
		xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);

		if(pVesa->pInt->ax != 0x005f) continue;

		if ((mode = I830VESAGetModeInfo (pScrn,i)) == NULL) continue;

		if (!(mode->ModeAttributes & (1 << 0)) ||	/* supported in the configured hardware */
			!(mode->ModeAttributes & (1 << 4)) ||	/* text mode */
			(pScrn->bitsPerPixel != 1 && !(mode->ModeAttributes & (1 << 3))) || /* monochrome */
			(mode->BitsPerPixel > 8 && (mode->RedMaskSize + mode->GreenMaskSize + mode->BlueMaskSize != pScrn->depth)) ||
			/* only linear mode, but no PhysBasePtr */
			((mode->ModeAttributes & (1 << 6)) && (mode->ModeAttributes & (1 << 7)) && !mode->PhysBasePtr) ||
			((mode->ModeAttributes & (1 << 6)) && !(mode->ModeAttributes & (1 << 7))) || mode->BitsPerPixel != pScrn->bitsPerPixel)
		  {
			 I830VESAFreeModeInfo (mode);
			 continue;
		  }

		pMode = xcalloc (sizeof (DisplayModeRec),1);
		pMode->prev = pMode->next = NULL;

		pMode->status = MODE_OK;
		pMode->type = M_T_DEFAULT;	/*M_T_BUILTIN;*/

		/* for adjust frame */
		pMode->HDisplay = mode->XResolution;
		pMode->VDisplay = mode->YResolution;

		data = xcalloc (sizeof (ModeInfoData),1);
		data->mode = i;
		data->data = mode;
		pMode->PrivSize = sizeof (ModeInfoData);
		pMode->Private = (INT32*) data;

		if (pScrn->modePool == NULL)
		  {
			 pScrn->modePool = pMode;
			 pMode->next = pMode->prev = pMode;
		  }
		else
		  {
			 tmp = pScrn->modePool;
			 tmp->prev = pMode;
			 while (tmp->next != pScrn->modePool) tmp = tmp->next;
			 tmp->next = pMode;
			 pMode->prev = tmp;
			 pMode->next = pScrn->modePool;
		  }
	 }

   if (pScrn->modePool == NULL) return FALSE;

   for (i = 0; pScrn->modePool != NULL && pScrn->display->modes[i] != NULL; i++)
	 {
		pMode = pScrn->modePool;

		do
		  {
			 DisplayModePtr next = pMode->next;
			 int width, height;

			 if (sscanf (pScrn->display->modes[i],"%dx%d",&width,&height) == 2 &&
				 width == pMode->HDisplay && height == pMode->VDisplay)
			   {
				  pMode->name = strdup(pScrn->display->modes[i]);

				  pMode->prev->next = pMode->next;
				  pMode->next->prev = pMode->prev;

				  if (pScrn->modes == NULL)
					{
					   pScrn->modes = pMode;
					   pMode->next = pMode->prev = pMode;
					}
				  else
					{
					   tmp = pScrn->modes;
					   tmp->prev = pMode;
					   while (tmp->next != pScrn->modes) tmp = tmp->next;
					   pMode->prev = tmp;
					   tmp->next = pMode;
					   pMode->next = pScrn->modes;
					}

				  if (pMode == pScrn->modePool)
					pScrn->modePool = next == pMode ? NULL : next;
				  break;
			   }

			 pMode = next;
		  }
		while (pMode != pScrn->modePool && pScrn->modePool != NULL);
	 }

   if (pScrn->modes == NULL)
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"No matching modes found in configuration file.\n");
		xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Only the following modes are supported by the BIOS in this bpp:\n");
		pMode = pScrn->modePool;
		do
		  {
			 data = (ModeInfoData *) pMode->Private;
			 mode = data->data;
			 xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"%dx%d\n",mode->XResolution,mode->YResolution);
			 pMode = pMode->next;
		  }
		while (pMode != pScrn->modePool);

		xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Please choose one of these modes.\n");

		return (FALSE);
	 }

   tmp = pScrn->modes;
   do
	 {
		mode = ((ModeInfoData *) tmp->Private)->data;
		if (mode->XResolution > pScrn->virtualX)
		  {
			 pScrn->virtualX = mode->XResolution;
			 pVesa->maxBytesPerScanline = mode->BytesPerScanline;
		  }
		if (mode->YResolution > pScrn->virtualY)
		  pScrn->virtualY = mode->YResolution;
	 }
   while ((tmp = tmp->next) != pScrn->modes);

   if (pVesa->monitor != NULL)
	 {
		DisplayModePtr tmp;
		int dotclock = 0;
		float hsync,vrefresh;

		pMode = pScrn->modes;
		do
		  {
			 for (tmp = pScrn->monitor->Modes; tmp != NULL; tmp = tmp->next)
			   if (!strcmp (pMode->name,tmp->name) && dotclock < tmp->Clock)
				 {
#ifdef I830DEBUG
					fprintf (stderr,"##############################################\n");
					  {
						 char flags[256];

						 *flags = '\0';

						 if (tmp->HSkew) sprintf (flags,"%s hskew %d",flags,tmp->HSkew);
						 if (tmp->VScan) sprintf (flags,"%s vscan %d",flags,tmp->VScan);
						 if (tmp->Flags & V_INTERLACE) strcat (flags," interlace");
						 if (tmp->Flags & V_CSYNC) strcat (flags," composite");
						 if (tmp->Flags & V_DBLSCAN) strcat (flags," doublescan");
						 if (tmp->Flags & V_BCAST) strcat (flags," bcast");
						 if (tmp->Flags & V_PHSYNC) strcat (flags," +hsync");
						 if (tmp->Flags & V_NHSYNC) strcat (flags," -hsync");
						 if (tmp->Flags & V_PVSYNC) strcat (flags," +vsync");
						 if (tmp->Flags & V_NVSYNC) strcat (flags," -vsync");
						 if (tmp->Flags & V_PCSYNC) strcat (flags," +csync");
						 if (tmp->Flags & V_NCSYNC) strcat (flags," -csync");
						 fprintf (stderr,
								  "Modeline \"%s\"  %6.2f  %d %d %d %d   %d %d %d %d%s\n",
								  tmp->name,
								  tmp->Clock / 1000.0,
								  tmp->HDisplay,tmp->HSyncStart,tmp->HSyncEnd,tmp->HTotal,
								  tmp->VDisplay,tmp->VSyncStart,tmp->VSyncEnd,tmp->VTotal,
								  flags);
					  }
					fprintf (stderr,"##############################################\n");
#endif

					pMode->Clock = tmp->Clock;
					pMode->HDisplay = tmp->HDisplay;
					pMode->HSyncStart = tmp->HSyncStart;
					pMode->HSyncEnd = tmp->HSyncEnd;
					pMode->HTotal = tmp->HTotal;
					pMode->HSkew = tmp->HSkew;
					pMode->VDisplay = tmp->VDisplay;
					pMode->VSyncStart = tmp->VSyncStart;
					pMode->VSyncEnd = tmp->VSyncEnd;
					pMode->VTotal = tmp->VTotal;
					pMode->VScan = tmp->VScan;
					pMode->Flags = tmp->Flags;

					dotclock = tmp->Clock;
				 }

			 data->mode |= 1 << 11;	/* FIXME: What is this used for? */

			 data = (ModeInfoData *) pMode->Private;
			 data->block = xcalloc (sizeof (CRTCInfoBlock),1);
			 data->block->HorizontalTotal = pMode->HTotal;
			 data->block->HorizontalSyncStart = pMode->HSyncStart;
			 data->block->HorizontalSyncEnd = pMode->HSyncEnd;
			 data->block->VerticalTotal = pMode->VTotal;
			 data->block->VerticalSyncStart = pMode->VSyncStart;
			 data->block->VerticalSyncEnd = pMode->VSyncEnd;
			 data->block->Flags = pMode->Flags;
			 data->block->PixelClock = pMode->Clock * 1000;

			 hsync = 0;
			 if (pMode->HSync > 0.0)
			   hsync = pMode->HSync;
			 else if (pMode->HTotal > 0)
			   hsync = (float) pMode->Clock / (float) pMode->HTotal;

			 vrefresh = 0;
			 if (pMode->VTotal > 0) vrefresh = hsync * 1000.0 / pMode->VTotal;
			 if (pMode->Flags & V_INTERLACE) vrefresh *= 2.0;
			 if (pMode->Flags & V_DBLSCAN) vrefresh /= 2.0;
			 if (pMode->VScan > 1) vrefresh /= pMode->VScan;
			 if (pMode->VRefresh > 0.0) vrefresh = pMode->VRefresh;

			 pMode->HSync = hsync;
			 pMode->VRefresh = vrefresh;

			 DPRINTF (PFX,
					  "pMode->HSync == %.1f KHz\n"
					  "pMode->VRefresh == %.1f Hz\n",
					  pMode->HSync,
					  pMode->VRefresh);

			 data->block->RefreshRate = vrefresh;

			 /* do some very primitive rounding to the nearest integer */
			 if ((vrefresh - data->block->RefreshRate) * 10.0 >= 5.0)
			   data->block->RefreshRate++;

			 data->block->RefreshRate *= 100;
		  }
		while (pMode != pScrn->modes);
	 }

   DPRINTF (PFX,
			"data->block->HorizontalTotal = %lu\n"
			"data->block->HorizontalSyncStart = %lu\n"
			"data->block->HorizontalSyncEnd = %lu\n"
			"data->block->VerticalTotal = %lu\n"
			"data->block->VerticalSyncStart = %lu\n"
			"data->block->VerticalSyncEnd = %lu\n"
			"data->block->Flags = %lu\n"
			"data->block->PixelClock = %lu\n"
			"data->block->RefreshRate = %lu\n",
			data->block->HorizontalTotal,
			data->block->HorizontalSyncStart,
			data->block->HorizontalSyncEnd,
			data->block->VerticalTotal,
			data->block->VerticalSyncStart,
			data->block->VerticalSyncEnd,
			data->block->Flags,
			data->block->PixelClock,
			data->block->RefreshRate);

   pScrn->currentMode = pScrn->modes;

#ifdef XF86DRI
   /* Need to find largest sized mode then pick the best pitch, this allows us to use tiled memory. */
	 {
		int maxXResolution = 0;
		int calc_pitch;
#if 0
		int i,i830_pitches[] = { 512, 1024, 2048, 4096, 8192 };
#endif

		pMode = pScrn->modes;
		do
		  {
			 if (pMode->HDisplay > maxXResolution) maxXResolution = pMode->HDisplay;
			 pMode = pMode->next;
		  }
		while (pMode != pScrn->modes);

		calc_pitch = 0;

/* FIXME: why isn't this working? */
#if 0
		for (i = 0; i < NB_OF (i830_pitches); i++)
		  if ((maxXResolution * pI810->cpp) <= i830_pitches[i])
			{
			   calc_pitch = i830_pitches[i] / pI810->cpp;
			   break;
			}
#endif

		pScrn->displayWidth = calc_pitch ? calc_pitch : pScrn->virtualX;
		if (pScrn->virtualY * pScrn->displayWidth * pI810->cpp > pI810->StolenSize)
		  {
			 xf86DrvMsg (pScrn->scrnIndex,X_WARNING,"Pitch is bigger than local/stolen size. Defaulting\n\tto original display width\n");
			 pScrn->displayWidth = pScrn->virtualX;
		  }

		DPRINTF (PFX,
				 "calc_pitch == %lu\n"
				 "pScrn->displayWidth == %lu\n"
				 "virtualX == %lu\n"
				 "virtualY == %lu\n"
				 "Memory needed: %lu x %lu x %lu == %luK\n",
				 calc_pitch,
				 pScrn->displayWidth,
				 pScrn->virtualX,
				 pScrn->virtualY,
				 pScrn->virtualY,MAX(pScrn->displayWidth,pScrn->virtualX),pI810->cpp,
				 (pScrn->virtualY * MAX(pScrn->displayWidth,pScrn->virtualX) * pI810->cpp) / 1024);
   }
#endif /* XF86DRI */

   if (pScrn->modes == NULL) return FALSE;

   /* Further validate the mode */
   mode = ((ModeInfoData *) pScrn->modes->Private)->data;
   switch (mode->MemoryModel)
	 {
	  case 0x0:	/* Text mode */
	  case 0x1:	/* CGA graphics */
	  case 0x2:	/* Hercules graphics */
	  case 0x3:	/* Planar */
	  case 0x5:	/* Non-chain 4, 256 color */
	  case 0x7:	/* YUV */
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,"Unsupported Memory Model: %d",mode->MemoryModel);
		return FALSE;
	  case 0x4:	/* Packed pixel */
	  case 0x6:	/*  Direct Color */
		break;
	 }

   xf86PrintModes (pScrn);

   /* Load the required sub modules */
   if (!xf86LoadSubModule (pScrn,"fb"))
	 {
		I830BIOSFreeRec (pScrn);
		return FALSE;
	 }

   xf86LoaderReqSymLists(I810fbSymbols, NULL);

   if (!xf86ReturnOptValBool(I810AvailableOptions(0,0), OPTION_NOACCEL, FALSE)) {
      if (!xf86LoadSubModule(pScrn, "xaa")) {
	 I830BIOSFreeRec(pScrn);
	 return FALSE;
      }
      xf86LoaderReqSymLists(I810xaaSymbols, NULL);
   }

   if (!xf86ReturnOptValBool(I810AvailableOptions(0,0), OPTION_SW_CURSOR, FALSE)) {
      if (!xf86LoadSubModule(pScrn, "ramdac")) {
	 I830BIOSFreeRec(pScrn);
	 return FALSE;
      }
      xf86LoaderReqSymLists(I810ramdacSymbols, NULL);
   }

   /*  We wont be using the VGA access after the probe */
   {
      resRange vgaio[] = { {ResShrIoBlock,0x3B0,0x3BB},
			   {ResShrIoBlock,0x3C0,0x3DF},
			   _END };
      resRange vgamem[] = {{ResShrMemBlock,0xA0000,0xAFFFF},
			   {ResShrMemBlock,0xB8000,0xBFFFF},
			   {ResShrMemBlock,0xB0000,0xB7FFF},
			   _END };

      I810SetMMIOAccess(pI810);
      xf86SetOperatingState(vgaio, pI810->pEnt->index, ResUnusedOpr);
      xf86SetOperatingState(vgamem, pI810->pEnt->index, ResDisableOpr);
   }

   return TRUE;
}

/*
 * Just adapted from the std* functions in vgaHW.c
 */

static void
WriteAttr(int index, int value)
{
   CARD8 tmp;

   tmp = inb(VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

   index |= 0x20;
   outb(VGA_ATTR_INDEX, index);
   outb(VGA_ATTR_DATA_W, value);
}

static int
ReadAttr(int index)
{
   CARD8 tmp;

   tmp = inb(VGA_IOBASE_COLOR + VGA_IN_STAT_1_OFFSET);

   index |= 0x20;
   outb(VGA_ATTR_INDEX, index);
   return (inb(VGA_ATTR_DATA_R));
}

#define WriteMiscOut(value)	outb(VGA_MISC_OUT_W, value)
#define ReadMiscOut()		inb(VGA_MISC_OUT_R)
#define WriteSeq(index, value)	outb(VGA_SEQ_INDEX, index);\
				outb(VGA_SEQ_DATA, value)

static int
ReadSeq(int index)
{
   outb(VGA_SEQ_INDEX, index);

   return (inb(VGA_SEQ_DATA));
}

#define WriteGr(index, value)	outb(VGA_GRAPH_INDEX, index);\
				outb(VGA_GRAPH_DATA, value)
static int
ReadGr(int index)
{
   outb(VGA_GRAPH_INDEX, index);

   return (inb(VGA_GRAPH_DATA));
}

static void
SeqReset(Bool start)
{
   if(start) {
      WriteSeq(0x00, 0x01);		/* Synchronous Reset */
   } else {
      WriteSeq(0x00, 0x03);			/* End Reset */
   }
}

static void
SaveFonts(ScrnInfoPtr pScrn)
{
   I810Ptr pI810;
   VESAPtr pVesa;
   unsigned char miscOut, attr10, gr4, gr5, gr6, seq2, seq4, scrn;

   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;

   if(pVesa->fonts != NULL) return;

   /* If in graphics mode, don't save anything */
   attr10 = ReadAttr(0x10);
   if(attr10 & 0x01) return;

   pVesa->fonts = xalloc(16384);

   /* save the registers that are needed here */
   miscOut = ReadMiscOut();
   gr4 = ReadGr(0x04);
   gr5 = ReadGr(0x05);
   gr6 = ReadGr(0x06);
   seq2 = ReadSeq(0x02);
   seq4 = ReadSeq(0x04);

   /* Force into colour mode */
   WriteMiscOut(miscOut | 0x01);

   scrn = ReadSeq(0x01) | 0x20;
   SeqReset(TRUE);
   WriteSeq(0x01, scrn);
   SeqReset(FALSE);

   WriteAttr(0x10, 0x01);	/* graphics mode */

   /*font1 */
   WriteSeq(0x02, 0x04);	/* write to plane 2 */
   WriteSeq(0x04, 0x06);	/* enable plane graphics */
   WriteGr(0x04, 0x02); 	/* read plane 2 */
   WriteGr(0x05, 0x00); 	/* write mode 0, read mode 0 */
   WriteGr(0x06, 0x05); 	/* set graphics */
   slowbcopy_frombus(pVesa->VGAbase, pVesa->fonts, 8192);

   /* font2 */
   WriteSeq(0x02, 0x08);	/* write to plane 3 */
   WriteSeq(0x04, 0x06);	/* enable plane graphics */
   WriteGr(0x04, 0x03); 	/* read plane 3 */
   WriteGr(0x05, 0x00); 	/* write mode 0, read mode 0 */
   WriteGr(0x06, 0x05); 	/* set graphics */
   slowbcopy_frombus(pVesa->VGAbase, pVesa->fonts + 8192, 8192);

   scrn = ReadSeq(0x01) & ~0x20;
   SeqReset(TRUE);
   WriteSeq(0x01, scrn);
   SeqReset(FALSE);

   /* Restore clobbered registers */
   WriteAttr(0x10, attr10);
   WriteSeq(0x02, seq2);
   WriteSeq(0x04, seq4);
   WriteGr(0x04, gr4);
   WriteGr(0x05, gr5);
   WriteGr(0x06, gr6);
   WriteMiscOut(miscOut);
}

static void
RestoreFonts(ScrnInfoPtr pScrn)
{
   I810Ptr pI810;
   VESAPtr pVesa;
   unsigned char miscOut, attr10, gr1, gr3, gr4, gr5, gr6, gr8, seq2, seq4, scrn;

   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;

   if(pVesa->fonts == NULL) return;

   /* save the registers that are needed here */
   miscOut = ReadMiscOut();
   attr10 = ReadAttr(0x10);
   gr1 = ReadGr(0x01);
   gr3 = ReadGr(0x03);
   gr4 = ReadGr(0x04);
   gr5 = ReadGr(0x05);
   gr6 = ReadGr(0x06);
   gr8 = ReadGr(0x08);
   seq2 = ReadSeq(0x02);
   seq4 = ReadSeq(0x04);

   /* Force into colour mode */
   WriteMiscOut(miscOut | 0x01);

   scrn = ReadSeq(0x01) | 0x20;
   SeqReset(TRUE);
   WriteSeq(0x01, scrn);
   SeqReset(FALSE);

   WriteAttr(0x10, 0x01);	/* graphics mode */

   WriteSeq(0x02, 0x04);   /* write to plane 2 */
   WriteSeq(0x04, 0x06);   /* enable plane graphics */
   WriteGr(0x04, 0x02);    /* read plane 2 */
   WriteGr(0x05, 0x00);    /* write mode 0, read mode 0 */
   WriteGr(0x06, 0x05);    /* set graphics */
   slowbcopy_tobus(pVesa->fonts, pVesa->VGAbase, 8192);

   WriteSeq(0x02, 0x08);   /* write to plane 3 */
   WriteSeq(0x04, 0x06);   /* enable plane graphics */
   WriteGr(0x04, 0x03);    /* read plane 3 */
   WriteGr(0x05, 0x00);    /* write mode 0, read mode 0 */
   WriteGr(0x06, 0x05);    /* set graphics */
   slowbcopy_tobus(pVesa->fonts + 8192, pVesa->VGAbase, 8192);

   scrn = ReadSeq(0x01) & ~0x20;
   SeqReset(TRUE);
   WriteSeq(0x01, scrn);
   SeqReset(FALSE);

   /* restore the registers that were changed */
   WriteMiscOut(miscOut);
   WriteAttr(0x10, attr10);
   WriteGr(0x01, gr1);
   WriteGr(0x03, gr3);
   WriteGr(0x04, gr4);
   WriteGr(0x05, gr5);
   WriteGr(0x06, gr6);
   WriteGr(0x08, gr8);
   WriteSeq(0x02, seq2);
   WriteSeq(0x04, seq4);
}

/* End font code */

void
I830BIOSSaveRegisters(ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR(pScrn);
   I830RegPtr i830Reg = &pI810->i830_SavedReg;
   int i;

   for (i = 0 ; i < 8 ; i++)
      i830Reg->Fence[i] = INREG(FENCE+i*4);

   i830Reg->LprbTail = INREG(LP_RING + RING_TAIL);
   i830Reg->LprbHead = INREG(LP_RING + RING_HEAD);
   i830Reg->LprbStart = INREG(LP_RING + RING_START);
   i830Reg->LprbLen = INREG(LP_RING + RING_LEN);

   if ((i830Reg->LprbTail & TAIL_ADDR) != (i830Reg->LprbHead & HEAD_ADDR) &&
       i830Reg->LprbLen & RING_VALID) {
      I810PrintErrorState( pScrn );
      FatalError( "Active ring not flushed\n");
   }
}

static void I830SetRingRegisters(ScrnInfoPtr pScrn, I830RegPtr i830Reg)
{
   I810Ptr pI810 = I810PTR(pScrn);
   unsigned int temp;

   /* First disable the ring buffer (Need to wait for empty first?, if so
    * should probably do it before entering this section)
    */
   temp = INREG(LP_RING + RING_LEN);
   temp &= ~RING_VALID_MASK;
   OUTREG(LP_RING + RING_LEN, temp );

   /* Set up the low priority ring buffer.
    */
   OUTREG(LP_RING + RING_TAIL, 0 );
   OUTREG(LP_RING + RING_HEAD, 0 );

   pI810->LpRing.head = 0;
   pI810->LpRing.tail = 0;

   temp = INREG(LP_RING + RING_START);
   temp &= ~(START_ADDR);
   temp |= i830Reg->LprbStart;
   OUTREG(LP_RING + RING_START, temp );

   temp = INREG(LP_RING + RING_LEN);
   temp &= ~(I830_RING_NR_PAGES | RING_REPORT_MASK | RING_VALID_MASK);
   temp |= i830Reg->LprbLen;
   OUTREG(LP_RING + RING_LEN, temp );
}

static void I830SetFenceRegisters(ScrnInfoPtr pScrn, I830RegPtr i830Reg)
{
   I810Ptr pI810 = I810PTR(pScrn);
   int i;

   for (i = 0 ; i < 8 ; i++) {
      OUTREG( FENCE+i*4, i830Reg->Fence[i] );
      if (I810_DEBUG & DEBUG_VERBOSE_VGA)
	 ErrorF("Fence Register : %x\n",  i830Reg->Fence[i]);
   }
}

void
I830BIOSSetRegisters(ScrnInfoPtr pScrn, int mode)
{
   I810Ptr pI810 = I810PTR(pScrn);
   I830RegPtr i830Reg;

   if(mode == SET_SAVED_MODE) {
      i830Reg = &pI810->i830_SavedReg;
   } else {
      i830Reg = &pI810->i830_ModeReg;
   }

   I830SetRingRegisters(pScrn, i830Reg);
   I830SetFenceRegisters(pScrn, i830Reg);
}

Bool
I830VESAGetVBEMode(ScrnInfoPtr pScrn, int *mode)
{
   I810Ptr pI810;
   VESAPtr pVesa;

   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x4f03;

   xf86ExecX86int10_wrapper(pVesa->pInt, pScrn);

   if (pVesa->pInt->ax == 0x4f) {
      *mode = pVesa->pInt->bx;

      return TRUE;
   }

   return FALSE;
}

/* At the moment some of the BIOS vesa mode sets doesn't seem to work, so
 * we set those modes using the legacy BIOS calls */
Bool I830VESASetVBEMode (ScrnInfoPtr pScrn,int mode,CRTCInfoBlock *block)
{
   I810Ptr pI810;
   VESAPtr pVesa;

   pI810 = I810PTR (pScrn);
   pVesa = pI810->vesa;

   DPRINTF(PFX,"Setting mode 0x%.8x\n",mode);

   pVesa->pInt->num = 0x10;
   switch (mode)
	 {
	  case 0x0118:
		pVesa->pInt->ax = 0x0054;
		xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
		return (TRUE);
	  case 0x011a:
		pVesa->pInt->ax = 0x0049;
		xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
		return (TRUE);
	  case 0x4003:
	  case 0x03:
		pVesa->pInt->ax = 0x0003;
		xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
		return (TRUE);
	  default:
		pVesa->pInt->ax = 0x4f02;
		pVesa->pInt->bx = mode & ~(1 << 11);
/* This doesn't work. The BIOS is f**cked */
#if 0
		if (block != NULL)
		  {
			 pVesa->pInt->bx |= 1 << 11;
			 memcpy (pVesa->block,block,sizeof (CRTCInfoBlock));
			 pVesa->pInt->es = SEG_ADDR (pVesa->page);
			 pVesa->pInt->di = SEG_OFF (pVesa->page);
		  }
#endif
		xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
		return (pVesa->pInt->ax == 0x004f);
	 }
}

Bool I830VESASaveRestore (ScrnInfoPtr pScrn,int function)
{
   I810Ptr pI810;
   VESAPtr pVesa;

   pI810 = I810PTR (pScrn);
   pVesa = pI810->vesa;

   if(MODE_QUERY < 0 || function > MODE_RESTORE) return (FALSE);

   /* Insure that if the Ring is active we are Synced */

#if 0
   if(pI810->AccelInfoRec != NULL)
	 {
		I810RefreshRing (pScrn);
		I810Sync (pScrn);
	 }
#endif

   /* FIXME: Workaround for 1600x1200 mode set problem */
#if 1
   if (function == MODE_RESTORE) {
	   int thisMode;
	   I830VESAGetVBEMode (pScrn,&thisMode);

	   DPRINTF (PFX,"Current mode: 0x%.8x\n",thisMode);

	   thisMode &= 0xff;

	   if (thisMode == 0x3a || thisMode == 0x4a || thisMode == 0x4b || thisMode == 0x5a || /* 1600x1200 */
		   thisMode == 0x38 || thisMode == 0x48 || thisMode == 0x49 || thisMode == 0x58) {	/* 1280x1024 */
		   DPRINTF (PFX,"Doing 1600x1200 / 1280x1024 mode hack\n");

		   /* first we restore everything the way we're supposed to do it */
		   I830VESASetVBEMode (pScrn,pVesa->stateMode,NULL);
		   RestoreFonts (pScrn);
		   I830BIOSSetRegisters (pScrn,SET_SAVED_MODE);

		   /* now we fake a 800x600x8 mode save and restore */
 		   I830VESASetVBEMode (pScrn,pVesa->stateMode,NULL);
		   I830BIOSSaveRegisters (pScrn);
		   SaveFonts (pScrn);
		   I830VESASetVBEMode (pScrn,0xc032,NULL);
		   I830VESASetVBEMode (pScrn,pVesa->stateMode,NULL);
		   RestoreFonts (pScrn);
		   I830BIOSSetRegisters (pScrn,SET_SAVED_MODE);
		   I830VESASetVBEMode (pScrn,pVesa->stateMode,NULL);

		   return (TRUE);
	   }
   }
#endif

#if 1
   /* This is a workaround that is needed to avoid a bug in VESA Save/Restore,
    * I will remove it as soon as the problem is fixed in the BIOS.
    * Basically it is a problem with 0x4f04 in that it corrupts the
    * GATT table after it has been executed.  We are working around this
    * by just setting the BIOS mode to the save one.  This is not completely
    * correct behavior, especially since it doesn't reset all the registers
    * to their correct state (if something other than the BIOS has set the
    * mode.)
    */

   if(function == MODE_RESTORE) {
	   I830VESASetVBEMode (pScrn,pVesa->stateMode,NULL);
	   RestoreFonts (pScrn);
	   I830BIOSSetRegisters (pScrn,SET_SAVED_MODE);
	   I830VESASetVBEMode (pScrn,pVesa->stateMode,NULL);
	   return (TRUE);
   }
#endif

   /* Save the accel and fence state */
   if(function == MODE_SAVE) I830BIOSSaveRegisters (pScrn);

   /* Query amount of memory to save state */
   if(function == MODE_QUERY || (function == MODE_SAVE && pVesa->state == NULL))
	 {
		int npages;

		/* Make sure we save at least this information in case of failure */
		I830VESAGetVBEMode (pScrn,&pVesa->stateMode);
		SaveFonts (pScrn);

		if (pVesa->major > 1)
		  {
			 pVesa->pInt->num = 0x10;
			 pVesa->pInt->ax = 0x4f04;
			 pVesa->pInt->dx = 0;
			 pVesa->pInt->cx = 0x000f;
			 xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
			 if(pVesa->pInt->ax != 0x4f) return (FALSE);

			 npages = (pVesa->pInt->bx * 64) / 4096 + 1;
			 if ((pVesa->state = xf86Int10AllocPages (pVesa->pInt,npages,&pVesa->statePage)) == NULL)
			   {
				  xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Cannot allocate memory to save SVGA state.\n");
				  return (FALSE);
			   }
			 DPRINTF (PFX,
					  "BIOS returned bx == %d\n"
					  "therefore I need npages == %d (4k pages)\n",
					  pVesa->pInt->bx,npages);
		  }
	 }

#if 0
   /* Save/Restore Super VGA state */
   if(function != MODE_QUERY)
	 {
		int ax_reg = 0;

		if (pVesa->major > 1)
		  {
			 pVesa->pInt->num = 0x10;
			 pVesa->pInt->ax = 0x4f04;
			 pVesa->pInt->dx = function;
			 pVesa->pInt->cx = 0x000f;

			 if (function == MODE_RESTORE)
			   memcpy (pVesa->state,pVesa->pstate,pVesa->stateSize);

			 pVesa->pInt->es = SEG_ADDR (pVesa->statePage);
			 pVesa->pInt->bx = SEG_OFF (pVesa->statePage);
			 xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
			 ax_reg = pVesa->pInt->ax;
		  }

		if(function == MODE_RESTORE)
		  {
			 I830VESASetVBEMode (pScrn,pVesa->stateMode,NULL);
			 RestoreFonts (pScrn);
			 I830BIOSSetRegisters (pScrn,SET_SAVED_MODE);
		  }

		if (pVesa->major > 1)
		  {
			 if (ax_reg != 0x4f) return (FALSE);

			 if (function == MODE_SAVE && pVesa->pstate == NULL)
			   {
				  /* don't rely on the memory not being touched */
				  pVesa->stateSize = pVesa->pInt->bx * 64;
				  pVesa->pstate = xalloc (pVesa->stateSize);
				  memcpy (pVesa->pstate,pVesa->state,pVesa->stateSize);
			   }
		  }
	 }
#endif

   /* Now that the modes setup, we can do our I830 specific fu */
   if(function == MODE_RESTORE) I830BIOSSetRegisters (pScrn,SET_SAVED_MODE);

   return (TRUE);
}

Bool
I830VESASetGetLogicalScanlineLength(ScrnInfoPtr pScrn, int command, int width,
				    int *pixels, int *bytes, int *max)
{
   I810Ptr pI810;
   VESAPtr pVesa;

   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;

   if(command < SCANWID_SET || command > SCANWID_GET_MAX)
     return FALSE;

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x4f06;
   pVesa->pInt->bx = command;
   if(command == SCANWID_SET || command == SCANWID_SET_BYTES)
     pVesa->pInt->cx = width;
   xf86ExecX86int10_wrapper(pVesa->pInt, pScrn);

   if(pVesa->pInt->ax != 0x4f)
     return FALSE;

   if(command == SCANWID_GET || command == SCANWID_GET_MAX) {
      if(pixels) *pixels = pVesa->pInt->cx;
      if(bytes) *bytes = pVesa->pInt->bx;
      if(max) *max = pVesa->pInt->dx;
   }

   return TRUE;
}

int
I830VESASetGetDACPaletteFormat(ScrnInfoPtr pScrn, int bits)
{
   I810Ptr pI810;
   VESAPtr pVesa;

   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x4f08;
   if(!bits) pVesa->pInt->bx = 0x01;
   else pVesa->pInt->bx = (bits & 0x00ff) << 8;
   xf86ExecX86int10_wrapper(pVesa->pInt, pScrn);

   if(pVesa->pInt->ax != 0x4f) return 0;

   return (bits != 0 ? bits : (pVesa->pInt->bx >> 8) & 0x00ff);
}

static Bool I830VESASetMode (ScrnInfoPtr pScrn,DisplayModePtr pMode)
{
   I810Ptr pI810;
   VESAPtr pVesa;
   ModeInfoData *data;
   int mode;

   pI810 = I810PTR (pScrn);
   pVesa = pI810->vesa;

   data = (ModeInfoData *) pMode->Private;

   /* Always Enable Linear Addressing */
   mode = data->mode | (1 << 15) | (1 << 14);

#ifdef XF86DRI
   if(pI810->directRenderingEnabled)
	 {
		DRILock (screenInfo.screens[pScrn->scrnIndex],0);
		pI810->LockHeld = 1;
	 }
#endif

   if (I830VESASetVBEMode (pScrn,mode,data->block) == FALSE)
	 {
		if ((data->block || (data->mode & (1 << 11))) &&
			I830VESASetVBEMode (pScrn,(mode & ~(1 << 11)),NULL) == TRUE)
		  {
			 xf86DrvMsg (pScrn->scrnIndex,X_WARNING,"Set VBE Mode rejected this modeline. Trying current mode instead!\n");
			 DPRINTF (PFX,"OOPS!\n");
			 xfree (data->block);
			 data->block = NULL;
			 data->mode &= ~(1 << 11);
		  }
		else
		  {
			 xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Set VBE Mode failed!\n");
			 return (FALSE);
		  }
	 }

   /* test if CRT display is present */
   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x5f55;
   xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);

   /* is this a CRT? */
   if (pVesa->pInt->ax == 0x005f && pVesa->pInt->bx != 0x0002)
	 {
		DPRINTF (PFX,
				 "data->data = {\n"
				 "   XResolution: %d\n"
				 "   YResolution: %d\n"
				 "}\n",
				 data->data->XResolution,data->data->YResolution);
		if (data->block != NULL)
		  DPRINTF (PFX,
				   "data->block = {\n"
				   "       HorizontalTotal: %d\n"
				   "   HorizontalSyncStart: %d\n"
				   "     HorizontalSyncEnd: %d\n"
				   "         VerticalTotal: %d\n"
				   "     VerticalSyncStart: %d\n"
				   "       VerticalSyncEnd: %d\n"
				   "                 Flags: %d\n"
				   "            PixelClock: %d\n"
				   "           RefreshRate: %d\n"
				   "}\n",
				   data->block->HorizontalTotal,data->block->HorizontalSyncStart,data->block->HorizontalSyncEnd,
				   data->block->VerticalTotal,data->block->VerticalSyncStart,data->block->VerticalSyncEnd,
				   data->block->Flags,data->block->PixelClock,data->block->RefreshRate / 100);

		/* make double sure it's not an LCD */
		pVesa->pInt->num = 0x10;
		pVesa->pInt->ax = 0x5f64;
		pVesa->pInt->bx = 0x0100;
		xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);
		if (data->data != NULL && data->block != NULL && pVesa->pInt->ax == 0x005f && !(pVesa->pInt->cx & 0x0008))
		  {
			 int i;
			 static const int VesaRefresh[] = { 43, 56, 60, 70, 72, 75, 85, 100, 120, -1 };

			 for (i = 0; VesaRefresh[i] != -1 && VesaRefresh[i] != data->block->RefreshRate / 100; i++) ;

			 if (VesaRefresh[i] == data->block->RefreshRate / 100)
			   {
				  DPRINTF (PFX,
						   "Setting refresh rate to %dHz for mode %d\n",
						   VesaRefresh[i],
						   mode & 0xff);

				  pVesa->pInt->num = 0x10;
				  pVesa->pInt->ax = 0x5f05;
				  pVesa->pInt->bx = mode & 0xff;
				  pVesa->pInt->cx = 1 << i;
				  xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);

				  if (pVesa->pInt->ax != 0x5f)
					xf86DrvMsg (pScrn->scrnIndex,X_WARNING,"Failed to set refresh rate to %dHz!\n",VesaRefresh[i]);
			   }
		  }
	 }

   if (data->data->XResolution != pScrn->displayWidth)
	 I830VESASetLogicalScanline (pScrn,pScrn->displayWidth);

   if (pScrn->bitsPerPixel >= 8 && pVesa->vbeInfo->Capabilities[0] & 0x01)
	 I830VESASetGetDACPaletteFormat (pScrn,8);

   I830BIOSSetRegisters (pScrn,SET_CURRENT_MODE);
   I810SetRingRegs (pScrn);

#ifdef XF86DRI
   if (pI810->directRenderingEnabled)
	 {
		DRIUnlock (screenInfo.screens[pScrn->scrnIndex]);
		pI810->LockHeld = 0;
	 }
#endif

   pScrn->vtSema = TRUE;
   return (TRUE);
}

CARD32 *
I830VESASetGetPaletteData(ScrnInfoPtr pScrn, Bool set, int first, int num,
			  CARD32 *data, Bool secondary, Bool wait_retrace)
{
   I810Ptr pI810;
   VESAPtr pVesa;

   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x4f09;

   if(!secondary) pVesa->pInt->bx = set && wait_retrace ? 0x80 : set ? 0 : 1;
   else pVesa->pInt->bx = set ? 2 : 3;

   pVesa->pInt->cx = num;
   pVesa->pInt->dx = first;
   pVesa->pInt->es = SEG_ADDR(pVesa->page);
   pVesa->pInt->di = SEG_OFF(pVesa->page);

   if(set) memcpy(pVesa->block, data, num * sizeof(CARD32));

   xf86ExecX86int10_wrapper(pVesa->pInt, pScrn);

   if(pVesa->pInt->ax != 0x4f) return NULL;

   if(set) return data;

   data = xalloc(num * sizeof(CARD32));
   memcpy(data, pVesa->block, num * sizeof(CARD32));

   return data;
}

Bool
I830BIOSInitializeFirstMode(int scrnIndex)
{
   ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
   I810Ptr pI810 = I810PTR(pScrn);
   VESAPtr pVesa = pI810->vesa;
   I830RegPtr i830Reg = &pI810->i830_ModeReg;
   int i;

   for (i = 0 ; i < 8 ; i++)
     i830Reg->Fence[i] = 0;

   /* Setup the ring buffer */
   i830Reg->LprbTail = 0;
   i830Reg->LprbHead = 0;
   i830Reg->LprbStart = pI810->LpRing.mem.Start;

   if (i830Reg->LprbStart) 
     i830Reg->LprbLen = ((pI810->LpRing.mem.Size-4096) |
			 RING_NO_REPORT | RING_VALID);
   else
     i830Reg->LprbLen = RING_INVALID;

   /* save current video state */ 
   if(!I830VESASaveRestore(pScrn, MODE_SAVE)) return FALSE;
   pVesa->savedPal = I830VESASetGetPaletteData(pScrn, FALSE, 0, 256,
					      NULL, FALSE, FALSE);

   /* set first video mode */
   if(!I830VESASetMode(pScrn, pScrn->currentMode))
     return FALSE;

   I830BIOSAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

   return TRUE;
}

#ifdef I830DEBUG
static void dump_DSPACNTR (ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR (pScrn);
   unsigned int tmp;

   /* Display A Control */
   tmp = INREG (0x70180);
   fprintf (stderr,"Display A Plane Control Register (0x%.8x)\n",tmp);

   if (tmp & BIT(31))
	 fprintf (stderr,"   Display Plane A (Primary) Enable\n");
   else
	 fprintf (stderr,"   Display Plane A (Primary) Disabled\n");

   if (tmp & BIT(30))
	 fprintf (stderr,"   Display A pixel data is gamma corrected\n");
   else
	 fprintf (stderr,"   Display A pixel data bypasses gamma correction logic (default)\n");

   switch ((tmp & 0x3c000000) >> 26)	/* bit 29:26 */
	 {
	  case 0x00:
	  case 0x01:
	  case 0x03:
		fprintf (stderr,"   Reserved\n");
		break;
	  case 0x02:
		fprintf (stderr,"   8-bpp Indexed\n");
		break;
	  case 0x04:
		fprintf (stderr,"   15-bit (5-5-5) pixel format (Targa compatible)\n");
		break;
	  case 0x05:
		fprintf (stderr,"   16-bit (5-6-5) pixel format (XGA compatible)\n");
		break;
	  case 0x06:
		fprintf (stderr,"   32-bit format (X:8:8:8)\n");
		break;
	  case 0x07:
		fprintf (stderr,"   32-bit format (8:8:8:8)\n");
		break;
	  default:
		fprintf (stderr,"   Unknown - Invalid register value maybe?\n");
	 }

   if (tmp & BIT(25))
	 fprintf (stderr,"   Stereo Enable\n");
   else
	 fprintf (stderr,"   Stereo Disable\n");

   if (tmp & BIT(24))
	 fprintf (stderr,"   Display A, Pipe B Select\n");
   else
	 fprintf (stderr,"   Display A, Pipe A Select\n");

   if (tmp & BIT(22))
	 fprintf (stderr,"   Source key is enabled\n");
   else
	 fprintf (stderr,"   Source key is disabled\n");

   switch ((tmp & 0x00300000) >> 20)	/* bit 21:20 */
	 {
	  case 0x00:
		fprintf (stderr,"   No line duplication\n");
		break;
	  case 0x01:
		fprintf (stderr,"   Line/pixel Doubling\n");
		break;
	  case 0x02:
	  case 0x03:
		fprintf (stderr,"   Reserved\n");
		break;
	 }

   if (tmp & BIT(18))
	 fprintf (stderr,"   Stereo output is high during second image\n");
   else
	 fprintf (stderr,"   Stereo output is high during first image\n");
}

static void dump_DSPBCNTR (ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR (pScrn);
   unsigned int tmp;

   /* Display B/Sprite Control */
   tmp = INREG (0x71180);
   fprintf (stderr,"Display B/Sprite Plane Control Register (0x%.8x)\n",tmp);

   if (tmp & BIT(31))
	 fprintf (stderr,"   Display B/Sprite Enable\n");
   else
	 fprintf (stderr,"   Display B/Sprite Disable\n");

   if (tmp & BIT(30))
	 fprintf (stderr,"   Display B pixel data is gamma corrected\n");
   else
	 fprintf (stderr,"   Display B pixel data bypasses gamma correction logic (default)\n");

   switch ((tmp & 0x3c000000) >> 26)	/* bit 29:26 */
	 {
	  case 0x00:
	  case 0x01:
	  case 0x03:
		fprintf (stderr,"   Reserved\n");
		break;
	  case 0x02:
		fprintf (stderr,"   8-bpp Indexed\n");
		break;
	  case 0x04:
		fprintf (stderr,"   15-bit (5-5-5) pixel format (Targa compatible)\n");
		break;
	  case 0x05:
		fprintf (stderr,"   16-bit (5-6-5) pixel format (XGA compatible)\n");
		break;
	  case 0x06:
		fprintf (stderr,"   32-bit format (X:8:8:8)\n");
		break;
	  case 0x07:
		fprintf (stderr,"   32-bit format (8:8:8:8)\n");
		break;
	  default:
		fprintf (stderr,"   Unknown - Invalid register value maybe?\n");
	 }

   if (tmp & BIT(25))
	 fprintf (stderr,"   Stereo is enabled and both start addresses are used in a two frame sequence\n");
   else
	 fprintf (stderr,"   Stereo disable and only a single start address is used\n");

   if (tmp & BIT(24))
	 fprintf (stderr,"   Display B/Sprite, Pipe B Select\n");
   else
	 fprintf (stderr,"   Display B/Sprite, Pipe A Select\n");

   if (tmp & BIT(22))
	 fprintf (stderr,"   Sprite source key is enabled\n");
   else
	 fprintf (stderr,"   Sprite source key is disabled (default)\n");

   switch ((tmp & 0x00300000) >> 20)	/* bit 21:20 */
	 {
	  case 0x00:
		fprintf (stderr,"   No line duplication\n");
		break;
	  case 0x01:
		fprintf (stderr,"   Line/pixel Doubling\n");
		break;
	  case 0x02:
	  case 0x03:
		fprintf (stderr,"   Reserved\n");
		break;
	 }

   if (tmp & BIT(18))
	 fprintf (stderr,"   Stereo output is high during second image\n");
   else
	 fprintf (stderr,"   Stereo output is high during first image\n");

   if (tmp & BIT(15))
	 fprintf (stderr,"   Alpha transfer mode enabled\n");
   else
	 fprintf (stderr,"   Alpha transfer mode disabled\n");

   if (tmp & BIT(0))
	 fprintf (stderr,"   Sprite is above overlay\n");
   else
	 fprintf (stderr,"   Sprite is above display A (default)\n");
}

static void dump_registers (ScrnInfoPtr pScrn)
{
   I810Ptr pI810 = I810PTR (pScrn);
   unsigned int i;
   fprintf (stderr,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");

   dump_DSPACNTR (pScrn);
   dump_DSPBCNTR (pScrn);

   fprintf (stderr,"0x71400 == 0x%.8x\n",INREG(0x71400));
   fprintf (stderr,"0x70008 == 0x%.8x\n",INREG(0x70008));
   for (i = 0x71410; i <= 0x71428; i += 4)
	 fprintf (stderr,"0x%x == 0x%.8x\n",i,INREG(i));

   fprintf (stderr,"%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n");
}
#endif

Bool
I830BIOSScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
   ScrnInfoPtr pScrn;
   vgaHWPtr hwp;
   I810Ptr pI810;
   VESAPtr pVesa;
   VisualPtr visual;

   pScrn = xf86Screens[pScreen->myNum];
   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;
   hwp = VGAHWPTR(pScrn);

   miClearVisualTypes();
   if(!xf86SetDefaultVisual(pScrn, -1)) return FALSE;
   if(pScrn->bitsPerPixel > 8) {
      if(!miSetVisualTypes(pScrn->depth, TrueColorMask,
			   pScrn->rgbBits, TrueColor)) return FALSE;
   } else {
      if(!miSetVisualTypes(pScrn->depth,
			   miGetDefaultVisualMask(pScrn->depth),
			   pScrn->rgbBits, pScrn->defaultVisual)) return FALSE;
   }
   if (!miSetPixmapDepths()) return FALSE;

#ifdef XF86DRI
   /*
    * Setup DRI after visuals have been established, but before cfbScreenInit
    * is called.   cfbScreenInit will eventually call into the drivers
    * InitGLXVisuals call back.
    */

   if (!xf86ReturnOptValBool(I810AvailableOptions(0,0), OPTION_NOACCEL, 
			     FALSE) &&
       xf86ReturnOptValBool(I810AvailableOptions(0,0), OPTION_DRI, TRUE)) {
      pI810->directRenderingEnabled = I830DRIScreenInit(pScreen); 
   } else {
      pI810->directRenderingEnabled = FALSE;
   }
#else
   pI810->directRenderingEnabled = FALSE;
   if(!I810AllocateGARTMemory( pScrn )) 
      return FALSE;
   if(!I810AllocateFront(pScrn))
      return FALSE;
#endif

	DPRINTF(PFX,"assert( if(!I810MapMem(pScrn)) )\n");
   if(!I810MapMem(pScrn)) return FALSE;

   pScrn->memPhysBase = (unsigned long)pI810->FbBase;
   pScrn->fbOffset = 0;

   vgaHWSetMmioFuncs(hwp, pI810->MMIOBase, 0);
   vgaHWGetIOBase(hwp);
	DPRINTF(PFX,"assert( if(!vgaHWMapMem(pScrn)) )\n");
   if(!vgaHWMapMem(pScrn)) return FALSE;

   /* Handle Setup of the mode here */
	DPRINTF(PFX,"assert( if(!(I830BIOSInitializeFirstMode(scrnIndex))) )\n");
   if(!(I830BIOSInitializeFirstMode(scrnIndex))) return FALSE;

	DPRINTF(PFX,"assert( if(!fbScreenInit(pScreen, ...) )\n");
   if(!fbScreenInit(pScreen, pI810->FbBase + pScrn->fbOffset,
		        pScrn->virtualX, pScrn->virtualY,
                        pScrn->xDpi, pScrn->yDpi,
                        pScrn->displayWidth, pScrn->bitsPerPixel))
       return FALSE;

   if(pScrn->bitsPerPixel > 8) {
      /* Fixup RGB ordering */
      visual = pScreen->visuals + pScreen->numVisuals;
      while(--visual >= pScreen->visuals) {
	 if((visual->class | DynamicClass) == DirectColor) {
	    visual->offsetRed = pScrn->offset.red;
	    visual->offsetGreen = pScrn->offset.green;
	    visual->offsetBlue = pScrn->offset.blue;
	    visual->redMask = pScrn->mask.red;
	    visual->greenMask = pScrn->mask.green;
	    visual->blueMask = pScrn->mask.blue;
	 }
      }
   }

   xf86SetBlackWhitePixels(pScreen);

#ifdef XF86DRI
   if(pI810->LpRing.mem.Start == 0 && pI810->directRenderingEnabled) {
      pI810->directRenderingEnabled = 0;
      I830DRICloseScreen(pScreen);
   }

   if(!pI810->directRenderingEnabled) {
      pI810->DoneFrontAlloc = FALSE;
	   DPRINTF(PFX,"assert( if(!I810AllocateGARTMemory( pScrn )) )\n");
      if(!I810AllocateGARTMemory( pScrn ))
         return FALSE;
	   DPRINTF(PFX,"assert( if(!I810AllocateFront(pScrn)) )\n");
      if(!I810AllocateFront(pScrn))
	 return FALSE;
   }
#endif

   I810DGAInit(pScreen);

	DPRINTF(PFX,"assert( if(!xf86InitFBManager(pScreen, &(pI810->FbMemBox))) )\n");
   if(!xf86InitFBManager(pScreen, &(pI810->FbMemBox))) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                 "Failed to init memory manager\n");
      return FALSE;
   }

   if(!xf86ReturnOptValBool(I830BIOSOptions, OPTION_NOACCEL, FALSE)) {
      if(pI810->LpRing.mem.Size != 0) {
         I810SetRingRegs( pScrn );

         if(!I830AccelInit(pScreen)) {
            xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "Hardware acceleration initialization failed\n");
         }
      }
   }

   miInitializeBackingStore(pScreen);
   xf86SetBackingStore(pScreen);
   xf86SetSilkenMouse(pScreen);
   miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

   if(!xf86ReturnOptValBool(I830BIOSOptions, OPTION_SW_CURSOR, FALSE))
	 {
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"Initializing HW Cursor\n");
		if(!I810CursorInit(pScreen))
		  xf86DrvMsg (pScrn->scrnIndex,X_ERROR,"Hardware cursor initialization failed\n");
	 }
   else xf86DrvMsg (pScrn->scrnIndex,X_INFO,"Initializing SW Cursor!\n");

	DPRINTF(PFX,"assert( if(!miCreateDefColormap(pScreen)) )\n");
   if(!miCreateDefColormap(pScreen)) return FALSE;

	DPRINTF(PFX,"assert( if(!vgaHWHandleColormaps(pScreen)) )\n");
   if(!vgaHWHandleColormaps(pScreen)) return FALSE;

#ifdef DPMSExtension
   xf86DPMSInit(pScreen, I830DisplayPowerManagementSet, 0);
#endif

   /* Init video here - Not supported as of yet */

#ifdef XF86DRI
   if(pI810->directRenderingEnabled) {
      pI810->directRenderingEnabled = I830DRIFinishScreenInit(pScreen);
      if(!pI810->directRenderingEnabled) {
	 pI810->agpAcquired2d = FALSE;
      }
   }
#endif

   if(pI810->directRenderingEnabled) {
      xf86DrvMsg(pScrn->scrnIndex, X_INFO, "direct rendering: Enabled\n");
      /* Setup 3D engine */
#ifdef XF86DRI
      I830EmitInvarientState(pScrn);
#if 0
      I830EmitInvarientState2(pScrn);
#endif
#endif
   } else {
      if(pI810->agpAcquired2d == TRUE) {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO, "direct rendering: Disabled\n");
      } else {
	 xf86DrvMsg(pScrn->scrnIndex, X_INFO, "direct rendering: Failed\n");
	 return FALSE;
      }
   }
   
   pScreen->SaveScreen = I830BIOSSaveScreen;
   pI810->CloseScreen = pScreen->CloseScreen;
   pScreen->CloseScreen = I830BIOSCloseScreen;

   if (serverGeneration == 1)
      xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
/*
#ifdef I830DEBUG
   dump_registers (pScrn);
#endif
*/
   return TRUE;
}

Bool
I830VESASetDisplayStart(ScrnInfoPtr pScrn, int x, int y, Bool wait_retrace)
{
   I810Ptr pI810;
   VESAPtr pVesa;

   pI810 = I810PTR(pScrn);
   pVesa = pI810->vesa;

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x4f07;
   pVesa->pInt->bx = wait_retrace ? 0x80 : 0x00;
   pVesa->pInt->cx = x;
   pVesa->pInt->dx = y;
   xf86ExecX86int10_wrapper(pVesa->pInt, pScrn);

   if(pVesa->pInt->ax != 0x4f) return FALSE;
   return TRUE;
}

void
I830BIOSAdjustFrame(int scrnIndex, int x, int y, int flags)
{
   ScrnInfoPtr pScrn;

   pScrn = xf86Screens[scrnIndex];
   I830VESASetDisplayStart(pScrn, x, y, TRUE);
}

void
I830BIOSFreeScreen(int scrnIndex, int flags)
{
   I830BIOSFreeRec(xf86Screens[scrnIndex]);
   if (xf86LoaderCheckSymbol("vgaHWFreeHWRec"))
       vgaHWFreeHWRec(xf86Screens[scrnIndex]);
}

void
I830BIOSLeaveVT (int scrnIndex,int flags) {
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	vgaHWPtr hwp = VGAHWPTR (pScrn);
#if defined(XF86DRI) || 0
	I810Ptr pI810 = I810PTR (pScrn);
#endif

	DPRINTF (PFX,"Leave VT\n");

#ifdef XF86DRI
	if (pI810->directRenderingEnabled) {
		DPRINTF (PFX,"calling dri lock\n");
		DRILock (screenInfo.screens[scrnIndex],0);
		pI810->LockHeld = 1;
	}
#endif

#if 0
	if (pI810->AccelInfoRec != NULL) {
		DPRINTF (PFX,"syncing ring buffer\n");
		I810Sync (pScrn);
		DO_RING_IDLE ();
	}
#endif

	I830VESASaveRestore (pScrn,MODE_RESTORE);

	if (!I810UnbindGARTMemory (pScrn)) return;

	vgaHWLock(hwp);
}

Bool
I830BIOSEnterVT (int scrnIndex,int flags) {
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	I810Ptr pI810 = I810PTR (pScrn);

	DPRINTF (PFX,"Enter VT\n");

	if (!I810BindGARTMemory (pScrn)) return FALSE;

#ifdef XF86DRI
	if (pI810->directRenderingEnabled) {
		DPRINTF (PFX,"calling dri unlock\n");
		DRIUnlock (screenInfo.screens[scrnIndex]);
		pI810->LockHeld = 0;
	}
#endif

	/* FIXME: Is this really necessary? */
#if 0
	VESAPtr pVesa = pI810->vesa;
	/* Switch to configured display device */
	pVesa->pInt->num = 0x10;
	pVesa->pInt->ax = 0x5f64;
	pVesa->pInt->bx = 0x0001;
	pVesa->pInt->cx = (CARD16) pI810->configured_device;
	xf86ExecX86int10_wrapper (pVesa->pInt,pScrn);

	if (pVesa->pInt->ax != 0x005f) {
		xf86DrvMsg (pScrn->scrnIndex,X_INFO,"Failed to switch to configured display device\n");
		return FALSE;
	}
#endif

	if (!I830VESASetMode (pScrn,pScrn->currentMode))
		return FALSE;

	I830BIOSAdjustFrame (scrnIndex,pScrn->frameX0,pScrn->frameY0,0);

#if 1
	if (pI810->AccelInfoRec != NULL) {
		DPRINTF (PFX,"syncing ring buffer\n");
		I810Sync (pScrn);
		DO_RING_IDLE ();
	}
#endif

	return TRUE;
}

Bool
I830BIOSSwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
   DPRINTF (PFX,"mode == %s\n",mode);
   return I830VESASetMode(xf86Screens[scrnIndex], mode);
}

Bool
I830BIOSSaveScreen(ScreenPtr pScreen, Bool unblack)
{
   return vgaHWSaveScreen(pScreen, unblack);
}

static void
I830DisplayPowerManagementSet(ScrnInfoPtr pScrn, int PowerManagementMode,
			     int flags)
{
   I810Ptr pI810 = I810PTR(pScrn);
   VESAPtr pVesa = pI810->vesa;

   pVesa->pInt->num = 0x10;
   pVesa->pInt->ax = 0x4f10;

   switch (PowerManagementMode) {
   case DPMSModeOn: pVesa->pInt->bx = 0x0001; break;
   case DPMSModeStandby: pVesa->pInt->bx = 0x0101; break;
   case DPMSModeSuspend: pVesa->pInt->bx = 0x0201; break;
   case DPMSModeOff: pVesa->pInt->bx = 0x0401; break;
   default: pVesa->pInt->bx = 0x0001; break;
   }

   xf86ExecX86int10_wrapper(pVesa->pInt, pScrn);   
}

Bool
I830BIOSCloseScreen(int scrnIndex, ScreenPtr pScreen)
{
   ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
   vgaHWPtr hwp = VGAHWPTR(pScrn);
   I810Ptr pI810 = I810PTR(pScrn);
   VESAPtr pVesa = pI810->vesa;
   XAAInfoRecPtr infoPtr = pI810->AccelInfoRec;

#ifdef XF86DRI
   if (pI810->directRenderingEnabled) {
       I830DRICloseScreen(pScreen);
       pI810->directRenderingEnabled = FALSE;
   }
#endif

   if (pScrn->vtSema == TRUE) {
      if(pI810->AccelInfoRec) {
	 I810Sync(pScrn);
	 DO_RING_IDLE();
      }
      I830VESASaveRestore(pScrn, MODE_RESTORE);
      I830VESASetGetPaletteData(pScrn, TRUE, 0, 256,
				pVesa->savedPal, FALSE, TRUE);
      I810UnbindGARTMemory(pScrn);
      vgaHWLock(hwp);
   }

   I810UnmapMem(pScrn);
   vgaHWUnmapMem(pScrn);

   if(pI810->ScanlineColorExpandBuffers) {
      xfree(pI810->ScanlineColorExpandBuffers);
      pI810->ScanlineColorExpandBuffers = 0;
   }

   if(infoPtr) {
      if(infoPtr->ScanlineColorExpandBuffers)
	xfree(infoPtr->ScanlineColorExpandBuffers);
      XAADestroyInfoRec(infoPtr);
      pI810->AccelInfoRec=0;
   }

   if(pI810->CursorInfoRec) {
      xf86DestroyCursorInfoRec(pI810->CursorInfoRec);
      pI810->CursorInfoRec=0;
   }

   /* Free all allocated video ram.
    */
   pI810->SysMem = pI810->SavedSysMem;
   pI810->DcacheMem = pI810->SavedDcacheMem;
   pI810->DoneFrontAlloc = FALSE;

   xf86GARTCloseScreen(scrnIndex);

   pScrn->vtSema=FALSE;
   pScreen->CloseScreen = pI810->CloseScreen;
   return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}

