/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/et4000/tseng_colexp.c,v 1.1.2.3 1998/02/20 15:13:56 robin Exp $ */
/*
 * ET4/6K acceleration interface -- color expansion primitives.
 *
 * Uses Harm Hanemaayer's generic acceleration interface (XAA).
 *
 * Author: Koen Gadeyne
 *
 * Much of the acceleration code is based on the XF86_W32 server code from
 * Glenn Lai.
 *
 *
 *     Color expansion capabilities of the Tseng chip families:
 *
 *     Chip     screen-to-screen   CPU-to-screen   Supported depths
 *
 *   ET4000W32/W32i   No               Yes             8bpp only
 *   ET4000W32p       Yes              Yes             8bpp only
 *   ET6000           Yes              No              8/16/24/32 bpp
 */

#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "tseng.h"
#include "tseng_acl.h"
#include "compiler.h"

#include "xf86xaa.h"

#include "tseng_colexp.h"
#include "tseng_inline.h"

/*
 * This is just for the messages.
 */
#include "xf86_Config.h"

/* for color expansion via scanline buffer in system memory */
#define COLEXP_BUF_SIZE 1024
static CARD32 colexp_buf[COLEXP_BUF_SIZE / 4];

static void TsengSubsequentScanlineCPUToScreenColorExpand_1to2to16 ();
static void TsengSubsequentScanlineCPUToScreenColorExpand_1to4to32 ();


void 
TsengAccelInit_Colexp ()
{

  if (OFLG_ISSET(OPTION_XAA_NO_COL_EXP, &vga256InfoRec.options)) return; 
  if (et4000_type < TYPE_ET4000W32P) return;    /* disable accelerated color expansion for W32/W32i until it's fixed */
  if (Is_W32p) return; /* disable for w32p as well, bug in drawing small pcs */

  /*
   * Screen-to-screen color expansion.
   *
   * Scanline-screen-to-screen color expansion is slower than
   * CPU-to-screen color expansion.
   */

  xf86AccelInfoRec.ColorExpandFlags =
    BIT_ORDER_IN_BYTE_LSBFIRST |
    VIDEO_SOURCE_GRANULARITY_PIXEL |
    NO_PLANEMASK;

  if (!Is_ET6K && (vgaBitsPerPixel != 24))
    {
      /* fast 8bpp-only XAA replacements for text drawing and Bitmap writing */
      if ((vgaBitsPerPixel == 8) && (et4000_type > TYPE_ET4000W32Pa))
	{
	  xf86AccelInfoRec.WriteBitmap = W32WriteBitmap;
	  xf86AccelInfoRec.ImageTextTE = W32ImageTextTECPUToScreenColorExpand;
	  xf86AccelInfoRec.PolyTextTE = W32PolyTextTECPUToScreenColorExpand;
	  xf86AccelInfoRec.FillRectOpaqueStippled = TsengScanlineCPUToScreenFillStippledRect;
	  xf86AccelInfoRec.FillRectStippled = TsengScanlineCPUToScreenFillStippledRect;
	  ErrorF ("%s %s: XAA/Tseng: Using ET4000W32-specific Color-expansion (WriteBitmap, Image/PolyTextTE, FillRect(Opaque)Stippled).\n",
		  XCONFIG_PROBED, vga256InfoRec.name);
	}

      /*
       * We'll use an intermediate memory buffer and fake
       * scanline-screen-to-screen color expansion, because the XAA
       * CPU-to-screen color expansion causes the accelerator to hang.
       * Reason unkown (yet). This also allows us to do 16 and 32 bpp color
       * expansion by first doubling the bitmap pattern before
       * color-expanding it, because W32s can only do 8bpp color expansion.
       *
       * XAA doesn't support scanline-CPU-to-SCreen color expansion yet.
       */

#if 0
      if (vgaBitsPerPixel == 24)
	xf86AccelInfoRec.ColorExpandFlags |= TRIPLE_BITS_24BPP;
#endif

      xf86AccelInfoRec.PingPongBuffers = 1;
      xf86AccelInfoRec.ScratchBufferSize = COLEXP_BUF_SIZE / tseng_bytesperpixel;
      xf86AccelInfoRec.ScratchBufferAddr = 1;	/* any non-zero value will do -- not used */
      xf86AccelInfoRec.ScratchBufferBase = (void *) colexp_buf;

      xf86AccelInfoRec.SetupForScanlineScreenToScreenColorExpand =
	TsengSetupForScanlineCPUToScreenColorExpand;

      xf86AccelInfoRec.SubsequentScanlineScreenToScreenColorExpand =
	TsengSubsequentScanlineCPUToScreenColorExpand;
    }

  if (Is_ET6K || (Is_W32p && (vgaBitsPerPixel == 8)))
    {
      xf86AccelInfoRec.SetupForScreenToScreenColorExpand =
	TsengSetupForScreenToScreenColorExpand;
      xf86AccelInfoRec.SubsequentScreenToScreenColorExpand =
	TsengSubsequentScreenToScreenColorExpand;
    }

  if (Is_ET6K)
    {
      if (tsengImageWriteBase)	/* uses the same buffer memory as ImageWrite */
	{
	  xf86AccelInfoRec.WriteBitmap = ET6KWriteBitmap;
	  xf86AccelInfoRec.FillRectOpaqueStippled = TsengScanlineScreenToScreenFillStippledRect;
	  xf86AccelInfoRec.FillRectStippled = TsengScanlineScreenToScreenFillStippledRect;
	  ErrorF ("%s %s: XAA/Tseng: Using ET6000-specific Color-expansion (WriteBitmap, FillRect(Opaque)Stippled).\n",
		  XCONFIG_PROBED, vga256InfoRec.name);
	}

      xf86AccelInfoRec.SetupForScanlineScreenToScreenColorExpand =
	TsengSetupForScanlineScreenToScreenColorExpand;

      xf86AccelInfoRec.SubsequentScanlineScreenToScreenColorExpand =
	TsengSubsequentScanlineScreenToScreenColorExpand;

      /* triple-buffering is needed to account for double-buffering of Tseng
       * acceleration registers. Increasing this number doesn't help solve the
       * problems with both ET4000W32 and ET6000 with text rendering.
       */
      xf86AccelInfoRec.PingPongBuffers = 3;

      xf86AccelInfoRec.ScratchBufferSize = tsengScratchVidBase + 1024 - (long) W32Mix;
      xf86AccelInfoRec.ScratchBufferAddr = W32Mix;

      if (!ET4000.ChipUseLinearAddressing)
	{
	  /* in banked mode, use aperture #0 */
	  xf86AccelInfoRec.ScratchBufferBase =
	    (unsigned char *)
	    (((int) vgaBase) + 0x18000L + 1024 - xf86AccelInfoRec.ScratchBufferSize);
	}
    }
#if 0
  ErrorF ("ColorExpand ScratchBuf: Addr = %d (0x%x); Size = %d (0x%x); Base = %d (0x%x)\n",
     xf86AccelInfoRec.ScratchBufferAddr, xf86AccelInfoRec.ScratchBufferAddr,
     xf86AccelInfoRec.ScratchBufferSize, xf86AccelInfoRec.ScratchBufferSize,
    xf86AccelInfoRec.ScratchBufferBase, xf86AccelInfoRec.ScratchBufferBase);
#endif

#if TSENG_CPU_TO_SCREEN_COLOREXPAND
  /*
   * CPU-to-screen color expansion doesn't seem to be reliable yet. The
   * W32 needs the correct amount of data sent to it in this mode, or it
   * hangs the machine until is does (?). Currently, the init code in this
   * file or the XAA code that uses this does something wrong, so that
   * occasionally we get accelerator timeouts, and after a few, complete
   * system hangs.
   *
   * The W32 engine requires SCANLINE_NO_PAD, but that doesn't seem to
   * work very well (accelerator hangs).
   *
   * What works is this: tell XAA that we have SCANLINE_PAD_DWORD, and then
   * add the following code in TsengSubsequentCPUToScreenColorExpand():
   *     w = (w + 31) & ~31; this code rounds the width up to the nearest
   * multiple of 32, and together with SCANLINE_PAD_DWORD, this makes
   * CPU-to-screen color expansion work. Of course, the display isn't
   * correct (4 chars are "blanked out" when only one is written, for
   * example). But this shows that the principle works. But the code
   * doesn't...
   *
   * The same thing goes for PAD_BYTE: this also works (with the same
   * problems as SCANLINE_PAD_DWORD, although less prominent)
   */
  if (Is_W32_any && (vgaBitsPerPixel == 8))
    {
      /*
       * CPU_TRANSFER_PAD_DWORD is implied by XAA, and I'm not sure this is
       * OK, because the W32 might be trying to expand the padding data.
       */
      xf86AccelInfoRec.ColorExpandFlags |=
	SCANLINE_NO_PAD | CPU_TRANSFER_PAD_DWORD;

      xf86AccelInfoRec.SetupForCPUToScreenColorExpand =
	TsengSetupForCPUToScreenColorExpand;
      xf86AccelInfoRec.SubsequentCPUToScreenColorExpand =
	TsengSubsequentCPUToScreenColorExpand;

      /* we'll be using MMU aperture 2 */
      xf86AccelInfoRec.CPUToScreenColorExpandBase = tsengCPU2ACLBase;
      /* ErrorF("tsengCPU2ACLBase = 0x%x\n", tsengCPU2ACLBase); */
      /* aperture size is 8kb in banked mode. Larger in linear mode, but 8kb is enough */
      xf86AccelInfoRec.CPUToScreenColorExpandRange = 8192;
    }
#endif
}


