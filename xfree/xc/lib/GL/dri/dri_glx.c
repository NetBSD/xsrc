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
/* $XFree86: xc/lib/GL/dri/dri_glx.c,v 1.7 2000/09/26 15:56:45 tsi Exp $ */

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *   Brian Paul <brian@precisioninsight.com>
 *
 */

#ifdef GLX_DIRECT_RENDERING

#include <unistd.h>
#include <Xlibint.h>
#include <Xext.h>
#include <extutil.h>
#include "glxclient.h"
#include "xf86dri.h"
#include "sarea.h"
#include <stdio.h>
#include <dlfcn.h>
#include "dri_glx.h"
#include <sys/types.h>


typedef void *(*CreateScreenFunc)(Display *dpy, int scrn, __DRIscreen *psc,
                                  int numConfigs, __GLXvisualConfig *config);



#ifdef BUILT_IN_DRI_DRIVER

extern void *__driCreateScreen(Display *dpy, int scrn, __DRIscreen *psc,
                               int numConfigs, __GLXvisualConfig *config);


#else /* BUILT_IN_DRI_DRIVER */


#ifndef DEFAULT_DRIVER_DIR
/* this is normally defined in the Imakefile */
#define DEFAULT_DRIVER_DIR "/usr/X11R6/lib/modules/dri"
#endif


static void ErrorMessage(const char *msg)
{
    if (getenv("LIBGL_DEBUG")) {
        fprintf(stderr, "libGL error: %s\n", msg);
    }
}


static void InfoMessage(const char *msg)
{
    const char *env = getenv("LIBGL_DEBUG");
    if (env && strstr(env, "verbose")) {
        fprintf(stderr, "libGL: %s\n", msg);
    }
}


/*
 * We'll save a pointer to this function when we couldn't find a
 * direct rendering driver for a given screen.
 */
static void *DummyCreateScreen(Display *dpy, int scrn, __DRIscreen *psc,
                               int numConfigs, __GLXvisualConfig *config)
{
    (void) dpy;
    (void) scrn;
    (void) psc;
    (void) numConfigs;
    (void) config;
    return NULL;
}



/*
 * Extract the ith directory path out of a colon-separated list of
 * paths.
 * Input:
 *   index - index of path to extract (starting at zero)
 *   paths - the colon-separated list of paths
 *   dirLen - max length of result to store in <dir>
 * Output:
 *   dir - the extracted directory path, dir[0] will be zero when
 *         extraction fails.
 */
static void ExtractDir(int index, const char *paths, int dirLen, char *dir)
{
   int i, len;
   const char *start, *end;

   /* find ith colon */
   start = paths;
   i = 0;
   while (i < index) {
      if (*start == ':') {
         i++;
         start++;
      }
      else if (*start == 0) {
         /* end of string and couldn't find ith colon */
         dir[0] = 0;
         return;
      }
      else {
         start++;
      }
   }

   while (*start == ':')
      start++;

   /* find next colon, or end of string */
   end = start + 1;
   while (*end != ':' && *end != 0) {
      end++;
   }

   /* copy string between <start> and <end> into result string */
   len = end - start;
   if (len > dirLen - 1)
      len = dirLen - 1;
   strncpy(dir, start, len);
   dir[len] = 0;
}



/*
 * Try to dlopen() the named driver.  This function adds the
 * "_dri.so" suffix to the driver name and searches the
 * directories specified by the LIBGL_DRIVERS_PATH env var
 * in order to find the driver.
 * Input:
 *   driverName - a name like "tdfx", "i810", "mga", etc.
 * Return:
 *   handle from dlopen, or NULL if driver file not found.
 */
static void *OpenDriver(const char *driverName)
{
   char *libPaths = NULL;
   int i;

   if (geteuid() == getuid()) {
      /* don't allow setuid apps to use LIBGL_DRIVERS_PATH */
      libPaths = getenv("LIBGL_DRIVERS_PATH");
      if (!libPaths)
         libPaths = getenv("LIBGL_DRIVERS_DIR"); /* deprecated */
   }
   if (!libPaths)
      libPaths = DEFAULT_DRIVER_DIR;

   for (i = 0; ; i++) {
      char libDir[1000], info[1000], realDriverName[100];
      void *handle;
      ExtractDir(i, libPaths, 1000, libDir);
      if (!libDir[0])
         return NULL;
      sprintf(realDriverName, "%s/%s_dri.so", libDir, driverName);
      sprintf(info, "trying %s", realDriverName);
      InfoMessage(info);
      handle = dlopen(realDriverName, RTLD_NOW | RTLD_GLOBAL);
      if (handle) {
         return handle;
      }
      else {
         char message[1000];
         snprintf(message, 1000, "dlopen failed: %s", dlerror());
         ErrorMessage(message);
      }
   }

   return NULL;
}



