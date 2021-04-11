/*
 * TWM list handling external definitions
 *
 *
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * $XConsortium: list.h,v 1.12 90/09/14 14:54:42 converse Exp $
 *
 * 11-Apr-88 Tom LaStrange        Initial Version.
 *
 * Copyright 1992 Claude Lecommandeur.
 */

#ifndef _CTWM_LIST_H
#define _CTWM_LIST_H

struct name_list {
	name_list *next;            /* pointer to the next name */
	char      *name;            /* the name of the window */
	void      *ptr;             /* list dependent data */
};

void AddToList(name_list **list_head, const char *name, void *ptr);
void *LookInList(name_list *list_head, const char *name,
                 XClassHint *class);
void *LookInNameList(name_list *list_head, const char *name);
void *LookInListWin(name_list *list_head, TwmWindow *twin);
bool IsInList(name_list *list_head, TwmWindow *twin);
void *LookPatternInList(name_list *list_head, const char *name,
                        XClassHint *class);
void *LookPatternInNameList(name_list *list_head, const char *name);
bool GetColorFromList(name_list *list_head, char *name,
                      XClassHint *class, Pixel *ptr);
void FreeList(name_list **list);

bool match(const char *pattern, const char *string);

#endif /* _CTWM_LIST_H */

