/*
 * Id: newport_regs.c,v 1.3 2000/11/29 20:58:10 agx Exp $
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/newport/newport_regs.c,v 1.6 2001/12/21 15:37:23 tsi Exp $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "newport.h"

static void NewportXmap9FifoWait(NewportRegsPtr pNewportRegs, unsigned long xmapChip);

void 
NewportVc2Set(NewportRegsPtr pNewportRegs, unsigned char vc2Ireg, unsigned short val)
{
	pNewportRegs->set.dcbmode = (NPORT_DMODE_AVC2 | VC2_REGADDR_INDEX | 
	    NPORT_DMODE_W3 | NPORT_DMODE_ECINC | VC2_PROTOCOL);
	pNewportRegs->set.dcbdata0.all = (vc2Ireg << 24) | (val << 8);
}

unsigned short 
NewportVc2Get(NewportRegsPtr pNewportRegs, unsigned char vc2Ireg)
{
	pNewportRegs->set.dcbmode = (NPORT_DMODE_AVC2 | VC2_REGADDR_INDEX | 
	    NPORT_DMODE_W1 | NPORT_DMODE_ECINC | VC2_PROTOCOL);
	pNewportRegs->set.dcbdata0.bytes.b3 = vc2Ireg;
	pNewportRegs->set.dcbmode = (NPORT_DMODE_AVC2 | VC2_REGADDR_IREG | 
	    NPORT_DMODE_W2 | NPORT_DMODE_ECINC | VC2_PROTOCOL);
	return pNewportRegs->set.dcbdata0.hwords.s1;
}

void 
NewportBackupVc2( ScrnInfoPtr pScrn)
{
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportRegsPtr pNewportRegs = NEWPORTREGSPTR(pScrn);

	pNewport->txt_vc2ctrl = NewportVc2Get( pNewportRegs, VC2_IREG_CONTROL );
	pNewport->txt_vc2cur_x = NewportVc2Get( pNewportRegs, VC2_IREG_CURSX );
	pNewport->txt_vc2cur_y = NewportVc2Get( pNewportRegs, VC2_IREG_CURSY );
}

void 
NewportRestoreVc2( ScrnInfoPtr pScrn)
{
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportRegsPtr pNewportRegs = NEWPORTREGSPTR(pScrn);

	NewportVc2Set( pNewportRegs, VC2_IREG_CONTROL, pNewport->txt_vc2ctrl );
	NewportVc2Set( pNewportRegs, VC2_IREG_CURSX, pNewport->txt_vc2cur_x );
	NewportVc2Set( pNewportRegs, VC2_IREG_CURSY, pNewport->txt_vc2cur_y );
}

void
NewportRestoreVc2Cursor( ScrnInfoPtr pScrn)
{
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportLoadCursorImage( pScrn, (CARD8*)pNewport->txt_vc2cur_data);
}

void
NewportBackupVc2Cursor( ScrnInfoPtr pScrn)
{
	int i;
	CARD16 tmp, *data;
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportRegsPtr pNewportRegs = NEWPORTREGSPTR(pScrn);

	data = (CARD16*)pNewport->txt_vc2cur_data;
	/* address of cursor data in vc2's ram */
        tmp = NewportVc2Get( pNewportRegs, VC2_IREG_CENTRY);
 	/* this is where we want to write to: */
        NewportVc2Set( pNewportRegs, VC2_IREG_RADDR, tmp);
        pNewportRegs->set.dcbmode = (NPORT_DMODE_AVC2 | VC2_REGADDR_RAM |
                               		NPORT_DMODE_W2 | VC2_PROTOCOL);
	/* read cursor data */
        for (i = 0; i < 128; i++) {
		NewportBfwait(pNewportRegs);
		*data = pNewportRegs->set.dcbdata0.hwords.s1;
		data++;
        }
}

/* Sometimes we just have to wait until we can do anything */
void
NewportWait(NewportRegsPtr pNewportRegs)
{
	while(1)
		if(!(pNewportRegs->cset.stat & NPORT_STAT_GBUSY))
			break;
}

void
NewportBfwait(NewportRegsPtr pNewportRegs)
{
	while(1)
		if(!(pNewportRegs->cset.stat & NPORT_STAT_BBUSY))
			break;
}