#define SET_FUNCTION_COLOREXPAND \
    if (Is_ET6K) \
      *ACL_MIX_CONTROL     = 0x32; \
    else \
      *ACL_ROUTING_CONTROL = 0x08;

#define SET_FUNCTION_COLOREXPAND_CPU \
    *ACL_ROUTING_CONTROL = 0x02;



static int ColorExpandDst;
static int colexp_width;
static int colexp_slot = 0;	/* slot offset in ping-pong buffers */


void 
TsengSetupForScanlineScreenToScreenColorExpand (x, y, w, h, bg, fg, rop, planemask)
     int x, y;
     int w, h;
     int bg, fg;
     int rop;
     unsigned int planemask;
{
  colexp_width = w;		/* only needed for 1-to-2-to-16 color expansion */

  ColorExpandDst = FBADDR (x, y);

  TsengSetupForScreenToScreenColorExpand (bg, fg, rop, planemask);

  *ACL_MIX_Y_OFFSET = 0x0FFF;	/* see remark below */

  SET_XY (w, 1);
}

void 
TsengSubsequentScanlineScreenToScreenColorExpand (srcaddr)
     int srcaddr;
{
  /* COP_FRAMEBUFFER_CONCURRENCY can cause text corruption !!!

   * Looks like some scanline data DWORDS are not written to the ping-pong
   * framebuffers, so that old data is rendered in some places. Is this
   * caused by PCI host bridge queueing? Or is data lost when written
   * while the accelerator is accessing the framebuffer (which would be
   * the real reason NOT to use COP_FRAMEBUFFER_CONCURRENCY)?
   *
   * Even with ping-ponging, parts of scanline three (which are supposed
   * to be written to the old, already rendered scanline 1 buffer) have
   * not yet arrived in the framebuffer, and thus some parts of the new
   * scanline are rendered with data from two lines above it.
   *
   * Extra problem: ET6000 queueing really needs triple buffering for
   * this, because XAA can still overwrite scanline 1 when writing data
   * for scanline three. Right _after_ that, the accelerator blocks on
   * queueing in TsengSubsequentScanlineScreenToScreenColorExpand(), but
   * then it's too late: the scanline data is already overwritten. That's
   * why we use 3 ping-pong buffers.
   *
   * "x11perf -fitext" is about 530k chars/sec now, but with
   * COP_FRAMEBUFFER_CONCURRENCY, this goes up to >700k (which is similar
   * to what Xinside can do).
   *
   * Needs to be investigated!
   *
   * Update: this seems to depend upon ACL_MIX_Y_OFFSET, although that
   * register should not do anything at all here (only one line done at a
   * time, so no Y_OFFSET needed). Setting the offset to 0x0FFF seems to
   * remedy this situation most of the time (still an occasional error
   * here and there). This _could_ be a bug, but then it would have to be
   * in both in the ET6000 _and_ the ET4000W32p.
   *
   * The more delay added after starting a color-expansion operation, the
   * less font corruption we get. But nothing really solves it.
   */

  wait_acl_queue ();

  *ACL_MIX_ADDRESS = srcaddr;
  START_ACL (ColorExpandDst);

  /* move to next scanline */
  ColorExpandDst += tseng_line_width;

  /*
   * If not using triple-buffering, we need to wait for the queued
   * register set to be transferred to the working register set here,
   * because otherwise an e.g. double-buffering mechanism could overwrite
   * the buffer that's currently being worked with with new data too soon.
   *
   * WAIT_QUEUE; // not needed with triple-buffering
   */
}


