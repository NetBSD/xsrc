/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmware[] =

    "Id: vmware.c,v 1.11 2001/02/23 02:10:39 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmware.c,v 1.8 2001/10/28 03:33:53 tsi Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Resources.h"

#include "compiler.h"	/* inb/outb */

#include "xf86PciInfo.h"	/* pci vendor id */
#include "xf86Pci.h"		/* pci */

#include "mipointer.h"		/* sw cursor */
#include "mibstore.h"		/* backing store */
#include "micmap.h"		/* mi color map */
#include "vgaHW.h"		/* VGA hardware */
#include "fb.h"

#include "xf86cmap.h"		/* xf86HandleColormaps */

#include "vmware.h"
#include "guest_os.h"
#include "vm_device_version.h"

/*
 * Sanity check that xf86PciInfo.h has the correct values (which come from
 * the VMware source tree in vm_device_version.h.
 */
#if PCI_CHIP_VMWARE0405 != PCI_DEVICE_ID_VMWARE_SVGA2
#error "PCI_CHIP_VMWARE0405 is wrong, update it from vm_device_version.h"
#endif
#if PCI_CHIP_VMWARE0710 != PCI_DEVICE_ID_VMWARE_SVGA
#error "PCI_CHIP_VMWARE0710 is wrong, update it from vm_device_version.h"
#endif
#if PCI_VENDOR_VMWARE != PCI_VENDOR_ID_VMWARE
#error "PCI_VENDOR_VMWARE is wrong, update it from vm_device_version.h"
#endif

/*
 * This is the only way I know to turn a #define of an integer constant into
 * a constant string.
 */
#define VMW_INNERSTRINGIFY(s)	#s
#define VMW_STRING(str)		VMW_INNERSTRINGIFY(str)

#define VMWARE_NAME	"VMWARE"
#define VMWARE_DRIVER_NAME	"vmware"
#define VMWARE_MAJOR_VERSION	10
#define VMWARE_MINOR_VERSION	7
#define VMWARE_PATCHLEVEL	0
#define VERSION		(VMWARE_MAJOR_VERSION * 65536 + VMWARE_MINOR_VERSION * 256 + VMWARE_PATCHLEVEL)

static const char VMWAREBuildStr[] =
                     "VMware Guest X Server " VMW_STRING(VMWARE_MAJOR_VERSION)
		     "." VMW_STRING(VMWARE_MINOR_VERSION)
		     "." VMW_STRING(VMWARE_PATCHLEVEL) " - build=$Name:  $\n";

static SymTabRec VMWAREChipsets[] = {
	{ PCI_CHIP_VMWARE0405, "vmware0405" },
	{ PCI_CHIP_VMWARE0710, "vmware0710" },
	{ -1,                  NULL }
};

static resRange vmwareLegacyRes[] = {
   { ResExcIoBlock, SVGA_LEGACY_BASE_PORT, SVGA_LEGACY_BASE_PORT + SVGA_NUM_PORTS*sizeof(uint32)},
   _VGA_EXCLUSIVE, _END
};

/*
 * Currently, even the PCI obedient 0405 chip still only obeys IOSE and
 * MEMSE for the SVGA resources.  Thus, RES_EXCLUSIVE_VGA is required.
 *
 * The 0710 chip also uses hardcoded IO ports that aren't disablable.
 */

static PciChipsets VMWAREPciChipsets[] = {
	{ PCI_CHIP_VMWARE0405, PCI_CHIP_VMWARE0405, RES_EXCLUSIVE_VGA },
	{ PCI_CHIP_VMWARE0710, PCI_CHIP_VMWARE0710, vmwareLegacyRes },
	{ -1,		       -1,		    RES_UNDEFINED }
};

static const char *vgahwSymbols[] = {
	"vgaHWGetHWRec",
	"vgaHWGetIOBase",
	"vgaHWGetIndex",
	"vgaHWInit",
	"vgaHWProtect",
	"vgaHWRestore",
	"vgaHWSave",
	"vgaHWSaveScreen",
	"vgaHWUnlock",
	NULL
};

static const char *fbSymbols[] = {
	"fbPictureInit",
	"fbScreenInit",
	NULL
};

#ifdef XFree86LOADER
static XF86ModuleVersionInfo vmwareVersRec = {
	"vmware",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XF86_VERSION_CURRENT,
	VMWARE_MAJOR_VERSION, VMWARE_MINOR_VERSION, VMWARE_PATCHLEVEL,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	MOD_CLASS_VIDEODRV,
	{ 0, 0, 0, 0}
};
#endif	/* XFree86LOADER */

typedef enum {
	OPTION_HW_CURSOR,
	OPTION_NOACCEL
} VMWAREOpts;

static const OptionInfoRec VMWAREOptions[] = {
	{ OPTION_HW_CURSOR,	"HWcursor",	OPTV_BOOLEAN,	{0},	FALSE },
	{ OPTION_NOACCEL,	"NoAccel",	OPTV_BOOLEAN,	{0},	FALSE },
	{ -1,			NULL,		OPTV_NONE,	{0},	FALSE }
};

static Bool
VMWAREGetRec(ScrnInfoPtr pScrn)
{
	if (pScrn->driverPrivate != NULL) {
		return TRUE;
	}
	pScrn->driverPrivate = xnfcalloc(sizeof(VMWARERec), 1);
	/* FIXME: Initialize driverPrivate... */
	return TRUE;
}

static void
VMWAREFreeRec(ScrnInfoPtr pScrn)
{
	if (pScrn->driverPrivate) {
		xfree(pScrn->driverPrivate);
		pScrn->driverPrivate = NULL;
	}
}

static void ScreenToPrivate(ScreenPtr, ScrnInfoPtr);
static void ScreenFromPrivate(ScreenPtr, ScrnInfoPtr);

/* VMware specific functions */

static CARD32
vmwareReadReg(VMWAREPtr pVMWARE, int index)
{
    outl(pVMWARE->indexReg, index);
    return inl(pVMWARE->valueReg);
}

void
vmwareWriteReg(VMWAREPtr pVMWARE, int index, CARD32 value)
{
    outl(pVMWARE->indexReg, index);
    outl(pVMWARE->valueReg, value);
}

void
vmwareWriteWordToFIFO(VMWAREPtr pVMWARE, CARD32 value)
{
    CARD32* vmwareFIFO = pVMWARE->vmwareFIFO;

    /* Need to sync? */
    if ((vmwareFIFO[SVGA_FIFO_NEXT_CMD] + sizeof(CARD32) == vmwareFIFO[SVGA_FIFO_STOP])
     || (vmwareFIFO[SVGA_FIFO_NEXT_CMD] == vmwareFIFO[SVGA_FIFO_MAX] - sizeof(CARD32) &&
	 vmwareFIFO[SVGA_FIFO_STOP] == vmwareFIFO[SVGA_FIFO_MIN])) {
	vmwareWriteReg(pVMWARE, SVGA_REG_SYNC, 1);
	while (vmwareReadReg(pVMWARE, SVGA_REG_BUSY)) ;
    }
    vmwareFIFO[vmwareFIFO[SVGA_FIFO_NEXT_CMD] / sizeof(CARD32)] = value;
    vmwareFIFO[SVGA_FIFO_NEXT_CMD] += sizeof(CARD32);
    if (vmwareFIFO[SVGA_FIFO_NEXT_CMD] == vmwareFIFO[SVGA_FIFO_MAX]) {
	vmwareFIFO[SVGA_FIFO_NEXT_CMD] = vmwareFIFO[SVGA_FIFO_MIN];
    }
}

