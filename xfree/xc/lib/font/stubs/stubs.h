/* $XFree86: xc/lib/font/stubs/stubs.h,v 1.2 1999/08/21 13:48:07 dawes Exp $ */

/* This directory includes dummy entry for bdftopcf and mkfontdir */

#include <stdio.h>
#include "fntfilst.h"
#include "font.h"


#ifndef True
#define True (-1)
#endif
#ifndef False
#define False (0)
#endif

extern FontPtr find_old_font ( FSID id );
extern FontResolutionPtr GetClientResolutions ( int *num );
extern int GetDefaultPointSize ( void );
extern int set_font_authorizations ( char **authorizations, 
				     int *authlen, 
				     ClientPtr client );
extern Bool XpClientIsBitmapClient ( ClientPtr client );
extern Bool XpClientIsPrintClient ( ClientPtr client, 
				    FontPathElementPtr fpe );

extern unsigned long GetTimeInMillis (void);

/* end of file */
