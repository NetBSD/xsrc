/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/Input.c,v 1.5 2000/10/20 14:59:02 alanh Exp $ */
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

static
xf86ConfigSymTabRec InputTab[] =
{
	{COMMENT, "###"},
	{ENDSECTION, "endsection"},
	{IDENTIFIER, "identifier"},
	{OPTION, "option"},
	{DRIVER, "driver"},
	{-1, ""},
};

#define CLEANUP xf86freeInputList

XF86ConfInputPtr
xf86parseInputSection (void)
{
	int has_ident = FALSE;
	parsePrologue (XF86ConfInputPtr, XF86ConfInputRec)

	while ((token = xf86getToken (InputTab)) != ENDSECTION)
	{
		switch (token)
		{
		case COMMENT:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "###");
			ptr->inp_comment = val.str;
			break;
		case IDENTIFIER:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Identifier");
			ptr->inp_identifier = val.str;
			has_ident = TRUE;
			break;
		case DRIVER:
			if (xf86getToken (NULL) != STRING)
				Error (QUOTE_MSG, "Driver");
			ptr->inp_driver = val.str;
			break;
		case OPTION:
			{
				char *name;
				if ((token = xf86getToken (NULL)) != STRING)
					Error (BAD_OPTION_MSG, NULL);
				name = val.str;
				if ((token = xf86getToken (NULL)) == STRING)
				{
					ptr->inp_option_lst = xf86addNewOption (ptr->inp_option_lst,
														name, val.str);
				}
				else
				{
					ptr->inp_option_lst = xf86addNewOption (ptr->inp_option_lst,
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

	if (!has_ident)
		Error (NO_IDENT_MSG, NULL);

#ifdef DEBUG
	printf ("InputDevice section parsed\n");
#endif

	return ptr;
}

#undef CLEANUP

void
xf86printInputSection (FILE * cf, XF86ConfInputPtr ptr)
{
	XF86OptionPtr optr;

	while (ptr)
	{
		fprintf (cf, "Section \"InputDevice\"\n");
		if (ptr->inp_comment)
			fprintf (cf, "\t###         \"%s\"\n", ptr->inp_comment);
		if (ptr->inp_identifier)
			fprintf (cf, "\tIdentifier  \"%s\"\n", ptr->inp_identifier);
		if (ptr->inp_driver)
			fprintf (cf, "\tDriver      \"%s\"\n", ptr->inp_driver);
		for (optr = ptr->inp_option_lst; optr; optr = optr->list.next)
		{
			fprintf (cf, "\tOption      \"%s\"", optr->opt_name);
			if (optr->opt_val)
				fprintf (cf, " \"%s\"", optr->opt_val);
			fprintf (cf, "\n");
		}
		fprintf (cf, "EndSection\n\n");
		ptr = ptr->list.next;
	}
}

void
xf86freeInputList (XF86ConfInputPtr ptr)
{
	XF86ConfInputPtr prev;

	while (ptr)
	{
		TestFree (ptr->inp_identifier);
		TestFree (ptr->inp_driver);
		xf86optionListFree (ptr->inp_option_lst);

		prev = ptr;
		ptr = ptr->list.next;
		xf86conffree (prev);
	}
}

int
xf86validateInput (XF86ConfigPtr p)
{
  XF86ConfInputPtr input = p->conf_input_lst;

#if 0 /* Enable this later */
  if (!input) {
    xf86validationError ("At least one InputDevice section is required.");
    return (FALSE);
  }
#endif

  while (input) {
    if (!input->inp_driver) {
      xf86validationError (UNDEFINED_INPUTDRIVER_MSG, input->inp_identifier);
      return (FALSE);
    }
  input = input->list.next;
  }
  return (TRUE);
}

XF86ConfInputPtr
xf86findInput (const char *ident, XF86ConfInputPtr p)
{
	while (p)
	{
		if (xf86nameCompare (ident, p->inp_identifier) == 0)
			return (p);

		p = p->list.next;
	}
	return (NULL);
}

XF86ConfInputPtr
xf86findInputByDriver (const char *driver, XF86ConfInputPtr p)
{
	while (p)
	{
		if (xf86nameCompare (driver, p->inp_driver) == 0)
			return (p);

		p = p->list.next;
	}
	return (NULL);
}

