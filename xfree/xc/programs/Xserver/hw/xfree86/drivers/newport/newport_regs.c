/*
 * Id: newport_regs.c,v 1.3 2000/11/29 20:58:10 agx Exp $
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/newport/newport_regs.c,v 1.1 2000/12/01 19:48:01 dawes Exp $ */

#include "newport.h"

static void NewportXmap9FifoWait(NewportRegsPtr pNewportRegs, unsigned long xmapChip);

void 
NewportVc2Set(NewportRegsPtr pNewportRegs, unsigned char vc2Ireg, unsigned short val)
{
	pNewportRegs->set.dcbmode = (NPORT_DMODE_AVC2 | VC2_REGADDR_INDEX | NPORT_DMODE_W3 |
					NPORT_DMODE_ECINC | VC2_PROTOCOL);
	pNewportRegs->set.dcbdata0.all = (vc2Ireg << 24) | (val << 8);
}

unsigned short 
NewportVc2Get(NewportRegsPtr pNewportRegs, unsigned char vc2Ireg)
{
	pNewportRegs->set.dcbmode = (NPORT_DMODE_AVC2 | VC2_REGADDR_INDEX | NPORT_DMODE_W1 |
					NPORT_DMODE_ECINC | VC2_PROTOCOL);
	pNewportRegs->set.dcbdata0.bytes.b3 = vc2Ireg;
	pNewportRegs->set.dcbmode = (NPORT_DMODE_AVC2 | VC2_REGADDR_IREG | NPORT_DMODE_W2 |
					NPORT_DMODE_ECINC | VC2_PROTOCOL);
	return pNewportRegs->set.dcbdata0.hwords.s1;
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
		NewportBfwait( pNewportRegs);
		pNewportRegs->set.dcbmode = (xmapChip | R_DCB_XMAP9_PROTOCOL |
						XM9_CRS_FIFO_AVAIL | NPORT_DMODE_W1);
		if( (pNewportRegs->set.dcbdata0.bytes.b3) & 7 ) 
			break;
	}
}

void
NewportXmap9SetModeRegister(NewportRegsPtr pNewportRegs, CARD8 address, CARD32 mode)
{
	NewportXmap9FifoWait( pNewportRegs, DCB_XMAP0);
	NewportXmap9FifoWait( pNewportRegs, DCB_XMAP1);

	pNewportRegs->set.dcbmode = (DCB_XMAP_ALL | W_DCB_XMAP9_PROTOCOL |
			XM9_CRS_MODE_REG_DATA | NPORT_DMODE_W4 );
	pNewportRegs->set.dcbdata0.all = (address << 24) | ( mode & 0xffffff );
}