/*
 * We use this intermediate CPU-to-Screen color expansion because the one
 * provided by XAA seems to lock up the accelerator engine.
 */

static int colexp_width_dwords;

void 
TsengSetupForScanlineCPUToScreenColorExpand (x, y, w, h, bg, fg, rop, planemask)
     int x, y;
     int w, h;
     int bg, fg;
     int rop;
     unsigned int planemask;
{
  /* the accelerator needs DWORD padding, and "w" is in PIXELS... */
  colexp_width_dwords = (MULBPP (w) + 31) >> 5;
  /* ErrorF("w=%d;d=%d ", w, colexp_width_dwords); */

  ColorExpandDst = FBADDR (x, y);

  TsengSetupForCPUToScreenColorExpand (bg, fg, rop, planemask);

  /* *ACL_MIX_Y_OFFSET = w-1; */

  SET_XY (w, 1);
}

void 
TsengSubsequentScanlineCPUToScreenColorExpand (srcaddr)
     int srcaddr;
{
  int i;
  CARD8* src=(CARD8*)colexp_buf;
  CARD8* dst=(CARD8*)tsengCPU2ACLBase;

  wait_acl_queue ();

  START_ACL (ColorExpandDst);

  switch (vgaBitsPerPixel)
    {
    case 8:
    case 24:			/* assumes TRIPLE_BITS_24BPP */
      /* Copy scanline data to accelerator MMU aperture */
      if (et4000_type < TYPE_ET4000W32P)
        for (i=0; i<(colexp_width_dwords*4); i++)
          *dst++ = *src++;
      else
        MoveDWORDS (tsengCPU2ACLBase, colexp_buf, colexp_width_dwords);
      break;
    case 16:
      /* expand the color expand data to 2 bits per pixel before copying it to the MMU aperture */
      TsengSubsequentScanlineCPUToScreenColorExpand_1to2to16 (colexp_buf);
      break;
    case 32:
      TsengSubsequentScanlineCPUToScreenColorExpand_1to4to32 (colexp_buf);
      break;
    }

  /* move to next scanline */
  ColorExpandDst += tseng_line_width;
}

/*
 * This function does direct memory-to-CPU bit doubling for color-expansion
 * at 16bpp on W32 chips. They can only do 8bpp color expansion, so we have
 * to expand the incoming data to 2bpp first.
 */

static void 
TsengSubsequentScanlineCPUToScreenColorExpand_1to2to16 (src)
     CARD32 *src;
{
  CARD32 *dest = (CARD32 *) tsengCPU2ACLBase;
  int i;
  CARD16 ind, *bufptr;
  CARD32 r;

  i = colexp_width_dwords;	/* amount of blocks of 16 bits to expand to 32 bits (=1 DWORD) */
  bufptr = (CARD16 *) (src);

  while (i--)
    {
      r = 0;
      ind = *bufptr++;

      if (ind & 0x0001)	r |= 0x00000003;
      if (ind & 0x0002)	r |= 0x0000000C;
      if (ind & 0x0004)	r |= 0x00000030;
      if (ind & 0x0008)	r |= 0x000000C0;
      if (ind & 0x0010)	r |= 0x00000300;
      if (ind & 0x0020)	r |= 0x00000C00;
      if (ind & 0x0040)	r |= 0x00003000;
      if (ind & 0x0080)	r |= 0x0000C000;

      if (ind & 0x0100)	r |= 0x00030000;
      if (ind & 0x0200)	r |= 0x000C0000;
      if (ind & 0x0400)	r |= 0x00300000;
      if (ind & 0x0800)	r |= 0x00C00000;
      if (ind & 0x1000)	r |= 0x03000000;
      if (ind & 0x2000)	r |= 0x0C000000;
      if (ind & 0x4000)	r |= 0x30000000;
      if (ind & 0x8000)	r |= 0xC0000000;

      *dest++ = r;
    }
}

/*
 * This function does direct memory-to-CPU bit doubling for color-expansion
 * at 32bpp on W32 chips. They can only do 8bpp color expansion, so we have
 * to expand the incoming data to 4bpp first.
 */

static void 
TsengSubsequentScanlineCPUToScreenColorExpand_1to4to32 (src)
     CARD32 *src;
{
  CARD32 *dest = (CARD32 *) tsengCPU2ACLBase;
  int i;
  CARD8 ind, *bufptr;
  CARD32 r;

  i = colexp_width_dwords;	/* amount of blocks of 8 bits to expand to 32 bits (=1 DWORD) */
  bufptr = (CARD8 *) (src);

  while (i--)
    {
      r = 0;
      ind = *bufptr++;

      if (ind & 0x0001)	r |= 0x0000000F;
      if (ind & 0x0002)	r |= 0x000000F0;
      if (ind & 0x0004)	r |= 0x00000F00;
      if (ind & 0x0008)	r |= 0x0000F000;
      if (ind & 0x0010)	r |= 0x000F0000;
      if (ind & 0x0020)	r |= 0x00F00000;
      if (ind & 0x0040)	r |= 0x0F000000;
      if (ind & 0x0080)	r |= 0xF0000000;

      *dest++ = r;
    }
}

/*
 * CPU-to-Screen color expansion.
 *   This is for ET4000 only (The ET6000 cannot do this)
 */