/*
 * Initialize two arrays:  an array of createScreen function pointers
 * and an array of dlopen library handles.  Arrays are indexed by
 * screen number.
 * We use the DRI in order to find the __driCreateScreen function
 * exported by each screen on a display.
 */
static void Find_CreateScreenFuncs(Display *dpy,
                                   CreateScreenFunc *createFuncs,
                                   void **libraryHandles)
{
    const int numScreens = ScreenCount(dpy);
    int scrn;

    __glXRegisterExtensions();

    for (scrn = 0; scrn < numScreens; scrn++) {
        int directCapable;
        Bool b;
        int driverMajor, driverMinor, driverPatch;
        char *driverName = NULL;
        void *handle;

        /* defaults */
        createFuncs[scrn] = DummyCreateScreen;
        libraryHandles[scrn] = NULL;

        if (!XF86DRIQueryDirectRenderingCapable(dpy, scrn, &directCapable)) {
            ErrorMessage("XF86DRIQueryDirectRenderingCapable failed");
            continue;
        }
        if (!directCapable) {
            ErrorMessage("XF86DRIQueryDirectRenderingCapable returned false");
            continue;
        }

        /*
         * Use DRI to find the device driver for use on screen number 'scrn'.
         */
        b = XF86DRIGetClientDriverName(dpy, scrn, &driverMajor, &driverMinor,
                                       &driverPatch, &driverName);
        if (!b) {
            char message[1000];
            snprintf(message, 1000, "Cannot determine driver name for screen %d", scrn);
            ErrorMessage(message);
            continue;
        }


        /*
         * Open the driver module and save the pointer to its
         * __driCreateScreen function.
         */
        handle = OpenDriver(driverName);
        if (handle) {
           CreateScreenFunc createScreenFunc;
           createScreenFunc = (CreateScreenFunc) dlsym(handle, "__driCreateScreen");
           if (createScreenFunc) {
              /* success! */
              createFuncs[scrn] = createScreenFunc;
              libraryHandles[scrn] = handle;
              break;  /* onto the next screen */
           }
           else {
              ErrorMessage("driCreateScreen() not defined in driver!");
              dlclose(handle);
           }
        }
    } /* for scrn */
}

#endif /* BUILT_IN_DRI_DRIVER */


static void driDestroyDisplay(Display *dpy, void *private)
{
    __DRIdisplayPrivate *pdpyp = (__DRIdisplayPrivate *)private;

    if (pdpyp) {
        const int numScreens = ScreenCount(dpy);
        int i;
        for (i = 0; i < numScreens; i++) {
            if (pdpyp->libraryHandles[i])
                dlclose(pdpyp->libraryHandles[i]);
        }
        Xfree(pdpyp->libraryHandles);
	Xfree(pdpyp);
    }
}


void *driCreateDisplay(Display *dpy, __DRIdisplay *pdisp)
{
    const int numScreens = ScreenCount(dpy);
    __DRIdisplayPrivate *pdpyp;
    int eventBase, errorBase;
    int major, minor, patch;

    /* Initialize these fields to NULL in case we fail.
     * If we don't do this we may later get segfaults trying to free random
     * addresses when the display is closed.
     */
    pdisp->private = NULL;
    pdisp->destroyDisplay = NULL;
    pdisp->createScreen = NULL;

    if (!XF86DRIQueryExtension(dpy, &eventBase, &errorBase)) {
	return NULL;
    }

    if (!XF86DRIQueryVersion(dpy, &major, &minor, &patch)) {
	return NULL;
    }

    pdpyp = (__DRIdisplayPrivate *)Xmalloc(sizeof(__DRIdisplayPrivate));
    if (!pdpyp) {
	return NULL;
    }

    pdpyp->driMajor = major;
    pdpyp->driMinor = minor;
    pdpyp->driPatch = patch;

    pdisp->destroyDisplay = driDestroyDisplay;

    /* allocate array of pointers to createScreen funcs */
    pdisp->createScreen = (CreateScreenFunc *) Xmalloc(numScreens * sizeof(void *));
    if (!pdisp->createScreen)
       return NULL;

    /* allocate array of library handles */
    pdpyp->libraryHandles = (void **) Xmalloc(numScreens * sizeof(void*));
    if (!pdpyp->libraryHandles) {
       Xfree(pdisp->createScreen);
       return NULL;
    }

#ifdef BUILT_IN_DRI_DRIVER
    /* we'll statically bind to the __driCreateScreen function */
    {
       int i;
       for (i = 0; i < numScreens; i++) {
          pdisp->createScreen[i] = __driCreateScreen;
          pdpyp->libraryHandles[i] = NULL;
       }
    }
#else
    Find_CreateScreenFuncs(dpy, pdisp->createScreen, pdpyp->libraryHandles);
#endif

    return (void *)pdpyp;
}


