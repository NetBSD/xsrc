/* 
 * Id: newport.h,v 1.4 2000/11/29 20:58:10 agx Exp $
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/newport/newport.h,v 1.2 2000/12/06 22:00:46 dawes Exp $ */

#ifndef __NEWPORT_H__
#define __NEWPORT_H__

/*
 * All drivers should include these:
 */
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Resources.h"

#include "xf86cmap.h"

/* xaa & hardware cursor */
#include "xaa.h"
#include "xf86Cursor.h"

/* register definitions of the Newport card */
#include "newport_regs.h"

#define NEWPORT_BASE_ADDR0  0x1f0f0000
#define NEWPORT_BASE_OFFSET 0x0040000
#define NEWPORT_MAX_BOARDS 4

typedef struct {
	unsigned busID;
	int bitplanes; 
	/* revision numbers of the various pieces of silicon */
	unsigned int board_rev, cmap_rev, rex3_rev, xmap9_rev, bt445_rev;
	NewportRegsPtr pNewportRegs;	/* Pointer to REX3 registers */
	npireg_t drawmode1;		/* REX3 drawmode1 common to all drawing operations */

	/* ShadowFB stuff: */
	pointer ShadowPtr;
	unsigned long int ShadowPitch;
	unsigned int Bpp;		/* Bytes per pixel */

	/* wrapped funtions: */
	CloseScreenProcPtr  CloseScreen;

	/* newport register backups: */
	npireg_t txt_drawmode1;		/* Rex3 drawmode1 register */
	unsigned short txt_vc2ctrl;	/* VC2 control register */
	CARD8 txt_xmap9_cfg0;		/* 0. Xmap9's control register */
	CARD8 txt_xmap9_cfg1;		/* 1. Xmap9's control register */
	CARD8 txt_xmap9_mi;		/* Xmap9's mode index register */
	LOCO txt_colormap[256];
} NewportRec, *NewportPtr;

#define NEWPORTPTR(p) ((NewportPtr)((p)->driverPrivate))
#define NEWPORTREGSPTR(p) ((NEWPORTPTR(p))->pNewportRegs)

/* Newport_regs.c */
unsigned short NewportVc2Get(NewportRegsPtr, unsigned char vc2Ireg);
void NewportVc2Set(NewportRegsPtr pNewportRegs, unsigned char vc2Ireg, unsigned short val);
void NewportWait(NewportRegsPtr pNewportRegs);
void NewportBfwait(NewportRegsPtr pNewportRegs);
void NewportXmap9SetModeRegister(NewportRegsPtr pNewportRegs, CARD8 address, CARD32 mode);

/* newort_cmap.c */
void NewportLoadPalette(ScrnInfoPtr pScrn, int numColors, int *indices, 
			LOCO* colors, VisualPtr pVisual);
void NewportRestorePalette(ScrnInfoPtr pScrn);
void NewportBackupPalette(ScrnInfoPtr pScrn);

/* newport_shadow.c */
void NewportRefreshArea8(ScrnInfoPtr pScrn, int num, BoxPtr pbox);

#endif /* __NEWPORT_H__ */