void 
TsengSetupForCPUToScreenColorExpand (bg, fg, rop, planemask)
     int bg, fg;
     int rop;
     unsigned int planemask;
{
/*  ErrorF("X"); */

  PINGPONG ();

  wait_acl_queue ();

  SET_FG_ROP (rop);
  SET_BG_ROP_TR (rop, bg);

  SET_XYDIR (0);

  SET_FG_BG_COLOR (fg, bg);

  SET_FUNCTION_COLOREXPAND_CPU;

  /* assure correct alignment of MIX address (ACL needs same alignment here as in MMU aperture) */
  *ACL_MIX_ADDRESS = 0;
}


/*
 * TsengSubsequentCPUToScreenColorExpand() is potentially dangerous:
 *   Not writing enough data to the MMU aperture for CPU-to-screen color
 *   expansion will eventually cause a system deadlock!
 *
 * Note that CPUToScreenColorExpand operations _always_ require a
 * WAIT_INTERFACE before starting a new operation (this is empyrical,
 * though)
 */

void 
TsengSubsequentCPUToScreenColorExpand (x, y, w, h, skipleft)
     int x, y;
     int w, h;
     int skipleft;
{
  int destaddr = FBADDR (x, y);

  /* ErrorF(" %dx%d|%d ",w,h,skipleft); */
  if (skipleft)
    ErrorF ("Can't do: Skipleft = %d\n", skipleft);

/*  wait_acl_queue(); */
  WAIT_ACL;

  *ACL_MIX_Y_OFFSET = w - 1;
  SET_XY (w, h);
  START_ACL (destaddr);
}

void 
TsengSetupForScreenToScreenColorExpand (bg, fg, rop, planemask)
     int bg, fg;
     int rop;
     unsigned int planemask;
{
/*  ErrorF("SSC "); */

  PINGPONG ();

  wait_acl_queue ();

  SET_FG_ROP (rop);
  SET_BG_ROP_TR (rop, bg);

  SET_FG_BG_COLOR (fg, bg);

  SET_FUNCTION_COLOREXPAND;

  SET_XYDIR (0);
}

void 
TsengSubsequentScreenToScreenColorExpand (srcx, srcy, x, y, w, h)
     int srcx, srcy;
     int x, y;
     int w, h;
{
  int destaddr = FBADDR (x, y);

  int mixaddr = FBADDR (srcx, srcy * 8);

  wait_acl_queue ();

  SET_XY (w, h);
  *ACL_MIX_ADDRESS = mixaddr;
  *ACL_MIX_Y_OFFSET = w - 1;

  START_ACL (destaddr);
}

/*
 * Below are XAA replacements for some commonly used functions. Some are
 * there because XAA doesn't support the Tseng hardware, some are there
 * because we think we can do it faster than XAA...
 */

/*
 * WriteBitmap() code. Largely written by Mark Vojkovich.
 *
 * This should really use triple-buffering as does the standard scanline
 * color expansion XAA code. Now the excessive Syncing causes it to drag a
 * little. Still, it beats drawing bitmaps through scanline-color expansion: 
 *   copyplane500 : 493     (up from 365)
 *   copyplane100 : 4440    (up from 2880)
 *   copyplane10  : 31900   (up from 24600)
 *
 * The main advantage here is that alignment problems are handled by the
 * accelerator instead of the XAA code (= the CPU).
 */

void 
ET6KWriteBitmap (x, y, w, h, src, srcwidth, srcx, srcy,
		 bg, fg, rop, planemask)
     int x, y, w, h;
     unsigned char *src;
     int srcwidth;
     int srcx, srcy;
     int bg, fg;
     int rop;
     unsigned int planemask;
{
  unsigned char *srcp;		/* pointer to src */
  Bool PlusOne = (h & 0x01);
  int dwords;

  TsengSetupForScanlineScreenToScreenColorExpand (x, y, w, h, bg, fg, rop, planemask);

  h >>= 1;			/* h now represents line pairs */

  /* calculate the src pointer to the nearest dword boundary */
  srcp = (srcwidth * srcy) + (srcx >> 5) + src;
  srcx &= 31;			/* srcx now contains the skipleft parameter */

  dwords = (w + 31 + srcx) >> 5;

  while (h--)
    {
      /* WAIT_ACL; */
      /* write the first line */
      MoveDWORDS (tsengFirstLinePntr, (CARD32 *) srcp, dwords);
      /* blit it */
      WAIT_QUEUE;
      TsengSubsequentScanlineScreenToScreenColorExpand ((tsengFirstLine << 3) + srcx);
      srcp += srcwidth;
      /* write the second line */
      MoveDWORDS (tsengSecondLinePntr, (CARD32 *) srcp, dwords);
      /* blit it */
      WAIT_QUEUE;
      TsengSubsequentScanlineScreenToScreenColorExpand ((tsengSecondLine << 3) + srcx);
      srcp += srcwidth;
    }

  if (PlusOne)
    {
      /* WAIT_ACL; */
      /* write the first line */
      MoveDWORDS (tsengFirstLinePntr, (CARD32 *) srcp, dwords);
      /* blit it */
      WAIT_QUEUE;
      TsengSubsequentScanlineScreenToScreenColorExpand ((tsengFirstLine << 3) + srcx);
    }

  xf86AccelInfoRec.Sync();
}



/* when defined, use faster (but less generic) ACL CPU-to-screen Subsequent setup code */
#define USE_FAST_ACLINIT 1


