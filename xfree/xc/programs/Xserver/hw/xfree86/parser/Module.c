/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/Module.c,v 1.18 2005/02/15 03:06:01 dawes Exp $ */
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
/*
 * Copyright (c) 1997-2005 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Copyright © 2003, 2004, 2005 David H. Dawes.
 * Copyright © 2003, 2004, 2005 X-Oz Technologies.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions, and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 * 
 *  3. The end-user documentation included with the redistribution,
 *     if any, must include the following acknowledgment: "This product
 *     includes software developed by X-Oz Technologies
 *     (http://www.x-oz.com/)."  Alternately, this acknowledgment may
 *     appear in the software itself, if and wherever such third-party
 *     acknowledgments normally appear.
 *
 *  4. Except as contained in this notice, the name of X-Oz
 *     Technologies shall not be used in advertising or otherwise to
 *     promote the sale, use or other dealings in this Software without
 *     prior written authorization from X-Oz Technologies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL X-OZ TECHNOLOGIES OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
	{IDENTIFIER, "identifier"},
	{LOAD, "load"},
	{LOAD_DRIVER, "loaddriver"},
	{SUBSECTION, "subsection"},
	{OPTION, "option"},
	{-1, ""},
};

#define CLEANUP xf86freeModulesList

XF86LoadPtr
xf86parseModuleSubSection (XF86LoadPtr head, char *name)
{
	int token;
	parsePrologue (XF86LoadPtr, XF86LoadRec)

	ptr->load_name = name;
	ptr->load_type = XF86_LOAD_MODULE;
	ptr->load_opt  = NULL;
	ptr->list.next = NULL;

	while ((token = xf86getToken (SubModuleTab)) != ENDSUBSECTION)
	{
		switch (token)
		{
		case COMMENT:
			ptr->load_comment = xf86addComment(ptr->load_comment, val.str);
			break;
		case OPTION:
			ptr->load_opt = xf86parseOption(ptr->load_opt);
			break;
		case EOF_TOKEN:
			xf86parseError (UNEXPECTED_EOF_MSG, NULL);
			xf86conffree(ptr);
			return NULL;
		default:
			xf86parseError (INVALID_KEYWORD_MSG, xf86tokenString ());
			xf86conffree(ptr);
			return NULL;
		}

	}

	return ((XF86LoadPtr) xf86addListItem ((glp) head, (glp) ptr));
}

XF86ConfModulePtr
xf86parseModuleSection (void)
{
	int has_ident = FALSE;
	int token;
	parsePrologue (XF86ConfModulePtr, XF86ConfModuleRec)

	while ((token = xf86getToken (ModuleTab)) != ENDSECTION)
	{
		switch (token)
		{
		case COMMENT:
			ptr->mod_comment = xf86addComment(ptr->mod_comment, val.str);
			break;
		case IDENTIFIER:
			if (xf86getSubToken (&(ptr->mod_comment)) != STRING)
				Error (QUOTE_MSG, "Identifier");
			if (has_ident)
				Error (MULTIPLE_MSG, "Identifier");
			ptr->mod_identifier = val.str;
			has_ident = TRUE;
			break;
		case LOAD:
			if (xf86getSubToken (&(ptr->mod_comment)) != STRING)
				Error (QUOTE_MSG, "Load");
			ptr->mod_load_lst =
				xf86addNewLoadDirective (ptr->mod_load_lst, val.str,
									 XF86_LOAD_MODULE, NULL);
			break;
		case LOAD_DRIVER:
			if (xf86getSubToken (&(ptr->mod_comment)) != STRING)
				Error (QUOTE_MSG, "LoadDriver");
			ptr->mod_load_lst =
				xf86addNewLoadDirective (ptr->mod_load_lst, val.str,
									 XF86_LOAD_DRIVER, NULL);
			break;
		case SUBSECTION:
			if (xf86getSubToken (&(ptr->mod_comment)) != STRING)
						Error (QUOTE_MSG, "SubSection");
			ptr->mod_load_lst =
				xf86parseModuleSubSection (ptr->mod_load_lst, val.str);
			break;
		case OPTION:
			ptr->mod_option_lst = xf86parseOption(ptr->mod_option_lst);
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

	while (ptr)
	{
		fprintf(cf, "Section \"Module\"\n");

		if (ptr->mod_comment)
			fprintf(cf, "%s", ptr->mod_comment);
		if (ptr->mod_identifier)
			fprintf (cf, "\tIdentifier  \"%s\"", ptr->mod_identifier);
		for (lptr = ptr->mod_load_lst; lptr; lptr = lptr->list.next)
		{
			switch (lptr->load_type)
			{
			case XF86_LOAD_MODULE:
				if( lptr->load_opt == NULL ) {
					fprintf (cf, "\tLoad  \"%s\"", lptr->load_name);
					if (lptr->load_comment)
						fprintf(cf, "%s", lptr->load_comment);
					else
						fputc('\n', cf);
				}
				else
				{
					fprintf (cf, "\tSubSection \"%s\"\n", lptr->load_name);
					if (lptr->load_comment)
						fprintf(cf, "%s", lptr->load_comment);
					xf86printOptionList(cf, lptr->load_opt, 2);
					fprintf (cf, "\tEndSubSection\n");
				}
				break;
			case XF86_LOAD_DRIVER:
				fprintf (cf, "\tLoadDriver  \"%s\"", lptr->load_name);
					if (lptr->load_comment)
						fprintf(cf, "%s", lptr->load_comment);
					else
						fputc('\n', cf);
				break;
#if 0
			default:
				fprintf (cf, "#\tUnknown type  \"%s\"\n", lptr->load_name);
				break;
#endif
			}
		}
		xf86printOptionList(cf, ptr->mod_option_lst, 1);
		fprintf(cf, "EndSection\n");
		ptr = ptr->list.next;
	}
}

XF86LoadPtr
xf86addNewLoadDirective (XF86LoadPtr head, char *name, int type, XF86OptionPtr opts)
{
	XF86LoadPtr new;
	int token;

	new = xf86confcalloc (1, sizeof (XF86LoadRec));
	new->load_name = name;
	new->load_type = type;
	new->load_opt  = opts;
	new->list.next = NULL;

	if ((token = xf86getToken(NULL)) == COMMENT)
		new->load_comment = xf86addComment(new->load_comment, val.str);
	else
		xf86unGetToken(token);

	return ((XF86LoadPtr) xf86addListItem ((glp) head, (glp) new));
}

void
xf86freeModulesList (XF86ConfModulePtr ptr)
{
	XF86LoadPtr lptr;
	XF86LoadPtr prev;
	XF86ConfModulePtr mprev;

	while (ptr) {
		lptr = ptr->mod_load_lst;
		while (lptr)
		{
			TestFree (lptr->load_name);
			TestFree (lptr->load_comment);
			prev = lptr;
			lptr = lptr->list.next;
			xf86conffree (prev);
		}
		TestFree (ptr->mod_comment);
		TestFree (ptr->mod_identifier);
		xf86optionListFree (ptr->mod_option_lst);
		mprev = ptr;
		ptr = ptr->list.next;
		xf86conffree (mprev);
	}
}