void
vmwareWaitForFB(VMWAREPtr pVMWARE)
{
    if (pVMWARE->vmwareFIFOMarkSet) {
	vmwareWriteReg(pVMWARE, SVGA_REG_SYNC, 1);
	while (vmwareReadReg(pVMWARE, SVGA_REG_BUSY)) ;
	pVMWARE->vmwareFIFOMarkSet = FALSE;
    }
}

void
vmwareSendSVGACmdUpdate(VMWAREPtr pVMWARE, BoxPtr pBB)
{
    vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_UPDATE);
    vmwareWriteWordToFIFO(pVMWARE, pBB->x1);
    vmwareWriteWordToFIFO(pVMWARE, pBB->y1);
    vmwareWriteWordToFIFO(pVMWARE, pBB->x2 - pBB->x1);
    vmwareWriteWordToFIFO(pVMWARE, pBB->y2 - pBB->y1);
}

static void
vmwareSendSVGACmdUpdateFullScreen(VMWAREPtr pVMWARE)
{
    BoxRec BB;

    BB.x1 = 0;
    BB.y1 = 0;
    BB.x2 = pVMWARE->ModeReg.svga_reg_width;
    BB.y2 = pVMWARE->ModeReg.svga_reg_height;
    vmwareSendSVGACmdUpdate(pVMWARE, &BB);
}

static CARD32
vmwareCalculateWeight(CARD32 mask)
{
    CARD32 weight;

    for (weight = 0; mask; mask >>= 1) {
	if (mask & 1) {
	    weight++;
	}
    }
    return weight;
}

/*
 *-----------------------------------------------------------------------------
 *
 * VMXGetVMwareSvgaId --
 *
 *    Retrieve the SVGA_ID of the VMware SVGA adapter.
 *    This function should hide any backward compatibility mess.
 *
 * Results:
 *    The SVGA_ID_* of the present VMware adapter.
 *
 * Side effects:
 *    ins/outs
 *
 *-----------------------------------------------------------------------------
 */

static uint32
VMXGetVMwareSvgaId(VMWAREPtr pVMWARE)
{
   uint32 vmware_svga_id;

   /* Any version with any SVGA_ID_* support will initialize SVGA_REG_ID
    * to SVGA_ID_0 to support versions of this driver with SVGA_ID_0.
    *
    * Versions of SVGA_ID_0 ignore writes to the SVGA_REG_ID register.
    *
    * Versions of SVGA_ID_1 will allow us to overwrite the content
    * of the SVGA_REG_ID register only with the values SVGA_ID_0 or SVGA_ID_1.
    *
    * Versions of SVGA_ID_2 will allow us to overwrite the content
    * of the SVGA_REG_ID register only with the values SVGA_ID_0 or SVGA_ID_1
    * or SVGA_ID_2.
    */

   vmwareWriteReg(pVMWARE, SVGA_REG_ID, SVGA_ID_2);
   vmware_svga_id = vmwareReadReg(pVMWARE, SVGA_REG_ID);
   if (vmware_svga_id == SVGA_ID_2) {
      return SVGA_ID_2;
   }

   vmwareWriteReg(pVMWARE, SVGA_REG_ID, SVGA_ID_1);
   vmware_svga_id = vmwareReadReg(pVMWARE, SVGA_REG_ID);
   if (vmware_svga_id == SVGA_ID_1) {
      return SVGA_ID_1;
   }

   if (vmware_svga_id == SVGA_ID_0) {
      return SVGA_ID_0;
   }

   /* No supported VMware SVGA devices found */
   return SVGA_ID_INVALID;
}


/*
 *----------------------------------------------------------------------
 *
 *  RewriteTagString --
 *
 *      Rewrites the given string, removing the $Name:  $, and
 *      replacing it with the contents.  The output string must
 *      have enough room, or else.
 *
 * Results:
 *
 *      Output string updated.
 *
 * Side effects:
 *      None.
 *
 *----------------------------------------------------------------------
 */

static void
RewriteTagString(const char *istr, char *ostr, int osize)
{
    int chr;
    Bool inTag = FALSE;
    char *op = ostr;

    do {
	chr = *istr++;
	if (chr == '$') {
	    if (inTag) {
		inTag = FALSE;
		for (; op > ostr && op[-1] == ' '; op--) {
		}
		continue;
	    }
	    if (strncmp(istr, "Name:", 5) == 0) {
		istr += 5;
		istr += strspn(istr, " ");
		inTag = TRUE;
		continue;
	    }
	}
	*op++ = chr;
    } while (chr);
}

static void
VMWAREIdentify(int flags)
{
	xf86PrintChipsets(VMWARE_NAME, "driver for VMware SVGA", VMWAREChipsets);
}

static const OptionInfoRec *
VMWAREAvailableOptions(int chipid, int busid)
{
	return VMWAREOptions;
}

static Bool
VMWAREPreInit(ScrnInfoPtr pScrn, int flags)
{
	MessageType from;
	VMWAREPtr pVMWARE;
	OptionInfoPtr options;
	int bpp24flags;
	uint32 id;
	int i;
	ClockRange* clockRanges;

	if (flags & PROBE_DETECT) {
		return FALSE;
	}

	if (pScrn->numEntities != 1) {
		return FALSE;
	}

	if (!VMWAREGetRec(pScrn)) {
		return FALSE;
	}
	pVMWARE = VMWAREPTR(pScrn);

	pVMWARE->pEnt = xf86GetEntityInfo(pScrn->entityList[0]);
	if (pVMWARE->pEnt->location.type != BUS_PCI) {
		return FALSE;
	}
	pVMWARE->PciInfo = xf86GetPciInfoForEntity(pVMWARE->pEnt->index);
	if (pVMWARE->PciInfo == NULL) {
		return FALSE;
	}

	if (pVMWARE->PciInfo->chipType == PCI_CHIP_VMWARE0710) {
		pVMWARE->indexReg =
		   SVGA_LEGACY_BASE_PORT + SVGA_INDEX_PORT*sizeof(uint32);
		pVMWARE->valueReg =
		   SVGA_LEGACY_BASE_PORT + SVGA_VALUE_PORT*sizeof(uint32);
	} else {
		pVMWARE->indexReg =
		   pVMWARE->PciInfo->ioBase[0] + SVGA_INDEX_PORT;
		pVMWARE->valueReg =
		   pVMWARE->PciInfo->ioBase[0] + SVGA_VALUE_PORT;
	}
	xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	      "VMware SVGA regs at (0x%04x, 0x%04x)\n",
	      pVMWARE->indexReg, pVMWARE->valueReg);

	id = VMXGetVMwareSvgaId(pVMWARE);
	if (id == SVGA_ID_0 || id == SVGA_ID_INVALID) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		      "No supported VMware SVGA found (read ID 0x%08x).\n", id);
		return FALSE;
	}

	if (!xf86LoadSubModule(pScrn, "vgahw")) {
		return FALSE;
	}

	xf86LoaderReqSymLists(vgahwSymbols, NULL);

	if (!vgaHWGetHWRec(pScrn)) {
		return FALSE;
	}

	pVMWARE->PciTag = pciTag(pVMWARE->PciInfo->bus, pVMWARE->PciInfo->device,
				 pVMWARE->PciInfo->func);
	pVMWARE->Primary = xf86IsPrimaryPci(pVMWARE->PciInfo);

	pScrn->monitor = pScrn->confScreen->monitor;

