/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/Vendor.c,v 1.8 2000/11/30 20:45:34 paulo Exp $ */
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

static xf86ConfigSymTabRec VendorSubTab[] =
{
	{ENDSUBSECTION, "endsubsection"},
	{IDENTIFIER, "identifier"},
	{OPTION, "option"},
	{-1, ""},
};

#define CLEANUP xf86freeVendorSubList

XF86ConfVendSubPtr
xf86parseVendorSubSection (void)
{
	parsePrologue (XF86ConfVendSubPtr, XF86ConfVendSubRec)

	while ((token = xf86getToken (VendorSubTab)) != ENDSUBSECTION)
	{
		switch (token)
		{
		case IDENTIFIER:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Identifier");
			ptr->vs_identifier = val.str;
			break;
		case OPTION:
			{
				char *name;
				if ((token = xf86getToken (NULL)) != STRING)
					Error (BAD_OPTION_MSG, NULL);
				name = val.str;
				if ((token = xf86getToken (NULL)) == STRING)
				{
					ptr->vs_option_lst =
						xf86addNewOption (ptr->vs_option_lst,
							name, val.str);
				}
				else
				{
					ptr->vs_option_lst =
						xf86addNewOption (ptr->vs_option_lst,
								name, NULL);
					xf86unGetToken (token);
				}
			}
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
	printf ("Vendor subsection parsed\n");
#endif

	return ptr;
}

#undef CLEANUP

static xf86ConfigSymTabRec VendorTab[] =
{
	{COMMENT, "###"},
	{ENDSECTION, "endsection"},
	{IDENTIFIER, "identifier"},
	{OPTION, "option"},
	{SUBSECTION, "subsection"},
	{-1, ""},
};

#define CLEANUP xf86freeVendorList

XF86ConfVendorPtr
xf86parseVendorSection (void)
{
	int has_ident = FALSE;
	parsePrologue (XF86ConfVendorPtr, XF86ConfVendorRec)

	while ((token = xf86getToken (VendorTab)) != ENDSECTION)
	{
		switch (token)
		{
		case COMMENT:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "###");
			ptr->vnd_comment = val.str;
			break;
		case IDENTIFIER:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Identifier");
			ptr->vnd_identifier = val.str;
			has_ident = TRUE;
			break;
		case OPTION:
			{
				char *name;
				if ((token = xf86getToken (NULL)) != STRING)
					Error (BAD_OPTION_MSG, NULL);
				name = val.str;
				if ((token = xf86getToken (NULL)) == STRING)
				{
					ptr->vnd_option_lst = xf86addNewOption (ptr->vnd_option_lst,
														name, val.str);
				}
				else
				{
					ptr->vnd_option_lst = xf86addNewOption (ptr->vnd_option_lst,
														name, NULL);
					xf86unGetToken (token);
				}
			}
			break;
		case SUBSECTION:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "SubSection");
			{
				HANDLE_LIST (vnd_sub_lst, xf86parseVendorSubSection,
							XF86ConfVendSubPtr);
			}
			break;
		case EOF_TOKEN:
			Error (UNEXPECTED_EOF_MSG, NULL);
			break;
		default:
			Error (INVALID_KEYWORD_MSG, xf86tokenString ());
			break;
		}

	}

	if (!has_ident)
		Error (NO_IDENT_MSG, NULL);

#ifdef DEBUG
	printf ("Vendor section parsed\n");
#endif

	return ptr;
}

#undef CLEANUP

void
xf86printVendorSection (FILE * cf, XF86ConfVendorPtr ptr)
{
    XF86ConfVendSubPtr pptr;
	XF86OptionPtr optr;

	while (ptr)
	{
		fprintf (cf, "Section \"Vendor\"\n");
		if (ptr->vnd_comment)
			fprintf (cf, "\t###            \"%s\"\n", ptr->vnd_comment);
		if (ptr->vnd_identifier)
			fprintf (cf, "\tIdentifier     \"%s\"\n", ptr->vnd_identifier);

		for (optr = ptr->vnd_option_lst; optr; optr = optr->list.next)
		{
			fprintf (cf, "\tOption      \"%s\"", optr->opt_name);
			if (optr->opt_val)
				fprintf (cf, " \"%s\"", optr->opt_val);
			fprintf (cf, "\n");
		}
		for (pptr = ptr->vnd_sub_lst; pptr; pptr = pptr->list.next)
		{
			fprintf (cf, "\tSubSection \"Vendor\"\n");
			if (pptr->vs_identifier)
					fprintf (cf, "\t\tIdentifier \"%s\"\n", pptr->vs_identifier);
			for (optr = pptr->vs_option_lst; optr; optr = optr->list.next)
			{
				fprintf (cf, "\t\tOption     \"%s\"", optr->opt_name);
				if (optr->opt_val)
						fprintf (cf, " \"%s\"", optr->opt_val);
				fprintf (cf, "\n");
			}
			fprintf (cf, "\tEndSubSection\n");
		}
		fprintf (cf, "EndSection\n\n");
		ptr = ptr->list.next;
	}
}

void
xf86freeVendorList (XF86ConfVendorPtr p)
{
	if (p == NULL)
		return;
	xf86freeVendorSubList (p->vnd_sub_lst);
	TestFree (p->vnd_identifier);
	xf86optionListFree (p->vnd_option_lst);
	xf86conffree (p);
}

void
xf86freeVendorSubList (XF86ConfVendSubPtr ptr)
{
	XF86ConfVendSubPtr prev;

	while (ptr)
	{
		TestFree (ptr->vs_identifier);
		TestFree (ptr->vs_name);
		xf86optionListFree (ptr->vs_option_lst);
		prev = ptr;
		ptr = ptr->list.next;
		xf86conffree (prev);
	}
}

XF86ConfVendorPtr
xf86findVendor (const char *name, XF86ConfVendorPtr list)
{
    while (list)
    {
        if (xf86nameCompare (list->vnd_identifier, name) == 0)
            return (list);
        list = list->list.next;
    }
    return (NULL);
}

