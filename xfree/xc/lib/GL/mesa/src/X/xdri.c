
#if defined(GLX_DIRECT_RENDERING) 

/* Mesa includes
 */
#ifdef HAVE_CONFIG_H
#include "conf.h"
#endif

#include "GL/xmesa.h"
#include "xmesaP.h"
#include "context.h"
#include "macros.h"
#include "matrix.h"
#include "types.h"

/* Direct rendering includes
 */
#include "dri_mesaint.h"


void XMesaDriSwapBuffers( XMesaBuffer b )
{
   __DRIdrawablePrivate *pdp = b->driDrawPriv;
   __DRIscreenPrivate *psp = pdp->driScreenPriv;
   drmContext hHWContext = pdp->driContextPriv->hHWContext;

   /*
   ** Grab the lock and make sure drawable info is still
   ** up to date.
   */
   DRM_LIGHT_LOCK(psp->fd, &psp->pSAREA->lock, hHWContext);
   XMESA_VALIDATE_DRAWABLE_INFO(b->display, psp, pdp);

   /* Copy back image to front buffer */
   if (pdp->numClipRects) {
      int numClipRects = pdp->numClipRects;
      XF86DRIClipRectPtr pRect = pdp->pClipRects;
      int y;
      GLbyte *s8, *d8;
      GLuint *s32, *d32;

      while (numClipRects--) {
	 int w = pRect->x2-pRect->x1;

	 switch (psp->fbBPP) {
	 case 8:
	    for (y = pRect->y1; y < pRect->y2; y++) {
	       s8 = (GLbyte *)b->backimage->data +
		  y*b->backimage->bytes_per_line +
		  pRect->x1;
	       d8 = (GLbyte *)psp->pFB + psp->fbOrigin +
		  (pdp->y + y)*psp->fbStride +
		  pdp->x + pRect->x1;
	       memcpy(d8, s8, w);
	    }
	    break;
	 case 15:
	 case 16:
	    break;
	 case 24:
	    break;
	 case 32:
	    for (y = pRect->y1; y < pRect->y2; y++) {
	       /* These are calculated in GLbytes */
	       s8 = (GLbyte *)b->backimage->data +
		  (y - pdp->y)*b->backimage->bytes_per_line;
	       d8 = (GLbyte *)psp->pFB + psp->fbOrigin +
		  y*psp->fbStride;
	       s32 = (GLuint *)s8;
	       d32 = (GLuint *)d8;
	       /* These are calculated in GLuints */
	       s32 += (pRect->x1 - pdp->x);
	       d32 += pRect->x1;
	       memcpy(d32, s32, w<<2);
	    }
	    break;
	 }

	 pRect++;
      }
   }
   /* Unlock the screen */
   DRM_UNLOCK(psp->fd, &psp->pSAREA->lock, hHWContext);
}



/*
 * Initialize the XMesa driver.
 */
GLboolean XMesaInitDriver( __DRIscreenPrivate *driScrnPriv )
{
    return GL_TRUE;
}

/*
 * Reset the XMesa driver when the X server resets.
 */
void XMesaResetDriver( __DRIscreenPrivate *driScrnPriv )
{
}

#else

extern void i_hate_these_stupid_dummy_functions();
void i_hate_these_stupid_dummy_functions()
{
}

#endif
