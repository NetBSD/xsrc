/* $XFree86: xc/lib/Xi/XIint.h,v 3.0.4.1 1999/07/21 18:07:28 hohndel Exp $ */

/*
 *	XIint.h - Header definition and support file for the internal
 *	support routines used by the Xi library.
 */

XExtDisplayInfo * XInput_find_display(
#if NeedFunctionPrototypes
		Display*
#endif
		);

XExtensionVersion * XInput_get_extension_version(
#if NeedFunctionPrototypes
               Display  *,
	       _Xconst char *
#endif
                );