#define ACCELERATE_OPS
#ifdef ACCELERATE_OPS
	pVMWARE->vmwareCapability = vmwareReadReg(pVMWARE, SVGA_REG_CAPABILITIES);
#else
	pVMWARE->vmwareCapability = 0;
#endif

        if (pVMWARE->vmwareCapability & SVGA_CAP_8BIT_EMULATION) {
	        pVMWARE->bitsPerPixel =
                   vmwareReadReg(pVMWARE, SVGA_REG_HOST_BITS_PER_PIXEL);
                vmwareWriteReg(pVMWARE,
                               SVGA_REG_BITS_PER_PIXEL, pVMWARE->bitsPerPixel);
        } else {
	        pVMWARE->bitsPerPixel =
                   vmwareReadReg(pVMWARE, SVGA_REG_BITS_PER_PIXEL);
        }
	pVMWARE->depth = vmwareReadReg(pVMWARE, SVGA_REG_DEPTH);
	pVMWARE->videoRam = vmwareReadReg(pVMWARE, SVGA_REG_FB_MAX_SIZE);
	pVMWARE->memPhysBase = vmwareReadReg(pVMWARE, SVGA_REG_FB_START);
	pVMWARE->maxWidth = vmwareReadReg(pVMWARE, SVGA_REG_MAX_WIDTH);
	pVMWARE->maxHeight = vmwareReadReg(pVMWARE, SVGA_REG_MAX_HEIGHT);
	pVMWARE->cursorDefined = FALSE;
	pVMWARE->mouseHidden = FALSE;

        if (pVMWARE->vmwareCapability & SVGA_CAP_CURSOR_BYPASS_2) {
                pVMWARE->cursorRemoveFromFB = SVGA_CURSOR_ON_REMOVE_FROM_FB;
                pVMWARE->cursorRestoreToFB = SVGA_CURSOR_ON_RESTORE_TO_FB;
        } else {
                pVMWARE->cursorRemoveFromFB = SVGA_CURSOR_ON_HIDE;
                pVMWARE->cursorRestoreToFB = SVGA_CURSOR_ON_SHOW;
        }

	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED, 2, "caps:  0x%08X\n", pVMWARE->vmwareCapability);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED, 2, "depth: %d\n", pVMWARE->depth);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED, 2, "bpp:   %d\n", pVMWARE->bitsPerPixel);

	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED, 2, "vram:  %d\n", pVMWARE->videoRam);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED, 2, "pbase: %p\n", pVMWARE->memPhysBase);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED, 2, "mwidt: %d\n", pVMWARE->maxWidth);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED, 2, "mheig: %d\n", pVMWARE->maxHeight);

	switch (pVMWARE->depth) {
		case 16:
		        /*
			 * In certain cases, the Windows host appears to
			 * report 16 bpp and 16 depth but 555 weight.  Just
			 * silently convert it to depth of 15.
			 */
			if (pVMWARE->bitsPerPixel == 16 &&
			    pVMWARE->weight.green == 5)
				pVMWARE->depth = 15;
		case 8:
		case 15:
			bpp24flags = NoDepth24Support;
			break;

		case 32:
			/*
			 * There is no 32 bit depth, apparently it can get
			 * reported this way sometimes on the Windows host.
			 */
			if (pVMWARE->bitsPerPixel == 32)
				pVMWARE->depth = 24;
		case 24:
			if (pVMWARE->bitsPerPixel == 24)
				bpp24flags = Support24bppFb;
			else
				bpp24flags = Support32bppFb;
			break;
		default:
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
				   "Adapter is using an unsupported depth (%d).\n",
				   pVMWARE->depth);
			return FALSE;
	}

	if (!xf86SetDepthBpp(pScrn, pVMWARE->depth, pVMWARE->bitsPerPixel, pVMWARE->bitsPerPixel, bpp24flags)) {
		return FALSE;
	}

	if (pScrn->bitsPerPixel != pVMWARE->bitsPerPixel) {
                if (pScrn->bitsPerPixel == 8 &&
                    pVMWARE->vmwareCapability & SVGA_CAP_8BIT_EMULATION) {
                        vmwareWriteReg(pVMWARE, SVGA_REG_BITS_PER_PIXEL, 8);
                        pVMWARE->bitsPerPixel =
                           vmwareReadReg(pVMWARE, SVGA_REG_BITS_PER_PIXEL);
                } else {
                        xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                           "Currently unavailable depth/bpp of %d/%d requested.\n"
                           "\tThe guest X server must run at the same depth and bpp as the host\n"
                           "\t(which are currently %d/%d).  This is automatically detected.  Please\n"
                           "\tdo not specify a depth on the command line or via the config file.\n",
                            pScrn->depth, pScrn->bitsPerPixel,
                            pVMWARE->depth, pVMWARE->bitsPerPixel);
		        return FALSE;
                }
	}

        /*
         * Reread depth and defer reading the colour registers until here
         * in case we changed bpp above.
         */

	pVMWARE->depth = vmwareReadReg(pVMWARE, SVGA_REG_DEPTH);
	pVMWARE->weight.red =
           vmwareCalculateWeight(vmwareReadReg(pVMWARE, SVGA_REG_RED_MASK));
	pVMWARE->weight.green =
           vmwareCalculateWeight(vmwareReadReg(pVMWARE, SVGA_REG_GREEN_MASK));
	pVMWARE->weight.blue =
           vmwareCalculateWeight(vmwareReadReg(pVMWARE, SVGA_REG_BLUE_MASK));
	pVMWARE->offset.blue = 0;
	pVMWARE->offset.green = pVMWARE->weight.blue;
	pVMWARE->offset.red = pVMWARE->weight.green + pVMWARE->offset.green;
	pVMWARE->defaultVisual = vmwareReadReg(pVMWARE, SVGA_REG_PSEUDOCOLOR) ?
           PseudoColor : TrueColor;

	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED,
                       2, "depth: %d\n", pVMWARE->depth);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED,
                       2, "bpp:   %d\n", pVMWARE->bitsPerPixel);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED,
                       2, "w.red: %d\n", pVMWARE->weight.red);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED,
                       2, "w.grn: %d\n", pVMWARE->weight.green);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED,
                       2, "w.blu: %d\n", pVMWARE->weight.blue);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_PROBED,
                       2, "vis:   %d\n", pVMWARE->defaultVisual);

	if (pScrn->depth != pVMWARE->depth) {
                xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                           "Currently unavailable depth of %d requested.\n"
                           "\tThe guest X server must run at the same depth as the host (which\n"
                           "\tis currently %d).  This is automatically detected.  Please do not\n"
                           "\tspecify a depth on the command line or via the config file.\n",
                            pScrn->depth, pVMWARE->depth);
                return FALSE;
	}
	xf86PrintDepthBpp(pScrn);

