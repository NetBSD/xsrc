/* $XConsortium: list.h /main/3 1995/12/15 14:51:15 converse $ */
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
    -------------------------------------------------------------------- **/

#ifndef LIST_DEF
#define LIST_DEF

#define LESS	-1
#define EQUAL	0
#define GREATER	1
#define DUP_WHOLE_LIST	0
#define START_AT_CURR	1

typedef struct _list_item {
    struct _list_item *next;
    union {
	void *item;		 /* in normal list node, pts to data */
	struct _list_item *curr; /* in list head, pts to curr for 1st, next */
    } ptr;
} list, list_item, *list_ptr;

typedef void (*DESTRUCT_FUNC_PTR)(
#if NeedFunctionPrototypes
void *
#endif
);

void zero_list( 
#if NeedFunctionPrototypes
          list_ptr 
#endif
    );
int add_to_list (
#if NeedFunctionPrototypes
          list_ptr , void *
#endif
    );
list_ptr new_list (
#if NeedFunctionPrototypes
          void
#endif
    );
list_ptr dup_list_head (
#if NeedFunctionPrototypes
          list_ptr , int 
#endif
    );
unsigned int list_length( 
#if NeedFunctionPrototypes
          list_ptr 
#endif
    );
void *delete_from_list (
#if NeedFunctionPrototypes
          list_ptr , void *
#endif
    );
void delete_list( 
#if NeedFunctionPrototypes
          list_ptr , int 
#endif
    );
void delete_list_destroying (
#if NeedFunctionPrototypes
          list_ptr , DESTRUCT_FUNC_PTR
#endif
    );
void *first_in_list (
#if NeedFunctionPrototypes
          list_ptr 
#endif
    );
void *next_in_list (
#if NeedFunctionPrototypes
          list_ptr 
#endif
    );
int list_is_empty (
#if NeedFunctionPrototypes
          list_ptr 
#endif
    );

#endif
