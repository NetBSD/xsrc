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
/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/Video.c,v 1.6 2000/11/30 20:45:34 paulo Exp $ */

/* View/edit this file with tab stops set to 4 */

#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"

extern LexRec val;

static xf86ConfigSymTabRec VideoPortTab[] =
{
	{ENDSUBSECTION, "endsubsection"},
	{IDENTIFIER, "identifier"},
	{OPTION, "option"},
	{-1, ""},
};

#define CLEANUP xf86freeVideoPortList

XF86ConfVideoPortPtr
xf86parseVideoPortSubSection (void)
{
	parsePrologue (XF86ConfVideoPortPtr, XF86ConfVideoPortRec)

	while ((token = xf86getToken (VideoPortTab)) != ENDSUBSECTION)
	{
		switch (token)
		{
		case IDENTIFIER:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Identifier");
			ptr->vp_identifier = val.str;
			break;
		case OPTION:
			{
				char *name;
				if ((token = xf86getToken (NULL)) != STRING)
					Error (BAD_OPTION_MSG, NULL);
				name = val.str;
				if ((token = xf86getToken (NULL)) == STRING)
				{
					ptr->vp_option_lst =
					    xf86addNewOption (ptr->vp_option_lst,
							  name, val.str);
				}
				else
				{
					ptr->vp_option_lst =
					    xf86addNewOption (ptr->vp_option_lst,
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
	printf ("VideoPort subsection parsed\n");
#endif

	return ptr;
}

#undef CLEANUP

static xf86ConfigSymTabRec VideoAdaptorTab[] =
{
	{ENDSECTION, "endsection"},
	{IDENTIFIER, "identifier"},
	{VENDOR, "vendorname"},
	{BOARD, "boardname"},
	{BUSID, "busid"},
	{DRIVER, "driver"},
	{OPTION, "option"},
	{SUBSECTION, "subsection"},
	{-1, ""},
};

#define CLEANUP xf86freeVideoAdaptorList

XF86ConfVideoAdaptorPtr
xf86parseVideoAdaptorSection (void)
{
	int has_ident = FALSE;

	parsePrologue (XF86ConfVideoAdaptorPtr, XF86ConfVideoAdaptorRec)

	while ((token = xf86getToken (VideoAdaptorTab)) != ENDSECTION)
	{
		switch (token)
		{
		case IDENTIFIER:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Identifier");
			ptr->va_identifier = val.str;
			has_ident = TRUE;
			break;
		case VENDOR:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Vendor");
			ptr->va_vendor = val.str;
			break;
		case BOARD:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Board");
			ptr->va_board = val.str;
			break;
		case BUSID:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "BusID");
			ptr->va_busid = val.str;
			break;
		case DRIVER:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Driver");
			ptr->va_driver = val.str;
			break;
		case OPTION:
			{
				char *name;
				if ((token = xf86getToken (NULL)) != STRING)
					Error (BAD_OPTION_MSG, NULL);
				name = val.str;
				if ((token = xf86getToken (NULL)) == STRING)
				{
					ptr->va_option_lst = xf86addNewOption (ptr->va_option_lst,
									   name, val.str);
				}
				else
				{
					ptr->va_option_lst = xf86addNewOption (ptr->va_option_lst,
									   name, NULL);
					xf86unGetToken (token);
				}
			}
			break;
		case SUBSECTION:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "SubSection");
			{
				HANDLE_LIST (va_port_lst, xf86parseVideoPortSubSection,
							 XF86ConfVideoPortPtr);
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
	printf ("VideoAdaptor section parsed\n");
#endif

	return ptr;
}

void
xf86printVideoAdaptorSection (FILE * cf, XF86ConfVideoAdaptorPtr ptr)
{
	XF86ConfVideoPortPtr pptr;
	XF86OptionPtr optr;

	while (ptr)
	{
		fprintf (cf, "Section \"VideoAdaptor\"\n");
		if (ptr->va_identifier)
			fprintf (cf, "\tIdentifier  \"%s\"\n", ptr->va_identifier);
		if (ptr->va_vendor)
			fprintf (cf, "\tVendorName  \"%s\"\n", ptr->va_vendor);
		if (ptr->va_board)
			fprintf (cf, "\tBoardName   \"%s\"\n", ptr->va_board);
		if (ptr->va_busid)
			fprintf (cf, "\tBusID       \"%s\"\n", ptr->va_busid);
		if (ptr->va_driver)
			fprintf (cf, "\tDriver      \"%s\"\n", ptr->va_driver);
		for (optr = ptr->va_option_lst; optr; optr = optr->list.next)
		{
			fprintf (cf, "\tOption      \"%s\"", optr->opt_name);
			if (optr->opt_val)
				fprintf (cf, " \"%s\"", optr->opt_val);
			fprintf (cf, "\n");
		}
		for (pptr = ptr->va_port_lst; pptr; pptr = pptr->list.next)
		{
			fprintf (cf, "\tSubSection \"VideoPort\"\n");
			if (pptr->vp_identifier)
				fprintf (cf, "\t\tIdentifier \"%s\"\n", pptr->vp_identifier);
			for (optr = pptr->vp_option_lst; optr; optr = optr->list.next)
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
xf86freeVideoAdaptorList (XF86ConfVideoAdaptorPtr ptr)
{
	XF86ConfVideoAdaptorPtr prev;

	while (ptr)
	{
		TestFree (ptr->va_identifier);
		TestFree (ptr->va_vendor);
		TestFree (ptr->va_board);
		TestFree (ptr->va_busid);
		TestFree (ptr->va_driver);
		TestFree (ptr->va_fwdref);
		xf86freeVideoPortList (ptr->va_port_lst);
		xf86optionListFree (ptr->va_option_lst);
		prev = ptr;
		ptr = ptr->list.next;
		xf86conffree (prev);
	}
}

void
xf86freeVideoPortList (XF86ConfVideoPortPtr ptr)
{
	XF86ConfVideoPortPtr prev;

	while (ptr)
	{
		TestFree (ptr->vp_identifier);
		xf86optionListFree (ptr->vp_option_lst);
		prev = ptr;
		ptr = ptr->list.next;
		xf86conffree (prev);
	}
}

XF86ConfVideoAdaptorPtr
xf86findVideoAdaptor (const char *ident, XF86ConfVideoAdaptorPtr p)
{
	while (p)
	{
		if (xf86nameCompare (ident, p->va_identifier) == 0)
			return (p);

		p = p->list.next;
	}
	return (NULL);
}