void 
W32WriteBitmap (x, y, w, h, src, srcwidth, srcx,
		srcy, bg, fg, rop, planemask)
     int x, y, w, h;
     unsigned char *src;
     int srcwidth;
     int srcx, srcy;
     int bg, fg;
     int rop;
     unsigned int planemask;
{
  unsigned char *srcp = (srcwidth * srcy) + (srcx >> 3) + src;
  int dwords = (w + 31) >> 5;
  register int shift = srcx & 0x07;
  int destaddr;

  TsengSetupForCPUToScreenColorExpand (bg, fg, rop, planemask);

#ifdef USE_FAST_ACLINIT
  /*
   * this is a replacement for the TsengSubsequentCPUToScreenColorExpand()
   * function, but optimized for this application.
   */
  destaddr = FBADDR (x, y);

  SET_XY (w, 1);
#endif

  if (shift)
    {
      int count;
      register CARD32 *destptr;
      register CARD32 *srcptr;
      while (h--)
	{
	  count = dwords;
	  srcptr = (CARD32 *) srcp;
	  destptr = (CARD32 *) tsengCPU2ACLBase;
#ifdef USE_FAST_ACLINIT
	  WAIT_ACL;
	  START_ACL (destaddr);
	  destaddr += tseng_line_width;
#else
	  TsengSubsequentCPUToScreenColorExpand (x, y++, w, 1, 0);
#endif
	  while (count--)
	    {
	      *(destptr++) = (srcptr[0] >> shift) |
		(srcptr[1] << (32 - shift));
	      srcptr++;
	    }
	  srcp += srcwidth;
	}
    }
  else
    {
      while (h--)
	{
#ifdef USE_FAST_ACLINIT
	  WAIT_ACL;
	  START_ACL (destaddr);
	  destaddr += tseng_line_width;
#else
	  TsengSubsequentCPUToScreenColorExpand (x, y++, w, 1, 0);
#endif
	  MoveDWORDS (tsengCPU2ACLBase, (CARD32 *) srcp, dwords);
	  srcp += srcwidth;
	}
    }

  xf86AccelInfoRec.Sync();
}



/*
 * Fast W32 8bpp TE ImageText and PolyText rendering code, using
 * CPU-to-screen color expansion. This draws one scanline at a time.
 * Author: Mark Vojkovich.
 */


#include "xf86expblt.h"

#define MAX_GLYPHS	1024	/* that's gotta be enough */
static unsigned int *Glyphs[MAX_GLYPHS];

void
W32ImageTextTECPUToScreenColorExpand (pDrawable, pGC, xInit, yInit,
				      nglyph, ppci, pglyphBase)
     DrawablePtr pDrawable;
     GC *pGC;
     int xInit, yInit;
     int nglyph;
     CharInfoPtr *ppci;		/* array of character info */
     unsigned char *pglyphBase;	/* start of array of glyphs */
{
  int w, h, x, y;
  int glyphWidth, count;
  int destaddr;

  glyphWidth = FONTMAXBOUNDS (pGC->font, characterWidth);

  /*
   * Check for non-standard glyphs, glyphs that are too wide.
   */
  if ((GLYPHWIDTHBYTESPADDED (*ppci) != 4) || glyphWidth > 32)
    {
      xf86GCInfoRec.ImageGlyphBltFallBack (
		    pDrawable, pGC, xInit, yInit, nglyph, ppci, pglyphBase);
      return;
    }

  h = FONTASCENT (pGC->font) + FONTDESCENT (pGC->font);
  if (!(h && glyphWidth))
    return;

  TsengSetupForCPUToScreenColorExpand (
			pGC->bgPixel, pGC->fgPixel, GXcopy, pGC->planemask);

  x = xInit + FONTMAXBOUNDS (pGC->font, leftSideBearing) + pDrawable->x;
  y = yInit - FONTASCENT (pGC->font) + pDrawable->y;
  w = nglyph * glyphWidth;

#ifdef USE_FAST_ACLINIT
  /*
   * this is a replacement for the TsengSubsequentCPUToScreenColorExpand()
   * function, but optimized for this application.
   */
  destaddr = FBADDR (x, y);

  SET_XY (w, 1);
#endif

  for (count = 0; count < nglyph; count++)
    Glyphs[count] = (unsigned int *) FONTGLYPHBITS (pglyphBase, *ppci++);


  for (count = 0; count < h; count++, y++)
    {
#ifdef USE_FAST_ACLINIT
      WAIT_ACL;
      START_ACL (destaddr);
      destaddr += tseng_line_width;
#else
      TsengSubsequentCPUToScreenColorExpand (x, y, w, 1, 0);
#endif
      xf86DrawTextScanline ((unsigned int *) tsengCPU2ACLBase,
			    Glyphs, count, nglyph, glyphWidth);
    }

  xf86AccelInfoRec.Sync();
}

void
W32PolyTextTECPUToScreenColorExpand (pDrawable, pGC, xInit, yInit,
				     nglyph, ppci, pglyphBase)
     DrawablePtr pDrawable;
     GC *pGC;
     int xInit, yInit;
     int nglyph;
     CharInfoPtr *ppci;		/* array of character info */
     unsigned char *pglyphBase;	/* start of array of glyphs */
{
  int w, h, x, y;
  int glyphWidth, count;
  int destaddr;

  glyphWidth = FONTMAXBOUNDS (pGC->font, characterWidth);

  /*
   * Check for non-standard glyphs, glyphs that are too wide.
   */
  if ((GLYPHWIDTHBYTESPADDED (*ppci) != 4) || glyphWidth > 32)
    {
      xf86GCInfoRec.PolyGlyphBltFallBack (
		    pDrawable, pGC, xInit, yInit, nglyph, ppci, pglyphBase);
      return;
    }

  h = FONTASCENT (pGC->font) + FONTDESCENT (pGC->font);
  if (!(h && glyphWidth))
    return;

  TsengSetupForCPUToScreenColorExpand (
				-1, pGC->fgPixel, pGC->alu, pGC->planemask);

  x = xInit + FONTMAXBOUNDS (pGC->font, leftSideBearing) + pDrawable->x;
  y = yInit - FONTASCENT (pGC->font) + pDrawable->y;
  w = nglyph * glyphWidth;

#ifdef USE_FAST_ACLINIT
  /*
   * this is a replacement for the TsengSubsequentCPUToScreenColorExpand()
   * function, but optimized for this application.
   */
  destaddr = FBADDR (x, y);

  SET_XY (w, 1);
#endif

  for (count = 0; count < nglyph; count++)
    Glyphs[count] = (unsigned int *) FONTGLYPHBITS (pglyphBase, *ppci++);

  for (count = 0; count < h; count++, y++)
    {
#ifdef USE_FAST_ACLINIT
      WAIT_ACL;
      START_ACL (destaddr);
      destaddr += tseng_line_width;
#else
      TsengSubsequentCPUToScreenColorExpand (x, y, w, 1, 0);
#endif
      xf86DrawTextScanline ((unsigned int *) tsengCPU2ACLBase,
			    Glyphs, count, nglyph, glyphWidth);
    }

  xf86AccelInfoRec.Sync();
}