/* wait til an entry in the Xmap9's Mode Fifo is free (xmapChip = DCB_XMAP0 | DCB_XMAP1) */
static void
NewportXmap9FifoWait(NewportRegsPtr pNewportRegs, unsigned long xmapChip)
{
	while(1) {
		CARD8 val;

		NewportBfwait( pNewportRegs);

		val = NewportXmap9ReadRegister(pNewportRegs, xmapChip,
		    XM9_CRS_FIFO_AVAIL);

		if (val & 0x07)
			break;
	}
}

/*
 * Write a value to the given Xmap9 chip.
 *
 * The caller is responsible for calling NewportBfwait() first in case
 * they want to optimise multiple writes.
 *
 * This can write to DCB_XMAP0, DCB_XMAP1 or both via DCB_XMAPALL.
 *
 * dcbreg is one of the XM9_CRS_* values which represent the already
 * shifted register values, just to make porting code over easier.
 */
void
NewportXmap9WriteRegister(NewportRegsPtr pNewportRegs, unsigned dcbaddr,
    unsigned dcbreg, CARD8 value)
{
	pNewportRegs->set.dcbmode = ( dcbaddr | W_DCB_XMAP9_PROTOCOL |
	    dcbreg | NPORT_DMODE_W1 );
	pNewportRegs->set.dcbdata0.bytes.b3 = value;
}

/*
 * Read a value to the given Xmap9 chip.
 *
 * This will stall the BFIFO until pending writes are completed, so I
 * believe NewportBfwait() isn't required.
 *
 * This must only be used on either DCB_XMAP0 or DCB_XMAP1,
 * /NOT/ DCB_XMAPALL.
 *
 * dcbreg is one of the XM9_CRS_* values which represent the already
 * shifted register values, just to make porting code over easier.
 */
CARD8
NewportXmap9ReadRegister(NewportRegsPtr pNewportRegs, unsigned dcbaddr,
    unsigned dcbreg)
{
	pNewportRegs->set.dcbmode = ( dcbaddr | R_DCB_XMAP9_PROTOCOL |
	    dcbreg | NPORT_DMODE_W1 );
	return (pNewportRegs->set.dcbdata0.bytes.b3);
}

/*
 * Configure the XMAP9 Mode Register at DID address 'address'.
 *
 * The default, "fast", isn't suitable for 1024x768x60Hz,
 * the default resolution for the Indy if there's no recognised
 * monitor.
 */
void
NewportXmap9SetModeRegister(NewportPtr pNewport, CARD8 address, CARD32 mode)
{
	NewportRegsPtr pNewportRegs = pNewport->pNewportRegs;
	uint32_t timing;

	NewportXmap9FifoWait( pNewportRegs, DCB_XMAP0);
	NewportXmap9FifoWait( pNewportRegs, DCB_XMAP1);

	switch (pNewport->XmapTiming) {
	case XmapTimingFast:
		timing = WFAST_DCB_XMAP9_PROTOCOL;
		break;
	case XmapTimingSlow:
		timing = WSLOW_DCB_XMAP9_PROTOCOL;
		break;
	case XmapTimingVerySlow:
		timing = WAYSLOW_DCB_XMAP9_PROTOCOL;
		break;
	default:
		timing = WFAST_DCB_XMAP9_PROTOCOL;
		break;
	}

	pNewportRegs->set.dcbmode = (DCB_XMAP_ALL | timing |
			XM9_CRS_MODE_REG_DATA | NPORT_DMODE_W4 );
	pNewportRegs->set.dcbdata0.all = (address << 24) | ( mode & 0xffffff );
}

CARD32
NewportXmap9GetModeRegister(NewportRegsPtr pNewportRegs, unsigned chip, CARD8 address)
{
	CARD32 dcbaddr, i, val, mode = 0;
	CARD8 index = ( address << 2 );

	if(chip)
		dcbaddr = DCB_XMAP1;
	else
		dcbaddr = DCB_XMAP0;

	/* we have to read one byte at at time */
	for( i = 0; i < 4; i++ ) {
		NewportXmap9FifoWait( pNewportRegs, dcbaddr);

		NewportXmap9WriteRegister(pNewportRegs, dcbaddr,
		    XM9_CRS_MODE_REG_INDEX, (index | i));
		val = NewportXmap9ReadRegister(pNewportRegs, dcbaddr,
		    XM9_CRS_MODE_REG_DATA);
		mode |= (val << ( i * 8 ) );
	}
	return mode;
}

