/* 
 * Id: newport_shadow.c,v 1.3 2000/11/29 20:58:10 agx Exp $
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/newport/newport_shadow.c,v 1.1 2000/12/01 19:48:02 dawes Exp $ */

#include "newport.h"

void
NewportRefreshArea8(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
	unsigned long *base, *src;
	int dx, dy, x;
	NewportPtr pNewport = NEWPORTPTR(pScrn);
	NewportRegsPtr pNewportRegs = pNewport->pNewportRegs;

#define RA8_PIXELS	4	/* burst 4 pixels each time */
#define RA8_PIXEL_SHIFT 2 	/* 4 Pixels on each burst   */
#define RA8_MASK        0xffc   /* move to 4Byte boundary   */

	NewportWait(pNewportRegs);
	pNewportRegs->set.drawmode1 = pNewport->drawmode1 | (NPORT_DMODE1_RWPCKD | \
					(NPORT_DMODE1_HDMASK & NPORT_DMODE1_HD8));
	pNewportRegs->set.drawmode0 = (NPORT_DMODE0_DRAW | NPORT_DMODE0_BLOCK | \
						     NPORT_DMODE0_CHOST);
	while(num--) {
		NewportWait(pNewportRegs);
		x = pbox->x1 & RA8_MASK;  	/* move x to 4 byte boundary */
		base = (unsigned long*)pNewport->ShadowPtr 
				+ (pbox->y1 * (pNewport->ShadowPitch >> RA8_PIXEL_SHIFT) ) 
				+ ( x >> RA8_PIXEL_SHIFT);

		pNewportRegs->set.xystarti = (x << 16) | pbox->y1;
		pNewportRegs->set.xyendi = (pbox->x2 << 16) | pbox->y2;

		for ( dy = pbox->y1; dy <= pbox->y2; dy++) {

			src = base;
			for ( dx = x; dx <= pbox->x2; dx += RA8_PIXELS) {
				pNewportRegs->go.hostrw0 = *src;  
				src++;
			}
			base += ( pNewport->ShadowPitch >> RA8_PIXEL_SHIFT );
		}
		pbox++;
	}
}