/*
 * Stipple acceleration code.
 */

/* shiftmasks for stipple acceleration */
static CARD32 ShiftMasks[32] =
{
  0x00000000, 0x00000001, 0x00000003, 0x00000007,
  0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,
  0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,
  0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,
  0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,
  0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,
  0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,
  0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff};



/*
   The TsengFillStippledRect function can replace:

   xf86AccelInfoRec.FillRectOpaqueStippled
   &
   xf86AccelInfoRec.FillRectStippled


   This function will probably be less satisfying since it will
   rarely be used.  In general, scanline expansions are only used
   when a bitmap won't fit in the cache, either because it's just
   too big or because video memory is low.  For the most part,
   the other options are preferred to the scanline expansion
   (blit from pixmap cache, 8x8 pattern fills).

   This code is based on stuff I wrote for the S3.  I was
   unhappy with the performance of the XAA stipple fallbacks in 
   cases where the pixmap cache couldn't be used.  It DOESN'T
   use a skipleft parameter.  All performance gains over XAA
   come from breaking up the code to handle 3 cases of stipple
   widths: powers of two <= 32, other less than 32, and larger
   stipple widths.

   As for x11perf:

   srect, osrect -

   These guys are 32x32 stipples reduceable to 8x8.  It would
   be good to establish a hierarchy between the 8x8 pattern fills,
   blitting from the pixmap cache, and the expansion routines.
   This function would only be used if there isn't enough room
   for the pixmap cache (or it's turned off), or if you specify
   DO_NOT_CACHE_STIPPLES.

   oddsrect, oddosrect -

   A 17x15 stipple.  Will only use this function if there's
   not enough room (need 15 lines) in the pixmap cache or with
   DO_NOT_CACHE_STIPPLES.

   bigsrect, bigosrect -

   This function will always be used, since the pixmap cache
   is never large enough to hold these stipples.  Conceivably
   I could make a skipleft version for the bigger stipples, but I 
   haven't yet.


   In general the ones that are used the least is where my
   version of this code has the biggest advantage over XAA,
   so I generally only think of this as a way to keep performance
   from sucking when there's not enough room for the pixmap cache.

   [ kmg ]
   Do we really need those shiftmasks?

   ShiftMasks[i] = (1 << i) - 1;

   This is really a very simple operation, probably even faster than indexing
   in an array -- and certainly a lot more cache-friendly (it's in-place)

 */

static void 
SetDWORDS (dest, value, dwords)
     register CARD32 *dest;
     register CARD32 value;
     register int dwords;
{
  while (dwords & ~0x03)
    {
      dest[0] = dest[1] = dest[2] = dest[3] = value;
      dest += 4;
      dwords -= 4;
    }
  switch (dwords)
    {
    case 1:
      dest[0] = value;
      break;
    case 2:
      dest[0] = dest[1] = value;
      break;
    case 3:
      dest[0] = dest[1] = dest[2] = value;
      break;
    }
}



/* this function attaches to those 2 xf86AccelInfoRec pointers.
   This function calls that previous function I posted */

void
TsengScanlineScreenToScreenFillStippledRect (pDrawable, pGC, nBoxInit, pBoxInit)
     DrawablePtr pDrawable;
     GCPtr pGC;
     int nBoxInit;		/* number of rectangles to fill */
     BoxPtr pBoxInit;		/* Pointer to first rectangle to fill */
{
  PixmapPtr pPixmap;		/* Pixmap of the area to draw */
  int rectWidth;		/* Width of the rect to be drawn */
  int rectHeight;		/* Height of the rect to be drawn */
  BoxPtr pBox;			/* current rectangle to fill */
  int nBox;			/* Number of rectangles to fill */
  int xoffset, yoffset;
  Bool AlreadySetup = FALSE;

  pPixmap = pGC->stipple;

  for (nBox = nBoxInit, pBox = pBoxInit; nBox > 0; nBox--, pBox++)
    {


      rectWidth = pBox->x2 - pBox->x1;
      rectHeight = pBox->y2 - pBox->y1;

      if ((rectWidth > 0) && (rectHeight > 0))
	{
	  if (!AlreadySetup)
	    {
	      TsengSetupForScreenToScreenColorExpand (
		       (pGC->fillStyle == FillStippled) ? -1 : pGC->bgPixel,
				    pGC->fgPixel, pGC->alu, pGC->planemask);

	      AlreadySetup = TRUE;
	    }

	  xoffset = (pBox->x1 - (pGC->patOrg.x + pDrawable->x))
	    % pPixmap->drawable.width;
	  if (xoffset < 0)
	    xoffset += pPixmap->drawable.width;
	  yoffset = (pBox->y1 - (pGC->patOrg.y + pDrawable->y))
	    % pPixmap->drawable.height;
	  if (yoffset < 0)
	    yoffset += pPixmap->drawable.height;
	  TsengSubsequentScanlineScreenToScreenFillStippledRect (
				  pBox->x1, pBox->y1, rectWidth, rectHeight,
				  pPixmap->devPrivate.ptr, pPixmap->devKind,
			  pPixmap->drawable.width, pPixmap->drawable.height,
							  xoffset, yoffset);
	}
    }				/* end for loop through each rectangle to draw */
  xf86AccelInfoRec.Sync();
}