#if 0
	if (pScrn->depth == 24 && pix24bpp == 0) {
		pix24bpp = xf86GetBppFromDepth(pScrn, 24);
	}
#endif

	if (pScrn->depth > 8) {
		rgb zeros = { 0, 0, 0 };

		if (!xf86SetWeight(pScrn, pVMWARE->weight, zeros)) {
			return FALSE;
		}
		/* FIXME check returned weight */
	}
	if (!xf86SetDefaultVisual(pScrn, pVMWARE->defaultVisual)) {
		return FALSE;
	}
	if (pScrn->defaultVisual != pVMWARE->defaultVisual) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "Given visual (%d) is not supported by this driver (%d is required)\n",
			   pScrn->defaultVisual, pVMWARE->defaultVisual);
		return FALSE;
	}
#if 0
	bytesPerPixel = pScrn->bitsPerPixel / 8;
#endif
	pScrn->progClock = TRUE;

#if 0 /* MGA does not do this */
	if (pScrn->visual != 0) {	/* FIXME */
		/* print error message */
		return FALSE;
	}
#endif

	xf86CollectOptions(pScrn, NULL);
	if (!(options = xalloc(sizeof(VMWAREOptions))))
		return FALSE;
	memcpy(options, VMWAREOptions, sizeof(VMWAREOptions));
	xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, options);

	if (pScrn->depth <= 8) {
		pScrn->rgbBits = 8;
	}

	from = X_PROBED;
	pScrn->chipset = (char*)xf86TokenToString(VMWAREChipsets, pVMWARE->PciInfo->chipType);

	if (!pScrn->chipset) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "ChipID 0x%04 is not recognised\n", pVMWARE->PciInfo->chipType);
		return FALSE;
	}

	from = X_DEFAULT;
	pVMWARE->hwCursor = TRUE;
	if (xf86GetOptValBool(options, OPTION_HW_CURSOR, &pVMWARE->hwCursor)) {
		from = X_CONFIG;
	}
	if (pVMWARE->hwCursor && !(pVMWARE->vmwareCapability & SVGA_CAP_CURSOR)) {
		xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "HW cursor is not supported in this configuration\n");
		from = X_PROBED;
		pVMWARE->hwCursor = FALSE;
	}
	xf86DrvMsg(pScrn->scrnIndex, from, "Using %s cursor\n",
		pVMWARE->hwCursor ? "HW" : "SW");
	if (xf86IsOptionSet(options, OPTION_NOACCEL)) {
		pVMWARE->noAccel = TRUE;
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "Acceleration disabled\n");
		pVMWARE->vmwareCapability = 0;
	} else {
		pVMWARE->noAccel = FALSE;
	}
	pScrn->videoRam = pVMWARE->videoRam / 1024;
	pScrn->memPhysBase = pVMWARE->memPhysBase;
	xfree(options);

	{
		Gamma zeros = { 0.0, 0.0, 0.0 };
		if (!xf86SetGamma(pScrn, zeros)) {
			return FALSE;
		}
	}
#if 0
	if ((i = xf86GetPciInfoForScreen(pScrn->scrnIndex, &pciList, NULL)) != 1) {
		/* print error message */
		VMWAREFreeRec(pScrn);
		if (i > 0) {
			xfree(pciList);
		}
		return FALSE;
	}
#endif
	clockRanges = xnfcalloc(sizeof(ClockRange), 1);
	clockRanges->next = NULL;
	clockRanges->minClock = 1;
	clockRanges->maxClock = 400000000;
	clockRanges->clockIndex = -1;
	clockRanges->interlaceAllowed = FALSE;
	clockRanges->doubleScanAllowed = FALSE;
	clockRanges->ClockMulFactor = 1;
	clockRanges->ClockDivFactor = 1;

	i = xf86ValidateModes(pScrn, pScrn->monitor->Modes, pScrn->display->modes,
			clockRanges, NULL, 256, pVMWARE->maxWidth, 32 * 32,
			128, pVMWARE->maxHeight,
			pScrn->display->virtualX, pScrn->display->virtualY,
			pVMWARE->videoRam,
			LOOKUP_BEST_REFRESH);
	if (i == -1) {
		VMWAREFreeRec(pScrn);
		return FALSE;
	}
	xf86PruneDriverModes(pScrn);
	if (i == 0 || pScrn->modes == NULL) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No valid modes found\n");
		VMWAREFreeRec(pScrn);
		return FALSE;
	}
	xf86SetCrtcForModes(pScrn, INTERLACE_HALVE_V);
	pScrn->currentMode = pScrn->modes;
	xf86PrintModes(pScrn);
	xf86SetDpi(pScrn, 0, 0);
	if (!xf86LoadSubModule(pScrn, "fb")) {
		VMWAREFreeRec(pScrn);
		return FALSE;
	}
	xf86LoaderReqSymLists(fbSymbols, NULL);
#if 0
	/* XXX This driver doesn't use XAA! */
	if (!pVMWARE->noAccel || pVMWARE->hwCursor) {
		if (!xf86LoadSubModule(pScrn, "xaa")) {
			VMWAREFreeRec(pScrn);
			return FALSE;
		}
		xf86LoaderReqSymLists(xaaSymbols, NULL);
	}
#endif
	return TRUE;
}

static Bool
VMWAREMapMem(ScrnInfoPtr pScrn)
{
	VMWAREPtr pVMWARE;

	pVMWARE = VMWAREPTR(pScrn);

	pVMWARE->FbBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_FRAMEBUFFER, pVMWARE->PciTag, pVMWARE->memPhysBase, pVMWARE->videoRam);
	if (!pVMWARE->FbBase)
		return FALSE;
	pVMWARE->FbSize = pVMWARE->videoRam;

	VmwareLog(("FB Mapped: %p/%u -> %p/%u\n",
			pVMWARE->memPhysBase, pVMWARE->videoRam,
			pVMWARE->FbBase, pVMWARE->FbSize));
	return TRUE;
}

static Bool
VMWAREUnmapMem(ScrnInfoPtr pScrn)
{
	VMWAREPtr pVMWARE;

	pVMWARE = VMWAREPTR(pScrn);

	VmwareLog(("Unmapped: %p/%u\n", pVMWARE->FbBase, pVMWARE->FbSize));

	xf86UnMapVidMem(pScrn->scrnIndex, pVMWARE->FbBase, pVMWARE->FbSize);
	pVMWARE->FbBase = NULL;
	return TRUE;
}