void 
NewportBackupRex3( ScrnInfoPtr pScrn)
{
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportRegsPtr pNewportRegs = NEWPORTREGSPTR(pScrn);

	pNewport->txt_drawmode0 = pNewportRegs->set.drawmode0;
	pNewport->txt_drawmode1 = pNewportRegs->set.drawmode1;
	pNewport->txt_wrmask = pNewportRegs->set.wrmask;
	pNewport->txt_smask1x = pNewportRegs->cset.smask1x;
	pNewport->txt_smask1y = pNewportRegs->cset.smask1y;
	pNewport->txt_smask2x = pNewportRegs->cset.smask2x;
	pNewport->txt_smask2y = pNewportRegs->cset.smask2y;
	pNewport->txt_clipmode = pNewportRegs->cset.clipmode;
}

void
NewportRestoreRex3( ScrnInfoPtr pScrn)
{
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportRegsPtr pNewportRegs = NEWPORTREGSPTR(pScrn);

	pNewportRegs->set.drawmode0 = pNewport->txt_drawmode0;
	pNewportRegs->set.drawmode1 = pNewport->txt_drawmode1;
	pNewportRegs->set.wrmask = pNewport->txt_wrmask;
	pNewportRegs->cset.smask1x = pNewport->txt_smask1x;
	pNewportRegs->cset.smask1y = pNewport->txt_smask1y;
	pNewportRegs->cset.smask2x = pNewport->txt_smask2x;
	pNewportRegs->cset.smask2y = pNewport->txt_smask2y;
	pNewportRegs->cset.clipmode = pNewport->txt_clipmode;
}

void NewportBackupXmap9s( ScrnInfoPtr pScrn)
{
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportRegsPtr pNewportRegs = NEWPORTREGSPTR(pScrn);

	/* config of xmap0 */
	NewportBfwait(pNewport->pNewportRegs);
	pNewport->txt_xmap9_cfg0 = NewportXmap9ReadRegister(pNewportRegs,
	    DCB_XMAP0, XM9_CRS_CONFIG);

	/* config of xmap1 */
	NewportBfwait(pNewport->pNewportRegs);
	pNewport->txt_xmap9_cfg1 = NewportXmap9ReadRegister(pNewportRegs,
	    DCB_XMAP1, XM9_CRS_CONFIG);

	/* mode index register of xmap0 */
	NewportBfwait(pNewport->pNewportRegs);
	pNewport->txt_xmap9_mi = NewportXmap9ReadRegister(pNewportRegs,
	    DCB_XMAP0, XM9_CRS_MODE_REG_INDEX);

	/* mode register 0 of xmap 0 */
	pNewport->txt_xmap9_mod0 = NewportXmap9GetModeRegister(pNewportRegs, 0, 0);

	/* cursor cmap msb */
	NewportBfwait(pNewport->pNewportRegs);
	pNewport->txt_xmap9_ccmsb = NewportXmap9ReadRegister(pNewportRegs,
	    DCB_XMAP0, XM9_CRS_CURS_CMAP_MSB);
}

void NewportRestoreXmap9s( ScrnInfoPtr pScrn)
{
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportRegsPtr pNewportRegs = NEWPORTREGSPTR(pScrn);

	/* mode register 0 */
	NewportXmap9SetModeRegister( pNewport, 0, pNewport->txt_xmap9_mod0 );

	/* mode index register */
	NewportBfwait(pNewport->pNewportRegs);
	NewportXmap9WriteRegister(pNewportRegs, DCB_XMAP_ALL,
	    XM9_CRS_MODE_REG_INDEX, pNewport->txt_xmap9_mi);

	/* cfg xmap0 */
	NewportBfwait(pNewport->pNewportRegs);
	NewportXmap9WriteRegister(pNewportRegs, DCB_XMAP0, XM9_CRS_CONFIG,
	    pNewport->txt_xmap9_cfg0);

	/* cfg xmap1 */
	NewportBfwait(pNewport->pNewportRegs);
	NewportXmap9WriteRegister(pNewportRegs, DCB_XMAP1, XM9_CRS_CONFIG,
	    pNewport->txt_xmap9_cfg1);

	/* cursor cmap msb */
	NewportBfwait(pNewport->pNewportRegs);
	NewportXmap9WriteRegister(pNewportRegs, DCB_XMAP_ALL,
	    XM9_CRS_CURS_CMAP_MSB, pNewport->txt_xmap9_ccmsb);
}

