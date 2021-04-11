/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
 */

/**********************************************************************
 *
 * $XConsortium: list.c,v 1.20 91/01/09 17:13:30 rws Exp $
 *
 * TWM code to deal with the name lists for the NoTitle list and
 * the AutoRaise list
 *
 * 11-Apr-88 Tom LaStrange        Initial Version.
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 *
 **********************************************************************/

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include "screen.h"
#include "list.h"
#include "util.h"
#include "parse.h"

#ifdef USE_SYS_REGEX
# include <regex.h>
#endif /* USE_SYS_REGEX */



/***********************************************************************
 *
 *  Procedure:
 *      AddToList - add a window name to the appropriate list
 *
 *  Inputs:
 *      list    - the address of the pointer to the head of a list
 *      name    - a pointer to the name of the window
 *      ptr     - pointer to list dependent data
 *
 *  Special Considerations
 *      If the list does not use the ptr value, a non-null value
 *      should be placed in it.  LookInList returns this ptr value
 *      and procedures calling LookInList will check for a non-null
 *      return value as an indication of success.
 *
 ***********************************************************************
 */

void AddToList(name_list **list_head, const char *name, void *ptr)
{
	name_list *nptr;

	if(!list_head) {
		return;        /* ignore empty inserts */
	}

	nptr = malloc(sizeof(name_list));
	if(nptr == NULL) {
		fprintf(stderr, "unable to allocate %lu bytes for name_list\n",
		        (unsigned long) sizeof(name_list));
		Done(0);
	}

	nptr->next = *list_head;
	nptr->name = strdup(name);
	nptr->ptr = (ptr == NULL) ? (char *)1 : ptr;
	*list_head = nptr;
}

/***********************************************************************
 *
 *  Procedure:
 *      LookInList - look through a list for a window name, or class
 *
 *  Returned Value:
 *      the ptr field of the list structure or NULL if the name
 *      or class was not found in the list
 *
 *  Inputs:
 *      list    - a pointer to the head of a list
 *      name    - a pointer to the name to look for
 *      class   - a pointer to the class to look for
 *
 ***********************************************************************
 */

void *LookInList(name_list *list_head, const char *name, XClassHint *class)
{
	name_list *nptr;

	/* look for the name first */
	for(nptr = list_head; nptr != NULL; nptr = nptr->next) {
		if(match(nptr->name, name)) {
			return (nptr->ptr);
		}
	}

	if(class) {
		/* look for the res_name next */
		for(nptr = list_head; nptr != NULL; nptr = nptr->next) {
			if(match(nptr->name, class->res_name)) {
				return (nptr->ptr);
			}
		}

		/* finally look for the res_class */
		for(nptr = list_head; nptr != NULL; nptr = nptr->next) {
			if(match(nptr->name, class->res_class)) {
				return (nptr->ptr);
			}
		}
	}
	return (NULL);
}

void *LookInNameList(name_list *list_head, const char *name)
{
	return (LookInList(list_head, name, NULL));
}

void *LookInListWin(name_list *list_head, TwmWindow *twin)
{
	return LookInList(list_head, twin->name, &(twin->class));
}

bool IsInList(name_list *list_head, TwmWindow *twin)
{
	return (bool)LookInList(list_head, twin->name, &(twin->class));
}

void *LookPatternInList(name_list *list_head, const char *name,
                        XClassHint *class)
{
	name_list *nptr;

	for(nptr = list_head; nptr != NULL; nptr = nptr->next)
		if(match(nptr->name, name)) {
			return (nptr->name);
		}

	if(class) {
		for(nptr = list_head; nptr != NULL; nptr = nptr->next)
			if(match(nptr->name, class->res_name)) {
				return (nptr->name);
			}

		for(nptr = list_head; nptr != NULL; nptr = nptr->next)
			if(match(nptr->name, class->res_class)) {
				return (nptr->name);
			}
	}
	return (NULL);
}

void *LookPatternInNameList(name_list *list_head, const char *name)
{
	return (LookPatternInList(list_head, name, NULL));
}

/***********************************************************************
 *
 *  Procedure:
 *      GetColorFromList - look through a list for a window name, or class
 *
 *  Returned Value:
 *      true  if the name was found
 *      false if the name was not found
 *
 *  Inputs:
 *      list    - a pointer to the head of a list
 *      name    - a pointer to the name to look for
 *      class   - a pointer to the class to look for
 *
 *  Outputs:
 *      ptr     - fill in the list value if the name was found
 *
 ***********************************************************************
 */

