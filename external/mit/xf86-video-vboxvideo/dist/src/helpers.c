/*
 * Copyright (C) 2014-2017 Oracle Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "vboxvideo.h"
#include "os.h"
#include "propertyst.h"
#include "windowstr.h"
#include "xf86.h"
#include <X11/Xatom.h>
#ifdef XORG_7X
# include <string.h>
#endif
#include "VBoxVideoErr.h"

VBOXPtr vbvxGetRec(ScrnInfoPtr pScrn)
{
    return ((VBOXPtr)pScrn->driverPrivate);
}

int vbvxGetIntegerPropery(ScrnInfoPtr pScrn, char *pszName, size_t *pcData, int32_t **ppaData)
{
    Atom atom;
    PropertyPtr prop;

    /* We can get called early, before the root window is created. */
    if (!ROOT_WINDOW(pScrn))
        return VERR_NOT_FOUND;
    atom = MakeAtom(pszName, strlen(pszName), TRUE);
    if (atom == BAD_RESOURCE)
        return VERR_NOT_FOUND;
    for (prop = wUserProps(ROOT_WINDOW(pScrn));
         prop != NULL && prop->propertyName != atom; prop = prop->next);
    if (prop == NULL)
        return VERR_NOT_FOUND;
    if (prop->type != XA_INTEGER || prop->format != 32)
        return VERR_NOT_FOUND;
    *pcData = prop->size;
    *ppaData = (int32_t *)prop->data;
    return VINF_SUCCESS;
}

void vbvxSetIntegerPropery(ScrnInfoPtr pScrn, char *pszName, size_t cData, int32_t *paData, Bool fSendEvent)
{
    Atom property_name;

    property_name = MakeAtom(pszName, strlen(pszName), TRUE);
    AssertMsg(property_name != BAD_RESOURCE, ("Failed to set atom \"%s\"\n", pszName));
    ChangeWindowProperty(ROOT_WINDOW(pScrn), property_name, XA_INTEGER, 32, PropModeReplace, cData, paData, fSendEvent);
}

void vbvxReprobeCursor(ScrnInfoPtr pScrn)
{
    if (ROOT_WINDOW(pScrn) == NULL)
        return;
#ifdef XF86_SCRN_INTERFACE
    pScrn->EnableDisableFBAccess(pScrn, FALSE);
    pScrn->EnableDisableFBAccess(pScrn, TRUE);
#else
    pScrn->EnableDisableFBAccess(pScrn->scrnIndex, FALSE);
    pScrn->EnableDisableFBAccess(pScrn->scrnIndex, TRUE);
#endif
}
