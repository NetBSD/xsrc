/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3bitblt.c,v 1.1.2.1 1999/07/30 11:21:29 hohndel Exp $ */

void SavageScreenToScreenBitBlt
(
    int nbox,
    DDXPointPtr pptSrc,
    BoxPtr pbox,
    int xdir, int ydir,
    int alu,
    unsigned planemask
)
{
    int w, h;
    setup();
    for (; nbox; pbox++, pptSrc++, nbox--)
    {
        w = pbox->x2 - pbox->x1;
    }
}