static void
VMWARESave(ScrnInfoPtr pScrn)
{
	vgaHWPtr hwp = VGAHWPTR(pScrn);
	vgaRegPtr vgaReg = &hwp->SavedReg;
	VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
	VMWARERegPtr vmwareReg = &pVMWARE->SavedReg;

	vgaHWSave(pScrn, vgaReg, VGA_SR_ALL);

	vmwareReg->svga_reg_enable = 0;
	/* FIXME: Save VMWARE state */
}

static void
VMWARERestoreRegs(ScrnInfoPtr pScrn, VMWARERegPtr vmwareReg)
{
	VMWAREPtr pVMWARE = VMWAREPTR(pScrn);

	if (vmwareReg->svga_reg_enable) {
		vmwareWriteReg(pVMWARE, SVGA_REG_WIDTH, vmwareReg->svga_reg_width);
		vmwareWriteReg(pVMWARE, SVGA_REG_HEIGHT, vmwareReg->svga_reg_height);
		vmwareWriteReg(pVMWARE, SVGA_REG_ENABLE, vmwareReg->svga_reg_enable);
		vmwareWriteReg(pVMWARE, SVGA_REG_GUEST_ID, GUEST_OS_LINUX);
	} else {
		vmwareWriteReg(pVMWARE, SVGA_REG_ENABLE, vmwareReg->svga_reg_enable);
	}
}

static void
VMWARERestore(ScrnInfoPtr pScrn)
{
	vgaHWPtr hwp = VGAHWPTR(pScrn);
	vgaRegPtr vgaReg = &hwp->SavedReg;
	VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
	VMWARERegPtr vmwareReg = &pVMWARE->SavedReg;

	/* FIXME: Sync */
	vgaHWProtect(pScrn, TRUE);
	VMWARERestoreRegs(pScrn, vmwareReg);
	vgaHWRestore(pScrn, vgaReg, VGA_SR_ALL);
	vgaHWProtect(pScrn, FALSE);
}

static Bool
VMWAREModeInit(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
	vgaHWPtr hwp = VGAHWPTR(pScrn);
	vgaRegPtr vgaReg = &hwp->ModeReg;
	VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
	VMWARERegPtr vmwareReg = &pVMWARE->ModeReg;

	vgaHWUnlock(hwp);
	if (!vgaHWInit(pScrn, mode))
		return FALSE;
	pScrn->vtSema = TRUE;

	if (!vgaHWInit(pScrn, mode))
		return FALSE;

	vmwareReg->svga_reg_enable = 1;
	vmwareReg->svga_reg_width = max(mode->HDisplay, pScrn->virtualX);
	vmwareReg->svga_reg_height = max(mode->VDisplay, pScrn->virtualY);

	vgaHWProtect(pScrn, TRUE);

	vgaHWRestore(pScrn, vgaReg, VGA_SR_ALL);
	VMWARERestoreRegs(pScrn, vmwareReg);

	VmwareLog(("Required mode: %ux%u\n", mode->HDisplay, mode->VDisplay));
	VmwareLog(("Virtual:       %ux%u\n", pScrn->virtualX, pScrn->virtualY));
	VmwareLog(("dispWidth:     %u\n", pScrn->displayWidth));
	pVMWARE->fbOffset = vmwareReadReg(pVMWARE, SVGA_REG_FB_OFFSET);
	pVMWARE->fbPitch = vmwareReadReg(pVMWARE, SVGA_REG_BYTES_PER_LINE);

	pScrn->displayWidth = (pVMWARE->fbPitch * 8) / ((pScrn->bitsPerPixel + 7) & ~7);
	VmwareLog(("fbOffset:      %u\n", pVMWARE->fbOffset));
	VmwareLog(("fbPitch:       %u\n", pVMWARE->fbPitch));
	VmwareLog(("New dispWidth: %u\n", pScrn->displayWidth));

	vgaHWProtect(pScrn, FALSE);
	pVMWARE->CurrentLayout.mode = mode;
	return TRUE;
}

static void
VMWAREAdjustFrame(int scrnIndex, int x, int y, int flags)
{
	/* FIXME */
}

static void
VMWAREInitFIFO(ScrnInfoPtr pScrn)
{
	VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
	CARD32* vmwareFIFO;

	pVMWARE->mmioPhysBase = vmwareReadReg(pVMWARE, SVGA_REG_MEM_START);
	pVMWARE->mmioSize = vmwareReadReg(pVMWARE, SVGA_REG_MEM_SIZE) & ~3;
	pVMWARE->mmioVirtBase = xf86MapPciMem(pScrn->scrnIndex, VIDMEM_MMIO, pVMWARE->PciTag, pVMWARE->mmioPhysBase, pVMWARE->mmioSize);
	vmwareFIFO = pVMWARE->vmwareFIFO = (CARD32*)pVMWARE->mmioVirtBase;
	vmwareFIFO[SVGA_FIFO_MIN] = 4 * sizeof(CARD32);
	vmwareFIFO[SVGA_FIFO_MAX] = pVMWARE->mmioSize;
	vmwareFIFO[SVGA_FIFO_NEXT_CMD] = 4 * sizeof(CARD32);
	vmwareFIFO[SVGA_FIFO_STOP] = 4 * sizeof(CARD32);
	pVMWARE->vmwareFIFOMarkSet = FALSE;
	vmwareWriteReg(pVMWARE, SVGA_REG_CONFIG_DONE, 1);
}

static void
VMWAREStopFIFO(ScrnInfoPtr pScrn)
{
	VMWAREPtr pVMWARE = VMWAREPTR(pScrn);

	/* ??? */
	vmwareWriteReg(pVMWARE, SVGA_REG_CONFIG_DONE, 0);
}

static Bool
VMWARECloseScreen(int scrnIndex, ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	VMWARERestore(pScrn);
	VMWAREStopFIFO(pScrn);
	VMWAREUnmapMem(pScrn);
	pScrn->vtSema = FALSE;
	ScreenFromPrivate(pScreen, pScrn);
	return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}

static Bool
VMWARESaveScreen(ScreenPtr pScreen, int mode)
{
	return vgaHWSaveScreen(pScreen, mode);
}

static void
VMWARELoadPalette(ScrnInfoPtr pScrn, int numColors, int* indices,
		LOCO* colors, VisualPtr pVisual)
{
	VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
	int i;

	for (i = 0; i < numColors; i++) {
		vmwareWriteReg(pVMWARE, SVGA_PALETTE_BASE + *indices * 3 + 0, colors[*indices].red);
		vmwareWriteReg(pVMWARE, SVGA_PALETTE_BASE + *indices * 3 + 1, colors[*indices].green);
		vmwareWriteReg(pVMWARE, SVGA_PALETTE_BASE + *indices * 3 + 2, colors[*indices].blue);
		indices++;
	}

	pVMWARE->checkCursorColor = TRUE;
	VmwareLog(("Palette loading done\n"));
}

