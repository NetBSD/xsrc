/* ddcProperty.c: Make the DDC monitor information available to clients
 * as properties on the root window
 * 
 * Copyright 1999 by Andrew C Aitchison <A.C.Aitchison@dpmms.cam.ac.uk>
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/ddc/ddcProperty.c,v 1.4 2000/11/06 19:24:07 dawes Exp $ */

#include "misc.h"
#include "xf86.h"
/* #include "xf86_ansic.h" */
/* #include "xf86_OSproc.h" */
#include "Xatom.h"
#include "property.h"
#include "propertyst.h"
#include "xf86DDC.h"

#define EDID1_ATOM_NAME         "XFree86_DDC_EDID1_RAWDATA"
#define EDID2_ATOM_NAME         "XFree86_DDC_EDID2_RAWDATA"
#define VDIF_ATOM_NAME          "XFree86_DDC_VDIF_RAWDATA"

Bool
xf86SetDDCproperties(ScrnInfoPtr pScrnInfo, xf86MonPtr DDC)
{
    Atom EDID1Atom=-1, EDID2Atom=-1, VDIFAtom=-1;
    CARD8 *EDID1rawdata = NULL;
    CARD8 *EDID2rawdata = NULL;
    CARD8 *VDIFrawdata = NULL;
    int  i, ret;

#ifdef DEBUG
    ErrorF("xf86SetDDCproperties(%p, %p)\n", pScrnInfo, DDC);
#endif

    if (pScrnInfo==NULL || pScrnInfo->monitor==NULL || DDC==NULL) {
      return FALSE;
    }

#ifdef DEBUG
    ErrorF("pScrnInfo->scrnIndex %d\n", pScrnInfo->scrnIndex);

    ErrorF("pScrnInfo->monitor was %p\n", pScrnInfo->monitor);
#endif

    pScrnInfo->monitor->DDC = DDC;

    if (DDC->ver.version == 1) {

      if ( (EDID1rawdata = xalloc(128*sizeof(CARD8)))==NULL ) {
	return FALSE;
      }

      EDID1Atom = MakeAtom(EDID1_ATOM_NAME, sizeof(EDID1_ATOM_NAME), TRUE);


      for (i=0; i<128; i++) {
	EDID1rawdata[i] = DDC->rawData[i];
      }

#ifdef DEBUG
      ErrorF("xf86RegisterRootWindowProperty %p(%d,%d,%d,%d,%d,%p)\n",
	     xf86RegisterRootWindowProperty,
	     pScrnInfo->scrnIndex,
	     EDID1Atom, XA_INTEGER, 8,
	     128, (unsigned char *)EDID1rawdata  );
#endif

      ret = xf86RegisterRootWindowProperty(pScrnInfo->scrnIndex,
					   EDID1Atom, XA_INTEGER, 8, 
					   128, (unsigned char *)EDID1rawdata
					   );
#ifdef DEBUG
      ErrorF("xf86RegisterRootWindowProperty returns %d\n", ret );
#endif

    } else if (DDC->ver.version == 2) {
      if ( (EDID2rawdata = xalloc(256*sizeof(CARD8)))==NULL ) {
	xfree(EDID2rawdata);
	return FALSE;
      }
      for (i=0; i<256; i++) {
	EDID2rawdata[i] = DDC->rawData[i];
      }

      EDID2Atom = MakeAtom(EDID2_ATOM_NAME, sizeof(EDID2_ATOM_NAME), TRUE);

#ifdef DEBUG
      ErrorF("xf86RegisterRootWindowProperty %p(%d,%d,%d,%d,%d,%p)\n",
	     xf86RegisterRootWindowProperty,
	     pScrnInfo->scrnIndex,
	     EDID2Atom, XA_INTEGER, 8,
	     256, (unsigned char *)EDID2rawdata  );
#endif
      ret = xf86RegisterRootWindowProperty(pScrnInfo->scrnIndex,
					   EDID2Atom, XA_INTEGER, 8, 
#if 1
					   256, (unsigned char *)EDID1rawdata
#else
#define EDID2_DUMMY_STRING "Dummy EDID2 property - please insert correct values"
					   strlen(EDID2_DUMMY_STRING),
					   EDID2_DUMMY_STRING 
#endif
					   );
#ifdef DEBUG
      ErrorF("xf86RegisterRootWindowProperty returns %d\n", ret );
#endif
    } else {
     xf86DrvMsg(pScrnInfo->scrnIndex, X_PROBED,
		"unexpected EDID version %d revision %d\n",
		DDC->ver.version, DDC->ver.revision );      
    }

    if (DDC->vdif) {
#define VDIF_DUMMY_STRING "setting dummy VDIF property - please insert correct values\n"
#ifdef DEBUG
      ErrorF("xf86RegisterRootWindowProperty %p(%d,%d,%d,%d,%d,%p)\n",
	     xf86RegisterRootWindowProperty,
	     pScrnInfo->scrnIndex,
	     VDIFAtom, XA_STRING, 8,
	     strlen(VDIF_DUMMY_STRING), VDIF_DUMMY_STRING 
	     );
#endif


      VDIFAtom = MakeAtom(VDIF_ATOM_NAME, sizeof(VDIF_ATOM_NAME), TRUE);

      ret = xf86RegisterRootWindowProperty(pScrnInfo->scrnIndex,
					   VDIFAtom, XA_STRING, 8, 
					   strlen(VDIF_DUMMY_STRING),
					   VDIF_DUMMY_STRING 
					   );
#ifdef DEBUG
      ErrorF("xf86RegisterRootWindowProperty returns %d\n", ret );
#endif
    }

    return TRUE;
}
