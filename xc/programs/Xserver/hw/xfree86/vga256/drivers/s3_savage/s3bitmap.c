/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_savage/s3bitmap.c,v 1.1.2.1 1999/07/30 11:21:30 hohndel Exp $ */

void SavageWriteBitmapScreenToScreenColorExpand
(
    int x, int y, int w, int h,
    unsigned char * src,
    int srcwidth,
    int srcx, int srcy,
    int bg, int fg,
    int rop,
    unsigned int planemask
)
{
    BCI_GET_PTR;
    unsigned int cmd;
    unsigned int bd_offset;
    unsigned int bd;
    
    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_SEND_COLOR
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_SBD_MONO_NEW;
    cmd |= (bg != -1) ? BCI_CMD_SEND_COLOR : BCI_CMD_SRC_TRANSPARENT;
    cmd |= s3vAlu[rop];

    bd |= BCI_BD_BW_DISABLE;
    BCI_BD_SET_BPP(bd, 1);
    BCI_BD_SET_STRIDE(bd, srcwidth);
    bd_offset = srcwidth * srcy + (srcx >> 3) + (int) src;

    WaitQueue(10);
    BCI_SEND(cmd);
    BCI_SEND(bd_offset);
    BCI_SEND(bd);
    BCI_SEND(fg);
    BCI_SEND((bg != -1) ? bg : 0);
    BCI_SEND(BCI_X_Y(srcx, srcy));
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
}

void SavageWriteBitmapCPUToScreenColorExpand
(
    int x, int y, int w, int h,
    unsigned char * src,
    int srcwidth,
    int srcx, int srcy,
    int bg, int fg,
    int rop,
    unsigned int planemask
)
{
    BCI_GET_PTR;
    int i, j, count, shift;
    unsigned int cmd;
    unsigned int * srcp;

/* We aren't using planemask at all here... */

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_SEND_COLOR | BCI_CMD_CLIP_LR
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_MONO;
    cmd |= s3vAlu[rop];

    count = (w + 31) / 32;
    BCI_SEND(cmd);
    BCI_SEND(BCI_CLIP_LR(x, x+w-1));
    BCI_SEND(fg);
    BCI_SEND(bg);

    /* Bitmaps come in in units of DWORDS, LSBFirst.  This is exactly */
    /* reversed of what we expect.  */

    x -= srcx & 31;
    w += srcx & 31;
    count = (w + 31) / 32;
    src += (srcy * srcwidth) + ((srcx & ~31) / 8);
    
    for (j = 0; j < h; j ++) {
        BCI_SEND(BCI_X_Y(x, y+j));
	BCI_SEND(BCI_W_H(w, 1));
	srcp = (unsigned int*) src;
	for (i = count; i > 0; srcp ++, i --) {
#if 1
	    /* We have to invert the bits in each dword. */
	    unsigned long u = *srcp;
	    u = ((u & 0x0000ffff) << 16) | ((u & 0xffff0000) >> 16);
	    u = ((u & 0x00ff00ff) << 8) | ((u & 0xff00ff00) >> 8);
	    u = ((u & 0x0f0f0f0f) << 4) | ((u & 0xf0f0f0f0) >> 4);
	    u = ((u & 0x33333333) << 2) | ((u & 0xcccccccc) >> 2);
	    u = ((u & 0x55555555) << 1) | ((u & 0xaaaaaaaa) >> 1);
	    BCI_SEND(u);
#else
	    BCI_SEND(*srcp);
#endif
	}
	src += srcwidth;
    }
}

void SavageImageWrite
(
    int x, int y, int w, int h,
    void * src,
    int srcwidth,
    int rop,
    unsigned int planemask
)
{
    BCI_GET_PTR;
    int i, j, count, shift;
    unsigned int cmd, t;
    unsigned int * srcp = src;

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_CLIP_LR
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_COLOR;
    cmd |= s3vAlu[rop];

    count = ((w * vgaBitsPerPixel + 31) / 32) * h;
    BCI_SEND(cmd);
    BCI_SEND(BCI_CLIP_LR(x, x+w));
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
    srcp = src;
    if (vgaBitsPerPixel == 32) {
      /* Color is 0xffffff00 on Intel */
	for (i = count; i > 0; srcp ++, i --) {
	    BCI_SEND(*srcp << 8);
	}
    }
    else {
	for (i = count; i > 0; srcp ++, i --) {
	    BCI_SEND(*srcp);
	}
    }
}