static Bool
VMWAREScreenInit(int scrnIndex, ScreenPtr pScreen, int argc, char **argv)
{
	ScrnInfoPtr pScrn;
	vgaHWPtr hwp;
	VMWAREPtr pVMWARE;
	Bool ret;

	/* Get the ScrnInfoRec */
	pScrn = xf86Screens[pScreen->myNum];
	pVMWARE = VMWAREPTR(pScrn);

        /*
         * If using the vgahw module, its data structures and related
         * things are typically initialised/mapped here.
         */

	hwp = VGAHWPTR(pScrn);
	vgaHWGetIOBase(hwp);

	/* Save the current video state */
	VMWARESave(pScrn);

	VMWAREInitFIFO(pScrn);

	/* Initialise the first mode */
	VMWAREModeInit(pScrn, pScrn->currentMode);

	/* Set the viewport if supported */
	VMWAREAdjustFrame(scrnIndex, pScrn->frameX0, pScrn->frameY0, 0);

	/*
	 * Setup the screen's visuals, and initialise the framebuffer
	 * code.
	 */
	VMWAREMapMem(pScrn);

	/* Reset the visual list */
	miClearVisualTypes();

	/*
	 * Setup the visuals supported.  This driver only supports
	 * TrueColor for bpp > 8, so the default set of visuals isn't
	 * acceptable.  To deal with this, call miSetVisualTypes with
	 * the appropriate visual mask.
	 */

	if (pScrn->bitsPerPixel > 8) {
		if (!miSetVisualTypes(pScrn->depth, TrueColorMask,
			pScrn->rgbBits, pScrn->defaultVisual)) {
			return FALSE;
		}
	} else {
		if (!miSetVisualTypes(pScrn->depth,
				miGetDefaultVisualMask(pScrn->depth),
			pScrn->rgbBits, pScrn->defaultVisual)) {
			return FALSE;
		}
	}

	miSetPixmapDepths ();

	/*
	 * Initialise the framebuffer.
	 */

	ret = fbScreenInit (pScreen, pVMWARE->FbBase,
			    pScrn->virtualX, pScrn->virtualY,
			    pScrn->xDpi, pScrn->yDpi,
			    pScrn->displayWidth,
			    pScrn->bitsPerPixel);
    
	if (!ret)
		return FALSE;

        /* Override the default mask/offset settings */
        if (pScrn->bitsPerPixel > 8) {
	int i;
		VisualPtr visual;

		for (i = 0, visual = pScreen->visuals;
			i < pScreen->numVisuals; i++, visual++) {
			if ((visual->class | DynamicClass) == DirectColor) {
				visual->offsetRed = pScrn->offset.red;
		                visual->offsetGreen = pScrn->offset.green;
		                visual->offsetBlue = pScrn->offset.blue;
				visual->redMask = pScrn->mask.red;
				visual->greenMask = pScrn->mask.green;
			        visual->blueMask = pScrn->mask.blue;
			}
		}
	}

	/* must be after RGB ordering fixed */
	fbPictureInit (pScreen, 0, 0);

	/*
	 * Wrap the CloseScreen vector and set SaveScreen.
	 */
	ScreenToPrivate(pScreen, pScrn);
        /*
         * If backing store is to be supported (as is usually the case),
         * initialise it.
         */
        miInitializeBackingStore(pScreen);

        /*
         * Set initial black & white colourmap indices.
         */
        xf86SetBlackWhitePixels(pScreen);

        /*
         * Install colourmap functions.  If using the vgahw module,
         * vgaHandleColormaps would usually be called here.
         */

        /*
         * Initialise cursor functions.  This example is for the mi
         * software cursor.
         */
	if (pVMWARE->hwCursor) {
		vmwareCursorInit(0, pScreen);
	} else {
	        miDCInitialize(pScreen, xf86GetPointerScreenFuncs());
	}

    	if (!fbCreateDefColormap(pScreen))
	    return FALSE;

	if (!xf86HandleColormaps(pScreen, 256, 8,
		VMWARELoadPalette, NULL,
		CMAP_PALETTED_TRUECOLOR | CMAP_RELOAD_ON_MODE_SWITCH)) {
	    return FALSE;
	}


	/* Report any unused options (only for the first generation) */
	if (serverGeneration == 1) {
		xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);
	}

	vmwareSendSVGACmdUpdateFullScreen(pVMWARE);
	if (pVMWARE->hwCursor) {
		vmwareRestoreCursor(pScreen);
	}
	/* Done */
	return TRUE;
}

static Bool
VMWARESwitchMode(int scrnIndex, DisplayModePtr mode, int flags)
{
	return VMWAREModeInit(xf86Screens[scrnIndex], mode);
}

static Bool
VMWAREEnterVT(int scrnIndex, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	return VMWAREModeInit(pScrn, pScrn->currentMode);
}

static void
VMWARELeaveVT(int scrnIndex, int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	VMWARERestore(pScrn);
}

static void
VMWAREFreeScreen(int scrnIndex, int flags)
{
	/*
	 * If the vgahw module is used vgaHWFreeHWRec() would be called
	 * here.
	 */
	VMWAREFreeRec(xf86Screens[scrnIndex]);
}

static Bool
VMWAREValidMode(int scrnIndex, DisplayModePtr mode, Bool verbose, int flags)
{
	/* FIXME... possible */
	return MODE_OK;
}

static Bool
VMWAREProbe(DriverPtr drv, int flags)
{
	int numDevSections, numUsed;
	GDevPtr *devSections;
	int *usedChips;
	int i;
	Bool foundScreen = FALSE;
	char buildString[sizeof(VMWAREBuildStr)];

	RewriteTagString(VMWAREBuildStr, buildString, sizeof(VMWAREBuildStr));
	xf86MsgVerb(X_PROBED, 4, "%s", buildString);

	numDevSections = xf86MatchDevice(VMWARE_DRIVER_NAME, &devSections);
	if (numDevSections <= 0) {
#ifdef DEBUG
		xf86MsgVerb(X_ERROR, 0, "No vmware driver section\n");
#endif
		return FALSE;
	}
	if (xf86GetPciVideoInfo()) {
		VmwareLog(("Some PCI Video Info Exists\n"));
		numUsed = xf86MatchPciInstances(VMWARE_NAME, PCI_VENDOR_VMWARE,
				VMWAREChipsets, VMWAREPciChipsets, devSections,
				numDevSections, drv, &usedChips);
		xfree(devSections);
		if (numUsed <= 0)
			return FALSE;
		if (flags & PROBE_DETECT)
			foundScreen = TRUE;
		else for (i = 0; i < numUsed; i++) {
			ScrnInfoPtr pScrn = NULL;

			VmwareLog(("Even some VMware SVGA PCI instances exists\n"));
			pScrn = xf86ConfigPciEntity(pScrn, flags, usedChips[i],
					VMWAREPciChipsets, NULL, NULL, NULL,
					NULL, NULL);
			if (pScrn) {
				VmwareLog(("And even configuration suceeded\n"));
				pScrn->driverVersion = VERSION;
				pScrn->driverName = VMWARE_DRIVER_NAME;
				pScrn->name = VMWARE_NAME;
				pScrn->Probe = VMWAREProbe;
				pScrn->PreInit = VMWAREPreInit;
				pScrn->ScreenInit = VMWAREScreenInit;
				pScrn->SwitchMode = VMWARESwitchMode;
				pScrn->AdjustFrame = VMWAREAdjustFrame;
				pScrn->EnterVT = VMWAREEnterVT;
				pScrn->LeaveVT = VMWARELeaveVT;
				pScrn->FreeScreen = VMWAREFreeScreen;
				pScrn->ValidMode = VMWAREValidMode;
				foundScreen = TRUE;
			}
		}
		xfree(usedChips);
	}
	return foundScreen;
}

