/*
 * $XFree86: xc/lib/Xft/xftdir.c,v 1.3 2001/05/16 10:32:54 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include "xftint.h"

Bool
XftDirScan (XftFontSet *set, const char *dir, Bool force)
{
    DIR		    *d;
    struct dirent   *e;
    char	    *file;
    char	    *base;
    XftPattern	    *font;
    char	    *name;
    int		    count;
    Bool	    ret = True;
    int		    id;

    file = (char *) malloc (strlen (dir) + 1 + 256 + 1);
    if (!file)
	return False;

    strcpy (file, dir);
    strcat (file, "/");
    base = file + strlen (file);
    if (!force)
    {
	strcpy (base, "XftCache");
	
	if (XftFileCacheReadDir (set, file))
	{
	    free (file);
	    return True;
	}
    }
    
    d = opendir (dir);
    if (!d)
    {
	free (file);
	return False;
    }
    while (ret && (e = readdir (d)))
    {
	if (e->d_name[0] != '.')
	{
	    id = 0;
	    strcpy (base, e->d_name);
	    do
	    {
		if (!force)
		    name = XftFileCacheFind (file, id, &count);
		else
		    name = 0;
		if (name)
		{
		    font = XftNameParse (name);
		    if (font)
			XftPatternAddString (font, XFT_FILE, file);
		}
		else
		{
		    font = XftFreeTypeQuery (file, id, &count);
		    if (font && !force)
		    {
			char	unparse[8192];

			if (XftNameUnparse (font, unparse, sizeof (unparse)))
			{
			    (void) XftFileCacheUpdate (file, id, unparse);
			}
		    }
		}
		if (font)
		{
		    if (!XftFontSetAdd (set, font))
		    {
			XftPatternDestroy (font);
			font = 0;
			ret = False;
		    }
		}
		id++;
	    } while (font && ret && id < count);
	}
    }
    free (file);
    closedir (d);
    return ret;
}

Bool
XftDirSave (XftFontSet *set, const char *dir)
{
    char	    *file;
    char	    *base;
    Bool	    ret;
    
    file = (char *) malloc (strlen (dir) + 1 + 256 + 1);
    if (!file)
	return False;

    strcpy (file, dir);
    strcat (file, "/");
    base = file + strlen (file);
    strcpy (base, "XftCache");
    ret = XftFileCacheWriteDir (set, file);
    free (file);
    return ret;
}

