/* $XFree86: xc/programs/Xserver/hw/xfree86/reconfig/strlist.h,v 3.2 1996/02/04 09:10:41 dawes Exp $ */





/* $XConsortium: strlist.h /main/4 1996/02/21 17:55:17 kaleb $ */

/* Used in the %union, therefore to be included in the scanner. */
typedef struct {
	int count;
	char **datap;
} string_list ;

typedef struct {
	int count;
	string_list **datap;
} string_list_list;