DriverRec VMWARE = {
	VERSION,
	VMWARE_DRIVER_NAME,
	VMWAREIdentify,
	VMWAREProbe,
	VMWAREAvailableOptions,
	NULL,
	0
};

#ifdef XFree86LOADER
static MODULESETUPPROTO(vmwareSetup);

XF86ModuleData vmwareModuleData = { &vmwareVersRec, vmwareSetup, NULL };

static pointer
vmwareSetup(pointer module, pointer opts, int *errmaj, int *errmin)
{
	static Bool setupDone = FALSE;

	if (!setupDone) {
		setupDone = TRUE;
		xf86AddDriver(&VMWARE, module, 0);

		LoaderRefSymLists(vgahwSymbols, fbSymbols, NULL);

		return (pointer)1;
	}
	if (errmaj) {
		*errmaj = LDR_ONCEONLY;
	}
	return NULL;
}
#endif	/* XFree86LOADER */

ScreenPtr funcglob;
ScrnInfoPtr scrnglob;
int vmwareGCPrivateIndex;

static void
ScreenToPrivate(ScreenPtr pScreen, ScrnInfoPtr pScrn)
{
    VMWAREPtr pVMWARE = VMWAREPTR(pScrn);

    pVMWARE->ScrnFuncs = *pScreen;
    funcglob = &pVMWARE->ScrnFuncs;
    scrnglob = pScrn;

    pScreen->CloseScreen = VMWARECloseScreen;
    pScreen->SaveScreen = VMWARESaveScreen;

#if 0
    pScreen->QueryBestSize = vmwareQueryBestSize;
    pScreen->GetImage = vmwareGetImage;
    pScreen->GetSpans = vmwareGetSpans;
    pScreen->PointerNonInterestBox = vmwarePointerNonInterestBox;
    pScreen->SourceValidate = vmwareSourceValidate;

    pScreen->CreateWindow           = vmwareCreateWindow;
    pScreen->DestroyWindow          = vmwareDestroyWindow;
    pScreen->PositionWindow         = vmwarePositionWindow;
    pScreen->ChangeWindowAttributes = vmwareChangeWindowAttributes;
    pScreen->RealizeWindow          = vmwareRealizeWindow;
    pScreen->UnrealizeWindow        = vmwareUnrealizeWindow;
    pScreen->ValidateTree           = vmwareValidateTree;
//    pScreen->PostValidateTree       = vmwarePostValidateTree;
    pScreen->WindowExposures        = vmwareWindowExposures;
    pScreen->PaintWindowBackground  = vmwarePaintWindowBackground;
    pScreen->PaintWindowBorder      = vmwarePaintWindowBorder;
    pScreen->CopyWindow             = vmwareCopyWindow;
    pScreen->ClearToBackground      = vmwareClearToBackground;
    pScreen->ClipNotify             = vmwareClipNotify;
    pScreen->RestackWindow          = vmwareRestackWindow;

    /* Pixmap procedures */

    pScreen->CreatePixmap           = vmwareCreatePixmap;
    pScreen->DestroyPixmap          = vmwareDestroyPixmap;

    /* Backing store procedures */

    pScreen->SaveDoomedAreas        = vmwareSaveDoomedAreas;
    pScreen->RestoreAreas           = vmwareRestoreAreas;
    pScreen->ExposeCopy             = vmwareExposeCopy;
    pScreen->TranslateBackingStore  = vmwareTranslateBackingStore;
    pScreen->ClearBackingStore      = vmwareClearBackingStore;
    pScreen->DrawGuarantee          = vmwareDrawGuarantee;
    /*
     * A read/write copy of the lower level backing store vector is needed now
     * that the functions can be wrapped.
     */
//    pScreen->BackingStoreFuncs      = vmwareBackingStoreFuncs;

    /* Font procedures */

    pScreen->RealizeFont            = vmwareRealizeFont;
    pScreen->UnrealizeFont          = vmwareUnrealizeFont;

    /* Cursor Procedures */

    pScreen->ConstrainCursor        = vmwareConstrainCursor;
    pScreen->CursorLimits           = vmwareCursorLimits;
    pScreen->DisplayCursor          = vmwareDisplayCursor;
    pScreen->RealizeCursor          = vmwareRealizeCursor;
    pScreen->UnrealizeCursor        = vmwareUnrealizeCursor;
    pScreen->RecolorCursor          = vmwareRecolorCursor;
    pScreen->SetCursorPosition      = vmwareSetCursorPosition;

    /* GC procedures */

    pScreen->CreateGC               = vmwareCreateGC;

    /* Colormap procedures */

    pScreen->CreateColormap         = vmwareCreateColormap;
    pScreen->DestroyColormap        = vmwareDestroyColormap;
    pScreen->InstallColormap        = vmwareInstallColormap;
    pScreen->UninstallColormap      = vmwareUninstallColormap;
    pScreen->ListInstalledColormaps = vmwareListInstalledColormaps;
    pScreen->StoreColors            = vmwareStoreColors;
    pScreen->ResolveColor           = vmwareResolveColor;


    pScreen->GetWindowPixmap        = vmwareGetWindowPixmap;
    pScreen->SetWindowPixmap        = vmwareSetWindowPixmap;
    pScreen->GetScreenPixmap        = vmwareGetScreenPixmap;
    pScreen->SetScreenPixmap        = vmwareSetScreenPixmap;

#endif

    pScreen->PaintWindowBackground  = vmwarePaintWindow;
    pScreen->PaintWindowBorder      = vmwarePaintWindow;
    pScreen->CopyWindow             = vmwareCopyWindow;
    pScreen->CreateGC               = vmwareCreateGC;
    pScreen->GetSpans               = vmwareGetSpans;
    pScreen->GetImage               = vmwareGetImage;
    pScreen->BlockHandler           = vmwareBlockHandler;
    pScreen->SaveDoomedAreas        = vmwareSaveDoomedAreas;
    pScreen->RestoreAreas           = vmwareRestoreAreas;

    vmwareGCPrivateIndex = AllocateGCPrivateIndex();
    if (!AllocateGCPrivate(pScreen, vmwareGCPrivateIndex,
	    sizeof(vmwarePrivGC))) return;


    switch (pScrn->bitsPerPixel) {
    case 8:
	pVMWARE->Pmsk = 0x000000FF;
	break;
    case 16:
	pVMWARE->Pmsk = 0x0000FFFF;
	break;
    case 24:
	pVMWARE->Pmsk = 0x00FFFFFF;
	break;
    case 32:
	pVMWARE->Pmsk = 0xFFFFFFFF;
	break;
    }
}

