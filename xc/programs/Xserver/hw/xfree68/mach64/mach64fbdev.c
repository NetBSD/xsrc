/* $XFree86: xc/programs/Xserver/hw/xfree68/mach64/mach64fbdev.c,v 1.1.2.2 1999/05/25 12:00:31 hohndel Exp $ */


#include "mach64.h"
#ifdef PIXPRIV
#include "mach64im.h"
#endif

int mach64alu[16] = {
    MIX_0,
    MIX_AND,
    MIX_SRC_AND_NOT_DST,
    MIX_SRC,
    MIX_NOT_SRC_AND_DST,
    MIX_DST,
    MIX_XOR,
    MIX_OR,
    MIX_NOR,
    MIX_XNOR,
    MIX_NOT_DST,
    MIX_SRC_OR_NOT_DST,
    MIX_NOT_SRC,
    MIX_NOT_SRC_OR_DST,
    MIX_NAND,
    MIX_1
};

int mach64MaxX, mach64MaxY;
int mach64VirtX, mach64VirtY;
pointer mach64MemReg = NULL;
pointer mach64VideoMem = NULL;

extern pointer fbdevVirtBase, fbdevRegBase;

#ifdef PIXPRIV
int mach64PixmapIndex;
#endif

Bool mach64IntegratedController;

extern Bool mach64DestroyPixmap();

int mach64_reinit(ScreenPtr pScreen)
{
    WaitIdleEmpty();

    regw(DST_OFF_PITCH, (mach64VirtX >> 3) << 22);
    regw(SRC_OFF_PITCH, (mach64VirtX >> 3) << 22);

    regw(CONTEXT_MASK, 0xffffffff);

    regw(DST_Y_X, 0);
    regw(DST_HEIGHT, 0);
    regw(DST_BRES_ERR, 0);
    regw(DST_BRES_INC, 0);
    regw(DST_BRES_DEC, 0);
    regw(DST_CNTL, (DST_X_LEFT_TO_RIGHT | DST_Y_TOP_TO_BOTTOM));

    regw(SRC_Y_X, 0);
    regw(SRC_HEIGHT1_WIDTH1, 0);
    regw(SRC_Y_X_START, 0);
    regw(SRC_HEIGHT2_WIDTH2, 0);
    regw(SRC_CNTL, 0);

    WaitQueue(6);
    regw(HOST_CNTL, regr(HOST_CNTL) & ~HOST_BYTE_ALIGN);
    regw(PAT_REG0, 0);
    regw(PAT_REG1, 0);
    regw(PAT_CNTL, 0);

    regw(SC_LEFT_RIGHT, ((mach64MaxX << 16) | 0 ));
    regw(SC_TOP_BOTTOM, ((mach64MaxY << 16) | 0 ));

    WaitQueue(9);
    regw(DP_BKGD_CLR, 0);
    regw(DP_FRGD_CLR, 1);
    regw(DP_WRITE_MASK, 0xffffffff);
    regw(DP_MIX, (MIX_SRC << 16) | MIX_DST);
    regw(DP_SRC, FRGD_SRC_FRGD_CLR);

    regw(CLR_CMP_CLR, 0);
    regw(CLR_CMP_MASK, 0xffffffff);
    regw(CLR_CMP_CNTL, 0);

    regw(GUI_TRAJ_CNTL, DST_X_LEFT_TO_RIGHT | DST_Y_TOP_TO_BOTTOM);

    WaitQueue(2);
    switch (mach64InfoRec.depth) {
	case 8:
	    regw(DP_PIX_WIDTH, HOST_8BPP | SRC_8BPP | DST_8BPP | BYTE_ORDER_LSB_TO_MSB);
	    regw(DP_CHAIN_MASK, DP_CHAIN_8BPP);
	    break;
	case 15:
	    regw(DP_PIX_WIDTH, HOST_16BPP | SRC_15BPP | DST_15BPP | BYTE_ORDER_LSB_TO_MSB);
	    regw(DP_CHAIN_MASK, DP_CHAIN_15BPP);
	case 16:
	    regw(DP_PIX_WIDTH, HOST_16BPP | SRC_16BPP | DST_16BPP | BYTE_ORDER_LSB_TO_MSB);
	    regw(DP_CHAIN_MASK, DP_CHAIN_16BPP);
	    break;
	case 32:
	    regw(DP_PIX_WIDTH, HOST_32BPP | SRC_32BPP | DST_32BPP |BYTE_ORDER_LSB_TO_MSB);
	    regw(DP_CHAIN_MASK, DP_CHAIN_32BPP);
	    break;
    }

    WaitIdleEmpty();

    mach64CacheInit(mach64VirtX, mach64VirtY);
    mach64FontCache8Init(mach64VirtX, mach64VirtY);
}

int mach64_init(ScreenPtr pScreen)
{
    mach64InfoRec = fbdevInfoRec;
    mach64VideoMem = fbdevVirtBase;

    mach64VirtX = mach64MaxX = mach64InfoRec.virtualX;
    mach64VirtY = mach64MaxY = mach64InfoRec.virtualY;

    mach64_reinit(pScreen);

    mach64ImageInit();

    mach64InitGC();

#ifdef PIXPRIV
    mach64PixmapIndex = AllocatePixmapPrivateIndex();
    if (!AllocatePixmapPrivate(pScreen, mach64PixmapIndex,
			       sizeof(mach64PixPrivRec)))
	return FALSE;
#endif

    pScreen->PaintWindowBackground = mach64PaintWindow;
    pScreen->PaintWindowBorder = mach64PaintWindow;
    pScreen->CopyWindow = mach64CopyWindow;
    pScreen->RealizeFont = mach64RealizeFont;
    pScreen->UnrealizeFont = mach64UnrealizeFont;
    pScreen->CreateGC = mach64CreateGC;
    pScreen->GetImage = mach64GetImage;
    pScreen->DestroyPixmap = mach64DestroyPixmap;
    return TRUE;
}

int mach64_gx_init(ScreenPtr pScreen)
{
    mach64MemReg = fbdevRegBase;
    mach64IntegratedController = FALSE;
    return mach64_init(pScreen);
}

int mach64_ct_init(ScreenPtr pScreen)
{
    mach64MemReg = fbdevRegBase;
    mach64IntegratedController = TRUE;
    return mach64_init(pScreen);
}

int mach64_vt_init(ScreenPtr pScreen)
{
    mach64MemReg = fbdevRegBase+0x400;
    mach64IntegratedController = TRUE;
    return mach64_init(pScreen);
}

int mach64_gt_init(ScreenPtr pScreen)
{
    mach64MemReg = fbdevRegBase+0x400;
    mach64IntegratedController = TRUE;
    return mach64_init(pScreen);
}

