/* $XConsortium: multiVis.h /main/3 1995/12/15 15:09:08 converse $ */
/** ------------------------------------------------------------------------
	This file contains routines for manipulating generic lists.
	Lists are implemented with a "harness".  In other words, each
	node in the list consists of two pointers, one to the data item
	and one to the next node in the list.  The head of the list is
	the same struct as each node, but the "item" ptr is used to point
	to the current member of the list (used by the first_in_list and
	next_in_list functions).

       (c)Copyright 1994 Hewlett-Packard Co.
       
                                RESTRICTED RIGHTS LEGEND
       Use, duplication, or disclosure by the U.S. Government is subject to
       restrictions as set forth in sub-paragraph (c)(1)(ii) of the Rights in
       Technical Data and Computer Software clause in DFARS 252.227-7013.
                                Hewlett-Packard Company
                                3000 Hanover Street
                                Palo Alto, CA 94304 U.S.A.
       Rights for non-DOD U.S. Government Departments and Agencies are as set
       forth in FAR 52.227-19(c)(1,2).
 ------------------------------------------------------------------------ **/

extern int GetMultiVisualRegions(
#if NeedFunctionPrototypes
    Display *, Window, int, int, unsigned int,
    unsigned int, int *, int *, XVisualInfo **, int *,
    OverlayInfo  **, int *, XVisualInfo ***, list_ptr *,
    list_ptr *, int *
#endif
); 

extern XImage *ReadAreaToImage(
#if NeedFunctionPrototypes
    Display *, Window, int, int, unsigned int,
    unsigned int, int, XVisualInfo *, int,
    OverlayInfo	*, int, XVisualInfo **, list_ptr,
    list_ptr, int, int
#endif
);

extern void initFakeVisual(
#if NeedFunctionPrototypes
    Visual *
#endif
);
