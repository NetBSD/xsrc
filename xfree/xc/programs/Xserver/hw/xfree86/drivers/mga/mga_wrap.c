/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/mga/mga_wrap.c,v 1.2 2000/09/24 13:51:28 alanh Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Keith Whitwell <keithw@precisioninsight.com>
 *
 */


#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86Priv.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"
#define PSZ 8
#include "cfb.h"
#undef PSZ
#include "cfb16.h"
#include "cfb32.h"

#include "miline.h"

#include "GL/glxtokens.h"

#include "mga_bios.h"
#include "mga_reg.h"
#include "mga.h"
#include "mga_macros.h"
#include "mga_dri.h"
#include "mga_wrap.h"



static void MGAWakeupHandler(int screenNum,
			     pointer wakeupData,
			     unsigned long result,
			     pointer pReadmask)
{
    ScreenPtr pScreen = screenInfo.screens[screenNum];
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    /*DRIWrappedFuncsRec *pDRIWrap = DRIGetWrappedFuncs(pScreen);*/

    if (0) ErrorF("MGAWakeupHandler (in)\n");

    /* Disabled: Check contention like the 3d clients do before trying
     *   to restore state.  
     */
    DRILock(pScreen, 0);
    if (xf86IsEntityShared(pScrn->entityList[0]))
        MGASwapContext_shared(pScreen);
    else
        MGASwapContext(pScreen);
}

static void MGABlockHandler(int screenNum,
			    pointer blockData,
			    pointer pTimeout,
			    pointer pReadmask)

{
   ScreenPtr pScreen = screenInfo.screens[screenNum];
   ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
   MGAPtr pMga = MGAPTR(pScrn);
   /*DRIWrappedFuncsRec *pDRIWrap = DRIGetWrappedFuncs(pScreen);*/
   MGASAREAPtr sa = (MGASAREAPtr)DRIGetSAREAPrivate( pScreen );

   if (0) ErrorF("MGABlockHandler (out)\n");


   /* Examine the cliprects for the most recently used 3d drawable.
    * If they've changed, attempt to push the updated values into the
    * sarea.
    *
    * This avoids the protocol round trip in all single-client
    * frontbuffer rendering cases providing the cliprects fit into the
    * sarea, and in all single-client backbuffer rendering with arbitary
    * numbers of cliprects, for all operations except swapbuffers.
    *
    * Thus the number of round trips in the cases where comparison is
    * possible is reduced to no more (and usually fewer) than the number
    * Utah requires.  
    */

   if (sa->req_drawable != sa->exported_drawable ||
       sa->exported_stamp != DRIGetDrawableStamp( pScreen, sa->exported_index ))
   {
      int i;
      XF86DRIClipRectPtr frontboxes, backboxes;
      XF86DRIClipRectPtr boxes = (XF86DRIClipRectPtr)sa->exported_boxes;
      WindowPtr window = LookupIDByType( sa->req_drawable, RT_WINDOW );

      if (0)
	 ErrorF("Trying to update req_drawable: %d (exp %d), stamp %d/%d\n", 
		sa->req_drawable, sa->exported_drawable);

      sa->exported_drawable = 0;

      if (!window) {
	 if (0)
	    ErrorF("Couldn't retreive window\n");
	 sa->req_drawable = 0;
	 goto finished;
      }

      if (!DRIGetDrawableInfo( pScreen, &(window->drawable), 
			       &sa->exported_index, 
			       &sa->exported_stamp, 
			       &sa->exported_front_x, 
			       &sa->exported_front_y, 
			       &sa->exported_w, 
			       &sa->exported_h,
			       &sa->exported_nfront, 
			       &frontboxes,
			       &sa->exported_back_x, 
			       &sa->exported_back_y,
			       &sa->exported_nback, 
			       &backboxes ))
      {
	 if (0)
	    ErrorF("Couldn't get DRI info\n");
	 sa->req_drawable = 0;
	 goto finished;
      }

      /* If we can't fit both sets of cliprects into the sarea, try to
       * fit the current draw buffer.  Otherwise return.
       */
      
      sa->exported_buffers = MGA_FRONT | MGA_BACK;

      if (sa->exported_nback + sa->exported_nfront >= MGA_NR_SAREA_CLIPRECTS) {
	 if (sa->req_draw_buffer == MGA_FRONT) {
	    sa->exported_buffers = MGA_FRONT;
	    if (sa->exported_nfront >= MGA_NR_SAREA_CLIPRECTS)
	       goto finished;
	 } else {
	    sa->exported_buffers = MGA_BACK;
	    if (sa->exported_nback >= MGA_NR_SAREA_CLIPRECTS)
	       goto finished;
	 }
      }

      if (sa->exported_buffers & MGA_BACK) {
	 for (i = 0 ; i < sa->exported_nback ; i++) 
	    *boxes++ = backboxes[i];	 
      }

      if (sa->exported_buffers & MGA_FRONT) {
	 for (i = 0 ; i < sa->exported_nfront ; i++) 
	    *boxes++ = frontboxes[i];	 
      }

      sa->exported_drawable = sa->req_drawable;
   } 

 finished:
   if (xf86IsEntityShared(pScrn->entityList[0])) {
      /* Restore to first screen */
      pMga->RestoreAccelState(pScrn);
      xf86SetLastScrnFlag(pScrn->entityList[0], pScrn->scrnIndex);
   }

   DRIUnlock(pScreen);
}




/* Just wrap validate tree for now to remove the quiescense hack.
 * This is more of a win for the i810 than the mga, but should be useful
 * here too.
 */
void MGADRIWrapFunctions(ScreenPtr pScreen, DRIInfoPtr pDRIInfo)
{
   pDRIInfo->wrap.BlockHandler = MGABlockHandler;   
   pDRIInfo->wrap.WakeupHandler = MGAWakeupHandler;    
/*     pDRIInfo->wrap.ValidateTree = NULL; */
/*     pDRIInfo->wrap.PostValidateTree = NULL; */
}


