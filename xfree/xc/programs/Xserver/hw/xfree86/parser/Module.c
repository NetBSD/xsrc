/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/Module.c,v 1.6 2000/10/20 14:59:02 alanh Exp $ */
/* 
 * 
 * Copyright (c) 1997  Metro Link Incorporated
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 * 
 */

/* View/edit this file with tab stops set to 4 */

#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"

extern LexRec val;

static xf86ConfigSymTabRec SubModuleTab[] =
{
	{ENDSUBSECTION, "endsubsection"},
	{OPTION, "option"},
	{-1, ""},
};

static xf86ConfigSymTabRec ModuleTab[] =
{
	{ENDSECTION, "endsection"},
	{LOAD, "load"},
	{LOAD_DRIVER, "loaddriver"},
	{SUBSECTION, "subsection"},
	{-1, ""},
};

#define CLEANUP xf86freeModules

XF86LoadPtr
xf86parseModuleSubSection (XF86LoadPtr head, char *name)
{
	parsePrologue (XF86LoadPtr, XF86LoadRec)

	ptr->load_name = name;
	ptr->load_type = XF86_LOAD_MODULE;
	ptr->load_opt  = NULL;
	ptr->list.next = NULL;

	while ((token = xf86getToken (SubModuleTab)) != ENDSUBSECTION)
	{
		switch (token)
		{
		case OPTION:
			{
				char *name;
				if ((token = xf86getToken (NULL)) != STRING)
				{
					xf86parseError (BAD_OPTION_MSG, NULL);
					xf86conffree(ptr);
					return NULL;
				}
				name = val.str;
				if ((token = xf86getToken (NULL)) == STRING)
				{
					ptr->load_opt = xf86addNewOption (ptr->load_opt,
														name, val.str);
				}
				else
				{
					ptr->load_opt = xf86addNewOption (ptr->load_opt,
														name, NULL);
					xf86unGetToken (token);
				}
			}
			break;
		case EOF_TOKEN:
			xf86parseError (UNEXPECTED_EOF_MSG, NULL);
			xf86conffree(ptr);
			return NULL;
			break;
		default:
			xf86parseError (INVALID_KEYWORD_MSG, xf86tokenString ());
			xf86conffree(ptr);
			return NULL;
			break;
		}

	}

	return ((XF86LoadPtr) xf86addListItem ((glp) head, (glp) ptr));
}

XF86ConfModulePtr
xf86parseModuleSection (void)
{
	parsePrologue (XF86ConfModulePtr, XF86ConfModuleRec)

	while ((token = xf86getToken (ModuleTab)) != ENDSECTION)
	{
		switch (token)
		{
		case LOAD:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Load");
			ptr->mod_load_lst =
				xf86addNewLoadDirective (ptr->mod_load_lst, val.str,
									 XF86_LOAD_MODULE, NULL);
			break;
		case LOAD_DRIVER:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "LoadDriver");
			ptr->mod_load_lst =
				xf86addNewLoadDirective (ptr->mod_load_lst, val.str,
									 XF86_LOAD_DRIVER, NULL);
			break;
		case SUBSECTION:
			if (xf86getToken (NULL) != STRING)
						Error (QUOTE_MSG, "SubSection");
			ptr->mod_load_lst =
				xf86parseModuleSubSection (ptr->mod_load_lst, val.str);
			break;
		case EOF_TOKEN:
			Error (UNEXPECTED_EOF_MSG, NULL);
			break;
		default:
			Error (INVALID_KEYWORD_MSG, xf86tokenString ());
			break;
		}
	}

#ifdef DEBUG
	printf ("Module section parsed\n");
#endif

	return ptr;
}

#undef CLEANUP

void
xf86printModuleSection (FILE * cf, XF86ConfModulePtr ptr)
{
	XF86LoadPtr lptr;

	if (ptr == NULL)
		return;

	for (lptr = ptr->mod_load_lst; lptr; lptr = lptr->list.next)
	{
		switch (lptr->load_type)
		{
		case XF86_LOAD_MODULE:
			if( lptr->load_opt == NULL )
				fprintf (cf, "\tLoad  \"%s\"\n", lptr->load_name);
			else
			{
				XF86OptionPtr optr;
				fprintf (cf, "\tSubSection \"%s\"\n", lptr->load_name);
				for(optr=lptr->load_opt;optr;optr=optr->list.next)
				{
					fprintf (cf, "\t\tOption \"%s\"", optr->opt_name);
					if( optr->opt_val)
						fprintf (cf, " \"%s\"", optr->opt_val);
					fprintf (cf, "\n" );
				}
				fprintf (cf, "\tEndSubSection\n");
			}
			break;
		case XF86_LOAD_DRIVER:
			fprintf (cf, "\tLoadDriver  \"%s\"\n", lptr->load_name);
			break;
		default:
			fprintf (cf, "#\tUnknown type  \"%s\"\n", lptr->load_name);
			break;
		}
	}
}

XF86LoadPtr
xf86addNewLoadDirective (XF86LoadPtr head, char *name, int type, XF86OptionPtr opts)
{
	XF86LoadPtr new;

	new = xf86confcalloc (1, sizeof (XF86LoadRec));
	new->load_name = name;
	new->load_type = type;
	new->load_opt  = opts;
	new->list.next = NULL;

	return ((XF86LoadPtr) xf86addListItem ((glp) head, (glp) new));
}

void
xf86freeModules (XF86ConfModulePtr ptr)
{
	XF86LoadPtr lptr;
	XF86LoadPtr prev;

	if (ptr == NULL)
		return;
	lptr = ptr->mod_load_lst;
	while (lptr)
	{
		TestFree (lptr->load_name);
		prev = lptr;
		lptr = lptr->list.next;
		xf86conffree (prev);
	}
	xf86conffree (ptr);
}
