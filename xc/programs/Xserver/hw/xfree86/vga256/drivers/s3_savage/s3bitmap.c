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
    unsigned char * bd_offset;
    unsigned int bd;
    
    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_SEND_COLOR
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_SBD_MONO_NEW;
    cmd |= (bg != -1) ? BCI_CMD_SEND_COLOR : BCI_CMD_SRC_TRANSPARENT;
    cmd |= s3vAlu[rop];

    bd |= BCI_BD_BW_DISABLE;
    BCI_BD_SET_BPP(bd, 1);
    BCI_BD_SET_STRIDE(bd, srcwidth);
    bd_offset = srcwidth * srcy + (srcx >> 3) + src;

    WaitQueue(10);
    BCI_SEND(cmd);
    BCI_SEND((unsigned int)bd_offset);
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
    int i, j, count, shift, reset;
    unsigned int cmd;
    unsigned int * srcp;

/* We aren't using planemask at all here... */

    cmd = BCI_CMD_RECT | BCI_CMD_RECT_XP | BCI_CMD_RECT_YP
        | BCI_CMD_SEND_COLOR | BCI_CMD_CLIP_LR
        | BCI_CMD_DEST_GBD | BCI_CMD_SRC_MONO;
    cmd |= s3vAlu[rop];

    if( !srcwidth )
	return;

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

    /* The BCI region is 128k bytes.  A screen-sized mono bitmap can */
    /* exceed that. */

    reset = 65536 / srcwidth;
    
    for (j = 0; j < h; j ++) {
        BCI_SEND(BCI_X_Y(x, y+j));
        BCI_SEND(BCI_W_H(w, 1));
        srcp = (unsigned int*) src;
        for (i = count; i > 0; srcp ++, i --) {
            /* We have to invert the bits in each byte. */
            unsigned long u = *srcp;
            u = ((u & 0x0f0f0f0f) << 4) | ((u & 0xf0f0f0f0) >> 4);
            u = ((u & 0x33333333) << 2) | ((u & 0xcccccccc) >> 2);
            u = ((u & 0x55555555) << 1) | ((u & 0xaaaaaaaa) >> 1);
            BCI_SEND(u);
        }
        src += srcwidth;
        if( !--reset ) {
	    BCI_RESET;
            reset = 65536 / srcwidth;
        }
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
    WaitQueue( count );
    BCI_SEND(cmd);
    BCI_SEND(BCI_CLIP_LR(x, x+w));
    BCI_SEND(BCI_X_Y(x, y));
    BCI_SEND(BCI_W_H(w, h));
    srcp = src;

    /* 
      The BCI region is 128k bytes.  We've used 16 bytes so far.
      We copy the rest in big chunks.
    */

#define CHUNKSIZE	(98304/4)

    while( count > CHUNKSIZE )
    {
        memcpy( bci_ptr, srcp, CHUNKSIZE*4 );
        count -= CHUNKSIZE;
        srcp += CHUNKSIZE;
    }

    for (i = count; i > 0; srcp ++, i --) {
        BCI_SEND(*srcp);
    }
}
