/* DRI.c -- DRI Section in XF86Config file
 * Created: Fri Mar 19 08:40:22 1999 by faith@precisioninsight.com
 * Revised: Thu Jun 17 16:08:05 1999 by faith@precisioninsight.com
 *
 * Copyright 1999 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 * 
 * $XFree86: xc/programs/Xserver/hw/xfree86/parser/DRI.c,v 1.15 2005/01/26 05:31:50 dawes Exp $
 * 
 */
/*
 * Copyright (c) 2003-2005 by The XFree86 Project, Inc.
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
 * Copyright © 2005 X-Oz Technologies.
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

#include "xf86Parser.h"
#include "xf86tokens.h"
#include "Configint.h"

extern LexRec val;

static xf86ConfigSymTabRec DRITab[] =
{
    {ENDSECTION, "endsection"},
    {IDENTIFIER, "identifier"},
    {GROUP,      "group"},
    {BUFFERS,    "buffers"},
    {MODE,       "mode"},
    {OPTION,     "option"},
    {-1,         ""},
};

#define CLEANUP xf86freeBuffersList

XF86ConfBuffersPtr
xf86parseBuffers (void)
{
    int token;
    parsePrologue (XF86ConfBuffersPtr, XF86ConfBuffersRec)

    if (xf86getSubToken (&(ptr->buf_comment)) != NUMBER)
	Error ("Buffers count expected", NULL);
    ptr->buf_count = val.num;

    if (xf86getSubToken (&(ptr->buf_comment)) != NUMBER)
	Error ("Buffers size expected", NULL);
    ptr->buf_size = val.num;

    if ((token = xf86getSubToken (&(ptr->buf_comment))) == STRING) {
	ptr->buf_flags = val.str;
    } else
	xf86unGetToken(token);
    if ((token = xf86getToken (NULL)) == COMMENT)
	ptr->buf_comment = xf86addComment(ptr->buf_comment, val.str);
    else
	    xf86unGetToken(token);

#ifdef DEBUG
    printf ("Buffers parsed\n");
#endif

    return ptr;
}

#undef CLEANUP
	
#define CLEANUP xf86freeDRIList

XF86ConfDRIPtr
xf86parseDRISection (void)
{
    int has_ident = FALSE;
    int token;
    parsePrologue (XF86ConfDRIPtr, XF86ConfDRIRec);

    /* Zero is a valid value for this. */
    ptr->dri_group = -1;
    while ((token = xf86getToken (DRITab)) != ENDSECTION) {
	switch (token)
	 {
	    case IDENTIFIER:
		if ((token = xf86getSubToken (&(ptr->dri_comment))) != STRING)
		    Error (QUOTE_MSG, "Identifier");
		if (has_ident == TRUE)
		    Error (MULTIPLE_MSG, "Identifier");
		ptr->dri_identifier = val.str;
		has_ident = TRUE;
		break;
	    case GROUP:
		if ((token = xf86getSubToken (&(ptr->dri_comment))) == STRING)
		    ptr->dri_group_name = val.str;
		else if (token == NUMBER)
		    ptr->dri_group = val.num;
		else
		    Error (GROUP_MSG, NULL);
		break;
	    case MODE:
		if (xf86getSubToken (&(ptr->dri_comment)) != NUMBER)
		    Error (NUMBER_MSG, "Mode");
		ptr->dri_mode = val.num;
		break;
	    case BUFFERS:
		HANDLE_LIST (dri_buffers_lst, xf86parseBuffers,
			     XF86ConfBuffersPtr);
		break;
	    case EOF_TOKEN:
		Error (UNEXPECTED_EOF_MSG, NULL);
		break;
	    case COMMENT:
		ptr->dri_comment = xf86addComment(ptr->dri_comment, val.str);
		break;
	    case OPTION:
		ptr->dri_option_lst = xf86parseOption(ptr->dri_option_lst);
		break;
	    default:
		Error (INVALID_KEYWORD_MSG, xf86tokenString ());
		break;
	    }
    }
    
#ifdef DEBUG
    ErrorF("DRI section parsed\n");
#endif
    
    return ptr;
}

#undef CLEANUP

void
xf86printDRISection (FILE * cf, XF86ConfDRIPtr ptr)
{
    XF86ConfBuffersPtr bufs;
    
    while (ptr) {
	fprintf (cf, "Section \"DRI\"\n");
	if (ptr->dri_comment)
	    fprintf (cf, "%s", ptr->dri_comment);
	if (ptr->dri_identifier)
	    fprintf (cf, "\tIdentifier   \"%s\"\n", ptr->dri_identifier);
	if (ptr->dri_group_name)
	    fprintf (cf, "\tGroup        \"%s\"\n", ptr->dri_group_name);
	else if (ptr->dri_group >= 0)
	    fprintf (cf, "\tGroup        %d\n", ptr->dri_group);
	if (ptr->dri_mode)
	    fprintf (cf, "\tMode         0%o\n", ptr->dri_mode);
	for (bufs = ptr->dri_buffers_lst; bufs; bufs = bufs->list.next) {
	    fprintf (cf, "\tBuffers      %d %d",
		 bufs->buf_count, bufs->buf_size);
	    if (bufs->buf_flags) fprintf (cf, " \"%s\"", bufs->buf_flags);
	    if (bufs->buf_comment)
		fprintf(cf, "%s", bufs->buf_comment);
	    else
		fprintf (cf, "\n");
	}
	xf86printOptionList(cf, ptr->dri_option_lst, 1);
	fprintf (cf, "EndSection\n\n");
    }
}

void
xf86freeDRIList (XF86ConfDRIPtr ptr)
{
    XF86ConfDRIPtr prev;

    while (ptr) {
	xf86freeBuffersList (ptr->dri_buffers_lst);
	TestFree (ptr->dri_comment);
	TestFree (ptr->dri_group_name);
	TestFree (ptr->dri_identifier);
	xf86optionListFree(ptr->dri_option_lst);
	prev = ptr;
	ptr = ptr->list.next;
	xf86conffree (prev);
    }
}

void
xf86freeBuffersList (XF86ConfBuffersPtr ptr)
{
    XF86ConfBuffersPtr prev;

    while (ptr) {
	TestFree (ptr->buf_flags);
	TestFree (ptr->buf_comment);
	prev = ptr;
	ptr  = ptr->list.next;
	xf86conffree (prev);
    }
}