bool GetColorFromList(name_list *list_head, char *name,
                      XClassHint *class, Pixel *ptr)
{
	bool save;
	name_list *nptr;

	for(nptr = list_head; nptr != NULL; nptr = nptr->next)
		if(match(nptr->name, name)) {
			save = Scr->FirstTime;
			Scr->FirstTime = true;
			GetColor(Scr->Monochrome, ptr, nptr->ptr);
			Scr->FirstTime = save;
			return true;
		}

	if(class) {
		for(nptr = list_head; nptr != NULL; nptr = nptr->next)
			if(match(nptr->name, class->res_name)) {
				save = Scr->FirstTime;
				Scr->FirstTime = true;
				GetColor(Scr->Monochrome, ptr, nptr->ptr);
				Scr->FirstTime = save;
				return true;
			}

		for(nptr = list_head; nptr != NULL; nptr = nptr->next)
			if(match(nptr->name, class->res_class)) {
				save = Scr->FirstTime;
				Scr->FirstTime = true;
				GetColor(Scr->Monochrome, ptr, nptr->ptr);
				Scr->FirstTime = save;
				return true;
			}
	}
	return false;
}

/***********************************************************************
 *
 *  Procedure:
 *      FreeList - free up a list
 *
 ***********************************************************************
 */

void FreeList(name_list **list)
{
	name_list *nptr;
	name_list *tmp;

	for(nptr = *list; nptr != NULL;) {
		tmp = nptr->next;
		free(nptr->name);
		free(nptr);
		nptr = tmp;
	}
	*list = NULL;
}

#ifdef USE_SYS_REGEX

bool match(const char *pattern, const char *string)
{
	regex_t preg;
	int error;

	if((pattern == NULL) || (string == NULL)) {
		return false;
	}
	error = regcomp(&preg, pattern, REG_EXTENDED | REG_NOSUB);
	if(error != 0) {
		char buf [256];
		regerror(error, &preg, buf, sizeof buf);
		fprintf(stderr, "%s : %s\n", buf, pattern);
		return false;
	}
	error = regexec(&preg, string, 5, 0, 0);
	regfree(&preg);
	if(error == 0) {
		return true;
	}
	return false;
}

#else



int regex_match(const char *p, const char *t);
int regex_match_after_star(const char *p, const char *t);

#if 0                           /* appears not to be used anywhere */
static int is_pattern(char *p)
{
	while(*p) {
		switch(*p++) {
			case '?':
			case '*':
			case '[':
				return TRUE;
			case '\\':
				if(!*p++) {
					return FALSE;
				}
		}
	}
	return FALSE;
}
#endif

#define ABORT 2

int regex_match(const char *p, const char *t)
{
	char range_start, range_end;
	int invert;
	int member_match;
	int loop;

	for(; *p; p++, t++) {
		if(!*t) {
			return (*p == '*' && *++p == '\0') ? TRUE : ABORT;
		}
		switch(*p) {
			case '?':
				break;
			case '*':
				return regex_match_after_star(p, t);
			case '[': {
				p++;
				invert = FALSE;
				if(*p == '!' || *p == '^') {
					invert = TRUE;
					p++;
				}
				if(*p == ']') {
					return ABORT;
				}
				member_match = FALSE;
				loop = TRUE;
				while(loop) {
					if(*p == ']') {
						loop = FALSE;
						continue;
					}
					if(*p == '\\') {
						range_start = range_end = *++p;
					}
					else {
						range_start = range_end = *p;
					}
					if(!range_start) {
						return ABORT;
					}
					if(*++p == '-') {
						range_end = *++p;
						if(range_end == '\0' || range_end == ']') {
							return ABORT;
						}
						if(range_end == '\\') {
							range_end = *++p;
						}
						p++;
					}
					if(range_start < range_end) {
						if(*t >= range_start && *t <= range_end) {
							member_match = TRUE;
							loop = FALSE;
						}
					}
					else {
						if(*t >= range_end && *t <= range_start) {
							member_match = TRUE;
							loop = FALSE;
						}
					}
				}
				if((invert && member_match) || !(invert || member_match)) {
					return (FALSE);
				}
				if(member_match) {
					while(*p != ']') {
						if(!*p) {
							return (ABORT);
						}
						if(*p == '\\') {
							p++;
						}
						p++;
					}
				}
				break;
			}
			case '\\':
				p++;

			default:
				if(*p != *t) {
					return (FALSE);
				}
		}
	}
	return (!*t);
}

int regex_match_after_star(const char *p, const char *t)
{
	int mat;
	int nextp;

	while((*p == '?') || (*p == '*')) {
		if(*p == '?') {
			if(!*t++) {
				return ABORT;
			}
		}
		p++;
	}
	if(!*p) {
		return TRUE;
	}

	nextp = *p;
	if(nextp == '\\') {
		nextp = p[1];
	}

	mat = FALSE;
	while(mat == FALSE) {
		if(nextp == *t || nextp == '[') {
			mat = regex_match(p, t);
		}
		if(!*t++) {
			mat = ABORT;
		}
	}
	return (mat);
}

int match(const char *p, const char *t)
{
	if((p == NULL) || (t == NULL)) {
		return (FALSE);
	}
	return ((regex_match(p, t) == TRUE) ? TRUE : FALSE);
}

#endif