void 
TsengSubsequentScanlineScreenToScreenFillStippledRect (x, y, w, h, src, srcwidth,
				    stipplewidth, stippleheight, srcx, srcy)
     int x, y, w, h;
     unsigned char *src;
     int srcwidth;
     int stipplewidth, stippleheight;
     int srcx, srcy;
{
  unsigned char *srcp;
  int dwords = (w + 31) >> 5;
  int line = 0;

  /* setup x/y/w in the ACL engine */
  ColorExpandDst = FBADDR (x, y);
  *ACL_MIX_Y_OFFSET = 0x0FFF;
  SET_XY (w, 1);

  srcp = (srcwidth * srcy) + src;

  if (!((stipplewidth > 32) || (stipplewidth & (stipplewidth - 1))))
    {
      CARD32 pattern;
      register unsigned char *kludge = (unsigned char *) (&pattern);

      while (h--)
	{
	  switch (stipplewidth)
	    {
	    case 32:
	      pattern = *((CARD32 *) srcp);
	      break;
	    case 16:
	      kludge[0] = kludge[2] = srcp[0];
	      kludge[1] = kludge[3] = srcp[1];
	      break;
	    case 8:
	      kludge[0] = kludge[1] = kludge[2] = kludge[3] = srcp[0];
	      break;
	    case 4:
	      kludge[0] = kludge[1] = kludge[2] = kludge[3] =
		(srcp[0] & 0x0F);
	      pattern |= (pattern << 4);
	      break;
	    case 2:
	      kludge[0] = kludge[1] = kludge[2] = kludge[3] =
		(srcp[0] & 0x03);
	      pattern |= (pattern << 2);
	      pattern |= (pattern << 4);
	      break;
	    default:		/* case 1: */
	      if (srcp[0] & 0x01)
		pattern = 0xffffffff;
	      else
		pattern = 0x00000000;
	      break;
	    }

	  if (srcx)
	    pattern = (pattern >> srcx) | (pattern << (32 - srcx));

	  if (line & 0x01)
	    {
	      SetDWORDS (tsengFirstLinePntr, pattern, dwords);
	      TsengSubsequentScanlineScreenToScreenColorExpand (tsengFirstLine << 3);
	    }
	  else
	    {
	      WAIT_ACL;
	      SetDWORDS (tsengSecondLinePntr, pattern, dwords);
	      TsengSubsequentScanlineScreenToScreenColorExpand (tsengSecondLine << 3);
	    }
	  line++;
	  srcy++;
	  srcp += srcwidth;
	  if (srcy >= stippleheight)
	    {
	      srcy = 0;
	      srcp = src;
	    }
	}
    }
  else if (stipplewidth < 32)
    {
      register int width, offset;
      int count;
      register CARD32 pattern;
      register CARD32 *DestPntr;

      while (h--)
	{
	  width = stipplewidth;
	  pattern = *((CARD32 *) srcp) & ShiftMasks[width];
	  while (!(width & ~15))
	    {
	      pattern |= (pattern << width);
	      width <<= 1;
	    }
	  pattern |= (pattern << width);

	  offset = srcx;
	  count = dwords;

	  if (line & 0x01)
	    {
	      DestPntr = tsengSecondLinePntr;
	      while (count--)
		{
		  *DestPntr = (pattern >> offset) |
		    (pattern << (width - offset));
		  DestPntr++;
		  offset += 32;
		  while (offset >= width)
		    offset -= width;
		}
	      TsengSubsequentScanlineScreenToScreenColorExpand (tsengSecondLine << 3);
	    }
	  else
	    {
	      DestPntr = tsengFirstLinePntr;
	      WAIT_ACL;
	      while (count--)
		{
		  *DestPntr = (pattern >> offset) |
		    (pattern << (width - offset));
		  DestPntr++;
		  offset += 32;
		  while (offset >= width)
		    offset -= width;
		}
	      TsengSubsequentScanlineScreenToScreenColorExpand (tsengFirstLine << 3);
	    }
	  line++;

	  srcy++;
	  srcp += srcwidth;
	  if (srcy >= stippleheight)
	    {
	      srcy = 0;
	      srcp = src;
	    }
	}
    }
  else
    {
      register CARD32 *scratch;
      int shift, offset, scratch2, count;
      register CARD32 *DestPntr;

      while (h--)
	{
	  count = dwords;
	  offset = srcx;

	  if (line & 0x01)
	    DestPntr = tsengSecondLinePntr;
	  else
	    {
	      DestPntr = tsengFirstLinePntr;
	      WAIT_ACL;
	    }
	  while (count--)
	    {
	      shift = stipplewidth - offset;
	      scratch = (CARD32 *) (srcp + (offset >> 3));
	      scratch2 = offset & 0x07;

	      if (shift & ~31)
		{
		  if (scratch2)
		    {
		      *DestPntr = (*scratch >> scratch2) |
			(scratch[1] << (32 - scratch2));
		    }
		  else
		    *DestPntr = *scratch;
		}
	      else
		{
		  *DestPntr = (*((CARD32 *) srcp) << shift) |
		    ((*scratch >> scratch2) & ShiftMasks[shift]);
		}
	      DestPntr++;
	      offset += 32;
	      while (offset >= stipplewidth)
		offset -= stipplewidth;
	    }
	  if (line & 0x01)
	    TsengSubsequentScanlineScreenToScreenColorExpand (tsengSecondLine << 3);
	  else
	    TsengSubsequentScanlineScreenToScreenColorExpand (tsengFirstLine << 3);

	  line++;
	  srcy++;
	  srcp += srcwidth;
	  if (srcy >= stippleheight)
	    {
	      srcy = 0;
	      srcp = src;
	    }
	}
    }
  xf86AccelInfoRec.Sync();
}


void
TsengScanlineCPUToScreenFillStippledRect (pDrawable, pGC, nBoxInit, pBoxInit)
     DrawablePtr pDrawable;
     GCPtr pGC;
     int nBoxInit;		/* number of rectangles to fill */
     BoxPtr pBoxInit;		/* Pointer to first rectangle to fill */
{
  PixmapPtr pPixmap;		/* Pixmap of the area to draw */
  int rectWidth;		/* Width of the rect to be drawn */
  int rectHeight;		/* Height of the rect to be drawn */
  BoxPtr pBox;			/* current rectangle to fill */
  int nBox;			/* Number of rectangles to fill */
  int xoffset, yoffset;
  Bool AlreadySetup = FALSE;

  pPixmap = pGC->stipple;

  for (nBox = nBoxInit, pBox = pBoxInit; nBox > 0; nBox--, pBox++)
    {


      rectWidth = pBox->x2 - pBox->x1;
      rectHeight = pBox->y2 - pBox->y1;

      if ((rectWidth > 0) && (rectHeight > 0))
	{
	  if (!AlreadySetup)
	    {
	      TsengSetupForCPUToScreenColorExpand (
		       (pGC->fillStyle == FillStippled) ? -1 : pGC->bgPixel,
				    pGC->fgPixel, pGC->alu, pGC->planemask);

	      AlreadySetup = TRUE;
	    }

	  xoffset = (pBox->x1 - (pGC->patOrg.x + pDrawable->x))
	    % pPixmap->drawable.width;
	  if (xoffset < 0)
	    xoffset += pPixmap->drawable.width;
	  yoffset = (pBox->y1 - (pGC->patOrg.y + pDrawable->y))
	    % pPixmap->drawable.height;
	  if (yoffset < 0)
	    yoffset += pPixmap->drawable.height;
	  TsengSubsequentScanlineCPUToScreenFillStippledRect (
				  pBox->x1, pBox->y1, rectWidth, rectHeight,
				  pPixmap->devPrivate.ptr, pPixmap->devKind,
			  pPixmap->drawable.width, pPixmap->drawable.height,
							  xoffset, yoffset);
	}
    }				/* end for loop through each rectangle to draw */
  xf86AccelInfoRec.Sync();
}