#ifndef BUILT_IN_DRI_DRIVER
/*
 * Use the DRI and dlopen/dlsym facilities to find the GL extensions
 * possible on the given display and screen.
 */
static void
register_extensions_on_screen(Display *dpy, int scrNum)
{
   GLboolean verbose = GL_FALSE;  /* for debugging only */
   int eventBase, errorBase;
   Bool b, b2;
   int driMajor, driMinor, driPatch;
   int driverMajor, driverMinor, driverPatch;
   char *driverName = NULL;
   void *handle;

   /*
    * Check if the DRI extension is available, check the DRI version,
    * determine the 3D driver for the screen.
    */
   b = XF86DRIQueryExtension(dpy, &eventBase, &errorBase);
   if (!b) {
      if (verbose)
         fprintf(stderr, "XF86DRIQueryExtension failed\n");
      return;
   }

   b = XF86DRIQueryDirectRenderingCapable(dpy, scrNum, &b2);
   if (!b || !b2) {
      if (verbose)
         fprintf(stderr, "XF86DRIQueryDirectRenderingCapable failed\n");
      return;
   }

   b = XF86DRIQueryVersion(dpy, &driMajor, &driMinor, &driPatch);
   if (!b) {
      if (verbose)
         fprintf(stderr, "XF86DRIQueryVersion failed\n");
      return;
   }

   b = XF86DRIGetClientDriverName(dpy, scrNum, &driverMajor, &driverMinor,
                                  &driverPatch, &driverName);
   if (!b) {
      if (verbose)
         fprintf(stderr, "XF86DRIGetClientDriverName failed\n");
      return;
   }
   else if (verbose) {
      printf("XF86DRIGetClientDriverName: %d.%d.%d %s\n", driverMajor,
             driverMinor, driverPatch, driverName);
   }

   /*
    * OK, now we know the name of the relevant driver for this screen.
    * dlopen() the driver library file, get a pointer to the driver's
    * __driRegisterExtensions() function, and call it if it exists.
    */
   handle = OpenDriver(driverName);
   if (handle) {
      typedef void *(*RegisterExtFunc)(void);
      RegisterExtFunc registerExtFunc = (RegisterExtFunc) dlsym(handle,
                                                    "__driRegisterExtensions");
      if (registerExtFunc) {
         (*registerExtFunc)();
      }
      dlclose(handle);
      return;
   }
}
#endif /* !BUILT_IN_DRI_DRIVER */



/*
** Here we'll query the DRI driver for each screen and let each
** driver register its GL extension functions.  We only have to
** do this once.  But it MUST be done before we create any contexts
** (i.e. before any dispatch tables are created) and before
** glXGetProcAddressARB() returns.
**
** Currently called by glXGetProcAddress(), __glXInitialize(), and
** __glXNewIndirectAPI().
*/
void
__glXRegisterExtensions(void)
{
   static GLboolean alreadyCalled = GL_FALSE;
   if (alreadyCalled)
      return;

#ifndef BUILT_IN_DRI_DRIVER
   {
      int displayNum, maxDisplays;
      if (getenv("LIBGL_MULTIHEAD"))
          maxDisplays = 10;  /* infinity, really */
      else
          maxDisplays = 1;
      for (displayNum = 0; displayNum < maxDisplays; displayNum++) {
         char displayName[200];
         Display *dpy;
         snprintf(displayName, 199, ":%d.0", displayNum);
         dpy = XOpenDisplay(displayName);
         if (dpy) {
            const int numScreens = ScreenCount(dpy);
            int screenNum;
            for (screenNum = 0; screenNum < numScreens; screenNum++) {
               register_extensions_on_screen(dpy, screenNum);
            }
            XCloseDisplay(dpy);
         }
         else {
            break;
         }
      }
   }

   alreadyCalled = GL_TRUE;
#endif
}


#endif /* GLX_DIRECT_RENDERING */
