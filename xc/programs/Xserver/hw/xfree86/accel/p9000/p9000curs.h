/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/p9000/p9000curs.h,v 3.2 1996/02/04 09:04:13 dawes Exp $ */





/* $XConsortium: p9000curs.h /main/3 1995/11/12 18:18:52 kaleb $ */

extern Bool p9000BlockCursor;
extern Bool p9000ReloadCursor;

#define BLOCK_CURSOR    p9000BlockCursor = TRUE;

#define UNBLOCK_CURSOR  { \
			    if (p9000ReloadCursor) \
			       p9000RestoreCursor(p9000savepScreen); \
			    p9000BlockCursor = FALSE; \
                        }
