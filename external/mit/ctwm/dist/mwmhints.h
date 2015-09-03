#ifndef _MWMHINTS_
#define _MWMHINTS_

#include <X11/Xlib.h>
#include "types.h"

/*
 * Contents of the _MOTIF_WM_HINTS property.
 */

#undef FULL_MWM_DATA

typedef struct
{
    int	         flags;
    int		 functions;
    int		 decorations;
#ifdef FULL_MWM_DATA
    int		 input_mode;
    int		 status;
#endif
} MotifWmHints;

/* bit definitions for MwmHints.flags */
#define MWM_HINTS_FUNCTIONS	(1L << 0)
#define MWM_HINTS_DECORATIONS	(1L << 1)
#define MWM_HINTS_INPUT_MODE	(1L << 2)
#define MWM_HINTS_STATUS	(1L << 3)

/* bit definitions for MwmHints.functions */
#define MWM_FUNC_ALL		(1L << 0)
#define MWM_FUNC_RESIZE		(1L << 1)
#define MWM_FUNC_MOVE		(1L << 2)
#define MWM_FUNC_MINIMIZE	(1L << 3)
#define MWM_FUNC_MAXIMIZE	(1L << 4)
#define MWM_FUNC_CLOSE		(1L << 5)

/* bit definitions for MwmHints.decorations */
#define MWM_DECOR_ALL		(1L << 0)	/* [v] */
#define MWM_DECOR_BORDER	(1L << 1)	/* [v] */
#define MWM_DECOR_RESIZEH	(1L << 2)
#define MWM_DECOR_TITLE		(1L << 3)	/* [v] */
#define MWM_DECOR_MENU		(1L << 4)
#define MWM_DECOR_MINIMIZE	(1L << 5)
#define MWM_DECOR_MAXIMIZE	(1L << 6)

/* values for MwmHints.input_mode */
#define MWM_INPUT_MODELESS			0
#define MWM_INPUT_PRIMARY_APPLICATION_MODAL	1
#define MWM_INPUT_SYSTEM_MODAL			2
#define MWM_INPUT_FULL_APPLICATION_MODAL	3

/* bit definitions for MwmHints.status */
#define MWM_TEAROFF_WINDOW	(1L << 0)

/*
 * The above includes some contents from <Xm/MwmUtil.h>.
 *
 * Copyright (c) 1987-2012, The Open Group. All rights reserved.
 *
 * These libraries and programs are free software; you can
 * redistribute them and/or modify them under the terms of the GNU
 * Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 */

extern int GetMWMHints(Window w, MotifWmHints *mwmHints);

#endif /* include guard */