void 
TsengSubsequentScanlineCPUToScreenFillStippledRect (x, y, w, h, src, srcwidth,
				    stipplewidth, stippleheight, srcx, srcy)
     int x, y, w, h;
     unsigned char *src;
     int srcwidth;
     int stipplewidth, stippleheight;
     int srcx, srcy;
{
  unsigned char *srcp;
  int dwords = (w + 31) >> 5;
  int destaddr;

#ifdef USE_FAST_ACLINIT
  /*
   * this is a replacement for the TsengSubsequentCPUToScreenColorExpand()
   * function, but optimized for this application.
   */
  destaddr = FBADDR (x, y);

  SET_XY (w, 1);
#endif

  srcp = (srcwidth * srcy) + src;

  if (!((stipplewidth > 32) || (stipplewidth & (stipplewidth - 1))))
    {
      CARD32 pattern;
      register unsigned char *kludge = (unsigned char *) (&pattern);

      while (h--)
	{
	  switch (stipplewidth)
	    {
	    case 32:
	      pattern = *((CARD32 *) srcp);
	      break;
	    case 16:
	      kludge[0] = kludge[2] = srcp[0];
	      kludge[1] = kludge[3] = srcp[1];
	      break;
	    case 8:
	      kludge[0] = kludge[1] = kludge[2] = kludge[3] = srcp[0];
	      break;
	    case 4:
	      kludge[0] = kludge[1] = kludge[2] = kludge[3] =
		(srcp[0] & 0x0F);
	      pattern |= (pattern << 4);
	      break;
	    case 2:
	      kludge[0] = kludge[1] = kludge[2] = kludge[3] =
		(srcp[0] & 0x03);
	      pattern |= (pattern << 2);
	      pattern |= (pattern << 4);
	      break;
	    default:		/* case 1: */
	      if (srcp[0] & 0x01)
		pattern = 0xffffffff;
	      else
		pattern = 0x00000000;
	      break;
	    }

	  if (srcx)
	    pattern = (pattern >> srcx) | (pattern << (32 - srcx));

#ifdef USE_FAST_ACLINIT
	  WAIT_ACL;
	  START_ACL (destaddr);
	  destaddr += tseng_line_width;
#else
	  TsengSubsequentCPUToScreenColorExpand (x, y++, w, 1, 0);
#endif
	  SetDWORDS (tsengCPU2ACLBase, pattern, dwords);

	  srcy++;
	  srcp += srcwidth;
	  if (srcy >= stippleheight)
	    {
	      srcy = 0;
	      srcp = src;
	    }
	}
    }
  else if (stipplewidth < 32)
    {
      register int width, offset;
      int count;
      register CARD32 pattern;
      register CARD32 *DestPntr;

      while (h--)
	{
	  width = stipplewidth;
	  pattern = *((CARD32 *) srcp) & ShiftMasks[width];
	  while (!(width & ~15))
	    {
	      pattern |= (pattern << width);
	      width <<= 1;
	    }
	  pattern |= (pattern << width);

	  offset = srcx;
	  count = dwords;

#ifdef USE_FAST_ACLINIT
	  WAIT_ACL;
	  START_ACL (destaddr);
	  destaddr += tseng_line_width;
#else
	  TsengSubsequentCPUToScreenColorExpand (x, y++, w, 1, 0);
#endif
	  DestPntr = (CARD32 *) tsengCPU2ACLBase;
	  while (count--)
	    {
	      *DestPntr = (pattern >> offset) |
		(pattern << (width - offset));
	      DestPntr++;
	      offset += 32;
	      while (offset >= width)
		offset -= width;
	    }

	  srcy++;
	  srcp += srcwidth;
	  if (srcy >= stippleheight)
	    {
	      srcy = 0;
	      srcp = src;
	    }
	}
    }
  else
    {
      register CARD32 *scratch;
      int shift, offset, scratch2, count;
      register CARD32 *DestPntr;

      while (h--)
	{
	  count = dwords;
	  offset = srcx;

	  DestPntr = (CARD32 *) tsengCPU2ACLBase;

#ifdef USE_FAST_ACLINIT
	  WAIT_ACL;
	  START_ACL (destaddr);
	  destaddr += tseng_line_width;
#else
	  TsengSubsequentCPUToScreenColorExpand (x, y++, w, 1, 0);
#endif
	  while (count--)
	    {
	      shift = stipplewidth - offset;
	      scratch = (CARD32 *) (srcp + (offset >> 3));
	      scratch2 = offset & 0x07;

	      if (shift & ~31)
		{
		  if (scratch2)
		    {
		      *DestPntr = (*scratch >> scratch2) |
			(scratch[1] << (32 - scratch2));
		    }
		  else
		    *DestPntr = *scratch;
		}
	      else
		{
		  *DestPntr = (*((CARD32 *) srcp) << shift) |
		    ((*scratch >> scratch2) & ShiftMasks[shift]);
		}
	      DestPntr++;
	      offset += 32;
	      while (offset >= stipplewidth)
		offset -= stipplewidth;
	    }

	  srcy++;
	  srcp += srcwidth;
	  if (srcy >= stippleheight)
	    {
	      srcy = 0;
	      srcp = src;
	    }
	}
    }
  xf86AccelInfoRec.Sync();
}