static void ScreenFromPrivate(ScreenPtr pScreen, ScrnInfoPtr p)
{
    VMWAREPtr pVMWARE = VMWAREPTR(p);
    ScreenPtr save = &pVMWARE->ScrnFuncs;

    /* Random screen procedures */

    pScreen->CloseScreen            = save->CloseScreen;
    pScreen->QueryBestSize          = save->QueryBestSize;
    pScreen->SaveScreen             = save->SaveScreen;
    pScreen->GetImage               = save->GetImage;
    pScreen->GetSpans               = save->GetSpans;
    pScreen->PointerNonInterestBox  = save->PointerNonInterestBox;
    pScreen->SourceValidate         = save->SourceValidate;

    /* Window Procedures */

    pScreen->CreateWindow           = save->CreateWindow;
    pScreen->DestroyWindow          = save->DestroyWindow;
    pScreen->PositionWindow         = save->PositionWindow;
    pScreen->ChangeWindowAttributes = save->ChangeWindowAttributes;
    pScreen->RealizeWindow          = save->RealizeWindow;
    pScreen->UnrealizeWindow        = save->UnrealizeWindow;
    pScreen->ValidateTree           = save->ValidateTree;
    pScreen->PostValidateTree       = save->PostValidateTree;
    pScreen->WindowExposures        = save->WindowExposures;
    pScreen->PaintWindowBackground  = save->PaintWindowBackground;
    pScreen->PaintWindowBorder      = save->PaintWindowBorder;
    pScreen->CopyWindow             = save->CopyWindow;
    pScreen->ClearToBackground      = save->ClearToBackground;
    pScreen->ClipNotify             = save->ClipNotify;
    pScreen->RestackWindow          = save->RestackWindow;

    /* Pixmap procedures */

    pScreen->CreatePixmap           = save->CreatePixmap;
    pScreen->DestroyPixmap          = save->DestroyPixmap;

    /* Backing store procedures */

    pScreen->SaveDoomedAreas        = save->SaveDoomedAreas;
    pScreen->RestoreAreas           = save->RestoreAreas;
    pScreen->ExposeCopy             = save->ExposeCopy;
    pScreen->TranslateBackingStore  = save->TranslateBackingStore;
    pScreen->ClearBackingStore      = save->ClearBackingStore;
    pScreen->DrawGuarantee          = save->DrawGuarantee;
    /*
     * A read/write copy of the lower level backing store vector is needed now
     * that the functions can be wrapped.
     */
    pScreen->BackingStoreFuncs      = save->BackingStoreFuncs;

    /* Font procedures */

    pScreen->RealizeFont            = save->RealizeFont;
    pScreen->UnrealizeFont          = save->UnrealizeFont;

    /* Cursor Procedures */

    pScreen->ConstrainCursor        = save->ConstrainCursor;
    pScreen->CursorLimits           = save->CursorLimits;
    pScreen->DisplayCursor          = save->DisplayCursor;
    pScreen->RealizeCursor          = save->RealizeCursor;
    pScreen->UnrealizeCursor        = save->UnrealizeCursor;
    pScreen->RecolorCursor          = save->RecolorCursor;
    pScreen->SetCursorPosition      = save->SetCursorPosition;

    /* GC procedures */

    pScreen->CreateGC               = save->CreateGC;

    /* Colormap procedures */

    pScreen->CreateColormap         = save->CreateColormap;
    pScreen->DestroyColormap        = save->DestroyColormap;
    pScreen->InstallColormap        = save->InstallColormap;
    pScreen->UninstallColormap      = save->UninstallColormap;
    pScreen->ListInstalledColormaps = save->ListInstalledColormaps;
    pScreen->StoreColors            = save->StoreColors;
    pScreen->ResolveColor           = save->ResolveColor;

    /* Region procedures */

#ifdef NEED_SCREEN_REGIONS
    pScreen->RegionCreate           = save->RegionCreate;
    pScreen->RegionInit             = save->RegionInit;
    pScreen->RegionCopy             = save->RegionCopy;
    pScreen->RegionDestroy          = save->RegionDestroy;
    pScreen->RegionUninit           = save->RegionUninit;
    pScreen->Intersect              = save->Intersect;
    pScreen->Union                  = save->Union;
    pScreen->Subtract               = save->Subtract;
    pScreen->Inverse                = save->Inverse;
    pScreen->RegionReset            = save->RegionReset;
    pScreen->TranslateRegion        = save->TranslateRegion;
    pScreen->RectIn                 = save->RectIn;
    pScreen->PointInRegion          = save->PointInRegion;
    pScreen->RegionNotEmpty         = save->RegionNotEmpty;
    pScreen->RegionBroken           = save->RegionBroken;
    pScreen->RegionBreak            = save->RegionBreak;
    pScreen->RegionEmpty            = save->RegionEmpty;
    pScreen->RegionExtents          = save->RegionExtents;
    pScreen->RegionAppend           = save->RegionAppend;
    pScreen->RegionValidate         = save->RegionValidate;
#endif /* NEED_SCREEN_REGIONS */
    pScreen->BitmapToRegion         = save->BitmapToRegion;
#ifdef NEED_SCREEN_REGIONS
    pScreen->RectsToRegion          = save->RectsToRegion;
#endif /* NEED_SCREEN_REGIONS */
    pScreen->SendGraphicsExpose     = save->SendGraphicsExpose;

    /* os layer procedures */

    pScreen->BlockHandler           = save->BlockHandler;
    pScreen->WakeupHandler          = save->WakeupHandler;

    pScreen->CreateScreenResources  = save->CreateScreenResources;
    pScreen->ModifyPixmapHeader     = save->ModifyPixmapHeader;

    pScreen->GetWindowPixmap        = save->GetWindowPixmap;
    pScreen->SetWindowPixmap        = save->SetWindowPixmap;
    pScreen->GetScreenPixmap        = save->GetScreenPixmap;
    pScreen->SetScreenPixmap        = save->SetScreenPixmap;

    pScreen->MarkWindow             = save->MarkWindow;
    pScreen->MarkOverlappedWindows  = save->MarkOverlappedWindows;
    pScreen->ChangeSaveUnder        = save->ChangeSaveUnder;
    pScreen->PostChangeSaveUnder    = save->PostChangeSaveUnder;
    pScreen->MoveWindow             = save->MoveWindow;
    pScreen->ResizeWindow           = save->ResizeWindow;
    pScreen->GetLayerWindow         = save->GetLayerWindow;
    pScreen->HandleExposures        = save->HandleExposures;
    pScreen->ReparentWindow         = save->ReparentWindow;

#ifdef SHAPE
    pScreen->SetShape               = save->SetShape;
#endif /* SHAPE */

    pScreen->ChangeBorderWidth      = save->ChangeBorderWidth;
    pScreen->MarkUnrealizedWindow   = save->MarkUnrealizedWindow;
}

