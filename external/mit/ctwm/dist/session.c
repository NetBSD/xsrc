/*****************************************************************************/
/**       Copyright 1988 by Evans & Sutherland Computer Corporation,        **/
/**                          Salt Lake City, Utah                           **/
/**  Portions Copyright 1989 by the Massachusetts Institute of Technology   **/
/**                        Cambridge, Massachusetts                         **/
/**                                                                         **/
/**                           All Rights Reserved                           **/
/**                                                                         **/
/**    Permission to use, copy, modify, and distribute this software and    **/
/**    its documentation  for  any  purpose  and  without  fee is hereby    **/
/**    granted, provided that the above copyright notice appear  in  all    **/
/**    copies and that both  that  copyright  notice  and  this  permis-    **/
/**    sion  notice appear in supporting  documentation,  and  that  the    **/
/**    names of Evans & Sutherland and M.I.T. not be used in advertising    **/
/**    in publicity pertaining to distribution of the  software  without    **/
/**    specific, written prior permission.                                  **/
/**                                                                         **/
/**    EVANS & SUTHERLAND AND M.I.T. DISCLAIM ALL WARRANTIES WITH REGARD    **/
/**    TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES  OF  MERCHANT-    **/
/**    ABILITY  AND  FITNESS,  IN  NO  EVENT SHALL EVANS & SUTHERLAND OR    **/
/**    M.I.T. BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL  DAM-    **/
/**    AGES OR  ANY DAMAGES WHATSOEVER  RESULTING FROM LOSS OF USE, DATA    **/
/**    OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER    **/
/**    TORTIOUS ACTION, ARISING OUT OF OR IN  CONNECTION  WITH  THE  USE    **/
/**    OR PERFORMANCE OF THIS SOFTWARE.                                     **/
/*****************************************************************************/
/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */

/*********************************************************************
 *
 * This module has been modified by 
 * Matthew McNeill (21 Mar 1997) - University of Durham (UK)
 * for the support of the X Session Management Protocol
 *
 ********************************************************************* 
 *
 * Copyright (c) 1996-1997 The University of Durham, UK -
 * Department of Computer Science.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * licence or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation, provided that usage, copying, modification
 * or distribution is not for direct commercial advantage and that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software. To use, copy, modify or distribute this
 * software and its documentation otherwise requires a fee and/or specific
 * permission.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF DURHAM BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * DURHAM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF DURHAM SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF DURHAM HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 *
 ********************************************************************/

/**********************************************************************
 *
 * $XConsortium: session.c,v 1.18 95/01/04 22:28:37 mor Exp $ 
 *
 * Session support for the X11R6 XSMP (X Session Management Protocol) 
 *
 * 95/01/04 Ralph Mor, X Consortium        Initial Version.
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 21 Mar 1997  Matthew McNeill, Durham University  Modified Version.
 *
 **********************************************************************/

#include <X11/Xos.h>

#ifndef X_NOT_POSIX
#ifdef _POSIX_SOURCE
#include <limits.h>
#else
#define _POSIX_SOURCE
#include <limits.h>
#undef _POSIX_SOURCE
#endif
#endif /* X_NOT_POSIX */
#ifndef PATH_MAX
#include <sys/param.h>
#ifndef PATH_MAX
#ifdef MAXPATHLEN
#define PATH_MAX MAXPATHLEN
#else
#define PATH_MAX 1024
#endif
#endif
#endif /* PATH_MAX */

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include "twm.h"
#include "icons.h"
#include "screen.h"
#include "session.h"

SmcConn smcConn = NULL;
XtInputId iceInputId;
char *twm_clientId;
TWMWinConfigEntry *winConfigHead = NULL;
Bool gotFirstSave = 0;
Bool sent_save_done = 0;

#define SAVEFILE_VERSION 2
 

/*===[ Get Client SM_CLIENT_ID ]=============================================*/

char *GetClientID (Window window)
/* This function returns the value of the session manager client ID property
 * given a valid window handle. If no such property exists on a window then
 * null is returned 
 */
{
    char *client_id = NULL;
    Window client_leader;
    XTextProperty tp;
    Atom actual_type;
    int actual_format;
    unsigned long nitems;
    unsigned long bytes_after;
    Window *prop = NULL;

    if (XGetWindowProperty (dpy, window, _XA_WM_CLIENT_LEADER,
	0L, 1L, False, AnyPropertyType,	&actual_type, &actual_format,
	&nitems, &bytes_after, (unsigned char **)&prop) == Success)
    {
	if (actual_type == XA_WINDOW && actual_format == 32 &&
	    nitems == 1 && bytes_after == 0)
	{
	    client_leader = *prop;

	    if (XGetTextProperty (dpy, client_leader, &tp, _XA_SM_CLIENT_ID))
	    {
		if (tp.encoding == XA_STRING &&
		    tp.format == 8 && tp.nitems != 0)
		    client_id = (char *) tp.value;
	    }
	}

	if (prop)
	    XFree (prop);
    }
    
    return client_id;
}

/*===[ Get Window Role ]=====================================================*/

char *GetWindowRole (Window window)
/* this function returns the WM_WINDOW_ROLE property of a window
 */
{
    XTextProperty tp;

    if (XGetTextProperty (dpy, window, &tp, _XA_WM_WINDOW_ROLE))
    {
	if (tp.encoding == XA_STRING && tp.format == 8 && tp.nitems != 0)
	    return ((char *) tp.value);
    }

    return NULL;
}

/*===[ Various file write procedures ]=======================================*/

static int write_byte (FILE *file, unsigned char b)
{
    if (fwrite ((char *) &b, 1, 1, file) != 1)
	return 0;
    return 1;
}

/*---------------------------------------------------------------------------*/

static int write_ushort (FILE *file, unsigned short s)
{
    unsigned char   file_short[2];

    file_short[0] = (s & (unsigned)0xff00) >> 8;
    file_short[1] = s & 0xff;
    if (fwrite ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return 0;
    return 1;
}

/*---------------------------------------------------------------------------*/

static int write_short (FILE *file, short s)
{
    unsigned char   file_short[2];

    file_short[0] = (s & (unsigned)0xff00) >> 8;
    file_short[1] = s & 0xff;
    if (fwrite ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return 0;
    return 1;
}

/*---------------------------------------------------------------------------*
 * Matthew McNeill Feb 1997 - required to save the occupation state as an
 *                            integer.
 */

static int write_int (FILE *file, int i)
{
    unsigned char   file_int[4];

    file_int[0] = (i & (unsigned)0xff000000) >> 24;
    file_int[1] = (i & (unsigned)0x00ff0000) >> 16;
    file_int[2] = (i & (unsigned)0x0000ff00) >> 8;
    file_int[3] = (i & (unsigned)0x000000ff);
    if (fwrite ((char *) file_int, (int) sizeof (file_int), 1, file) != 1)
	return 0;
    return 1;
}

/*---------------------------------------------------------------------------*/

static int write_counted_string (FILE *file, char *string)
{
    if (string)
    {
	unsigned char count = strlen (string);

	if (write_byte (file, count) == 0)
	    return 0;
	if (fwrite (string, (int) sizeof (char), (int) count, file) != count)
	    return 0;
    }
    else
    {
	if (write_byte (file, 0) == 0)
	    return 0;
    }

    return 1;
}

/*===[ various file read procedures ]========================================*/

static int read_byte (FILE *file, unsigned char *bp)
{
    if (fread ((char *) bp, 1, 1, file) != 1)
	return 0;
    return 1;
}

/*---------------------------------------------------------------------------*/

static int read_ushort (FILE *file, unsigned short *shortp)
{
    unsigned char   file_short[2];

    if (fread ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return 0;
    *shortp = file_short[0] * 256 + file_short[1];
    return 1;
}

/*---------------------------------------------------------------------------*/

static int read_short (FILE *file, short *shortp)
{
    unsigned char   file_short[2];

    if (fread ((char *) file_short, (int) sizeof (file_short), 1, file) != 1)
	return 0;
    *shortp = file_short[0] * 256 + file_short[1];
    return 1;
}

/*---------------------------------------------------------------------------*
 * Matthew McNeill Feb 1997 - required to save the occupation state as an
 *                            integer.
 */

static int read_int (FILE *file, int *intp)
{
    unsigned char   file_int[4];

    if (fread ((char *) file_int, (int) sizeof (file_int), 1, file) != 1)
	return 0;
    *intp =  (((int) file_int[0]) << 24) & 0xff000000; 
    *intp += (((int) file_int[1]) << 16) & 0x00ff0000;
    *intp += (((int) file_int[2]) << 8)  & 0x0000ff00;
    *intp += ((int) file_int[3]);
    return 1;
}

/*---------------------------------------------------------------------------*/

static int read_counted_string (FILE *file, char **stringp)
{
    unsigned char  len;
    char	   *data;

    if (read_byte (file, &len) == 0)
	return 0;
    if (len == 0) {
	data = 0;
    } else {
	data = malloc ((unsigned) len + 1);
	if (!data)
	    return 0;
	if (fread (data, (int) sizeof (char), (int) len, file) != len) {
	    free (data);
	    return 0;
	}
	data[len] = '\0';
    }
    *stringp = data;
    return 1;
}

/*===[ Definition of a window config entry ]===================================
 *
 * An entry in the saved window config file looks like this:
 *
 * FIELD				BYTES
 * -----                                ----
 * SM_CLIENT_ID ID len			1	       (may be 0)
 * SM_CLIENT_ID				LIST of bytes  (may be NULL)
 *
 * WM_WINDOW_ROLE length		1	       (may be 0)
 * WM_WINDOW_ROLE			LIST of bytes  (may be NULL)
 *
 * if no WM_WINDOW_ROLE (length = 0)
 *
 *   WM_CLASS "res name" length		1
 *   WM_CLASS "res name"		LIST of bytes
 *   WM_CLASS "res class" length        1
 *   WM_CLASS "res class"               LIST of bytes
 *   WM_NAME length			1		(0 if name changed)
 *   WM_NAME				LIST of bytes
 *   WM_COMMAND arg count		1      		(0 if no SM_CLIENT_ID)
 *   For each arg in WM_COMMAND
 *      arg length			1
 *      arg				LIST of bytes
 *
 * Iconified bool			1
 * Icon info present bool		1
 *
 * if icon info present
 *	icon x				2
 *	icon y				2
 *
 * Geom x				2
 * Geom y				2
 * Geom width				2
 * Geom height				2
 *
 * Width ever changed by user		1
 * Height ever changed by user		1
 *
 * ------------------[ Matthew McNeill Feb 1997 ]----------------------------
 *
 * Workspace Occupation                 4
 *
 */


/*===[ Write Window Config Entry to file ]===================================*/

int WriteWinConfigEntry (FILE *configFile, TwmWindow *theWindow,
			 char *clientId, char *windowRole)
/* this function writes a window configuration entry of a given window to
 * the given configuration file
 */
{
    char **wm_command;
    int wm_command_count, i;

    /* ...unless the config file says otherwise. */
    if (LookInList (Scr == NULL ? ScreenList [0]->DontSave : Scr->DontSave,
		    theWindow->full_name, &theWindow->class))
	return 1;
        
    if (!write_counted_string (configFile, clientId))
	return 0;

    if (!write_counted_string (configFile, windowRole))
	return 0;

    if (!windowRole)
    {
	if (!write_counted_string (configFile, theWindow->class.res_name))
	    return 0;
	if (!write_counted_string (configFile, theWindow->class.res_class))
	    return 0;
	if (theWindow->nameChanged)
	{
	    /*
	     * If WM_NAME changed on this window, we can't use it as
	     * a criteria for looking up window configurations.  See the
	     * longer explanation in the GetWindowConfig() function below.
	     */

	    if (!write_counted_string (configFile, NULL))
		return 0;
	}
	else
	{
	    if (!write_counted_string (configFile, theWindow->name))
		return 0;
	}
    
	wm_command = NULL;
	wm_command_count = 0;
	XGetCommand (dpy, theWindow->w, &wm_command, &wm_command_count);

	if (clientId || !wm_command || wm_command_count == 0)
	{
	    if (!write_byte (configFile, 0))
		return 0;
	}
	else
	{
	    if (!write_byte (configFile, (char) wm_command_count))
	        return 0;
	    for (i = 0; i < wm_command_count; i++)
	        if (!write_counted_string (configFile, wm_command[i]))
	            return 0;
	    XFreeStringList (wm_command);	    
	}
    }

    /* ===================[ Matthew McNeill Feb 1997 ]========================= *
     * there has been a structural change to TwmWindow in ctwm. The Icon information
     * is in a sub-structure now. The presence of icon information is not indicative
     * of its current state. There is a new boolean condition for this (isicon)
     */

    if (!write_byte (configFile, theWindow->isicon ? 1 : 0)) return 0;    /* iconified */

    /* ===================[ Matthew McNeill Feb 1997 ]========================= *
     * there has been a structural change to TwmWindow in ctwm. The Icon information
     * is in a sub-structure now, if there is no icon, this sub-structure does
     * not exist and the attempted access (below) causes a core dump.
     * we need to check that the structure exists before trying to access it
     */
    if (theWindow->icon)
      {
	if (!write_byte (configFile, theWindow->icon->w ? 1 : 0)) return 0; /* icon info exists */
	if (theWindow->icon->w)
	  {
	    int icon_x, icon_y;
	    XGetGeometry (dpy, theWindow->icon->w, &JunkRoot, &icon_x,
			  &icon_y, &JunkWidth, &JunkHeight, &JunkBW, &JunkDepth);
	    if (!write_short (configFile, (short) icon_x)) return 0;
	    if (!write_short (configFile, (short) icon_y)) return 0;
	  }
      }
    else
      {
	if (!write_byte (configFile, 0)) return 0; /* icon does not have any information */
      }
    /* ======================================================================= */

    if (!write_short (configFile, (short) theWindow->frame_x))
	return 0;
    if (!write_short (configFile, (short) theWindow->frame_y))
	return 0;
    if (!write_ushort (configFile, (unsigned short) theWindow->attr.width))
	return 0;
    if (!write_ushort (configFile, (unsigned short) theWindow->attr.height))
	return 0;
    if (!write_byte (configFile, theWindow->widthEverChangedByUser ? 1 : 0))
	return 0;
    if (!write_byte (configFile, theWindow->heightEverChangedByUser ? 1 : 0))
	return 0;

    /* ===================[ Matthew McNeill Feb 1997 ]=======================*
     * write an extra piece of information to the file, this is the occupation 
     * number and is a bit field of the workspaces occupied by the client.
     */

    if (!write_int (configFile, theWindow->occupation))
	return 0;

    /* ======================================================================*/

    return 1;
}

/*===[ Read Window Configuration Entry ]=====================================*/

int ReadWinConfigEntry (FILE *configFile, unsigned short version,
			TWMWinConfigEntry **pentry)
/* this function reads the next window configuration entry from the given file
 * else it returns FALSE if none exists or there is a problem
 */
{
    TWMWinConfigEntry *entry;
    unsigned char byte;
    int i;

    *pentry = entry = (TWMWinConfigEntry *) malloc (
	sizeof (TWMWinConfigEntry));
    if (!*pentry)
	return 0;

    entry->tag = 0;
    entry->client_id = NULL;
    entry->window_role = NULL;
    entry->class.res_name = NULL;
    entry->class.res_class = NULL;
    entry->wm_name = NULL;
    entry->wm_command = NULL;
    entry->wm_command_count = 0;

    if (!read_counted_string (configFile, &entry->client_id))
	goto give_up;

    if (!read_counted_string (configFile, &entry->window_role))
	goto give_up;

    if (!entry->window_role)
    {
	if (!read_counted_string (configFile, &entry->class.res_name))
	    goto give_up;
	if (!read_counted_string (configFile, &entry->class.res_class))
	    goto give_up;
	if (!read_counted_string (configFile, &entry->wm_name))
	    goto give_up;
    
	if (!read_byte (configFile, &byte))
	    goto give_up;
	entry->wm_command_count = byte;
	
	if (entry->wm_command_count == 0)
	    entry->wm_command = NULL;
	else
	{
	    entry->wm_command = (char **) malloc (entry->wm_command_count *
	        sizeof (char *));

	    if (!entry->wm_command)
		goto give_up;

	    for (i = 0; i < entry->wm_command_count; i++)
		if (!read_counted_string (configFile, &entry->wm_command[i]))
		    goto give_up;
	}
    }

    if (!read_byte (configFile, &byte))
	goto give_up;

    entry->iconified = byte;

    if (!read_byte (configFile, &byte))
	goto give_up;

    entry->icon_info_present = byte;

    if (entry->icon_info_present)
    {
	if (!read_short (configFile, (short *) &entry->icon_x))
	    goto give_up;
	if (!read_short (configFile, (short *) &entry->icon_y))
	    goto give_up;
    }

    if (!read_short (configFile, (short *) &entry->x))
	goto give_up;
    if (!read_short (configFile, (short *) &entry->y))
	goto give_up;
    if (!read_ushort (configFile, &entry->width))
	goto give_up;
    if (!read_ushort (configFile, &entry->height))
	goto give_up;

    if (version > 1)
    {
	if (!read_byte (configFile, &byte))
	    goto give_up;
	entry->width_ever_changed_by_user = byte;

	if (!read_byte (configFile, &byte))
	    goto give_up;
	entry->height_ever_changed_by_user = byte;
    }
    else
    {
	entry->width_ever_changed_by_user = False;
	entry->height_ever_changed_by_user = False;
    }

    /* ===================[ Matthew McNeill Feb 1997 ]======================= *
     * read in the occupation information to restore the windows to the 
     * correct workspaces.
     */

    if (!read_int (configFile, &entry->occupation))
	goto give_up;

    /* ====================================================================== */

    return 1;

give_up:

    if (entry->client_id)
	free (entry->client_id);
    if (entry->window_role)
	free (entry->window_role);
    if (entry->class.res_name)
	free (entry->class.res_name);
    if (entry->class.res_class)
	free (entry->class.res_class);
    if (entry->wm_name)
	free (entry->wm_name);
    if (entry->wm_command_count)
    {
	for (i = 0; i < entry->wm_command_count; i++)
	    if (entry->wm_command[i])
		free (entry->wm_command[i]);
    }
    if (entry->wm_command)
	free ((char *) entry->wm_command);
    
    free ((char *) entry);
    *pentry = NULL;

    return 0;
}

/*===[ Read In Win Config File ]=============================================*/

void ReadWinConfigFile (char *filename)
/* this function reads the window configuration file and stores the information
 * in a data structure which is returned
 */
{
    FILE *configFile;
    TWMWinConfigEntry *entry;
    int done = 0;
    unsigned short version;

    configFile = fopen (filename, "rb");
    if (!configFile)
	return;

    if (!read_ushort (configFile, &version) ||
	version > SAVEFILE_VERSION)
    {
	done = 1;
    }

    while (!done)
    {
	if (ReadWinConfigEntry (configFile, version, &entry))
	{
	    entry->next = winConfigHead;
	    winConfigHead = entry;
	}
	else
	    done = 1;
    }

    fclose (configFile);
}

/*===[ Get Window Configuration ]============================================*
 * Matthew McNeill Feb 1997 - added extra parameter (occupation) to return 
 *                            restored occupation of the window 
 */

int GetWindowConfig (TwmWindow *theWindow, short *x, short *y,
		     unsigned short *width, unsigned short *height,
		     Bool *iconified, Bool *icon_info_present,
		     short *icon_x, short *icon_y,
		     Bool *width_ever_changed_by_user,
		     Bool *height_ever_changed_by_user,
		     int *occupation) /* <== [ Matthew McNeill Feb 1997 ] == */
/* This function attempts to extract all the relevant information from the 
 * given window and return values via the rest of the parameters to the
 * function
 */
{
    char *clientId, *windowRole;
    TWMWinConfigEntry *ptr;
    int found = 0;

    ptr = winConfigHead;

    if (!ptr)
	return 0;

    clientId = GetClientID (theWindow->w);
    windowRole = GetWindowRole (theWindow->w);

    while (ptr && !found)
    {
	int client_id_match = (!clientId && !ptr->client_id) ||
	    (clientId && ptr->client_id &&
	    strcmp (clientId, ptr->client_id) == 0);

	if (!ptr->tag && client_id_match)
	{
	    if (windowRole || ptr->window_role)
	    {
		found = (windowRole && ptr->window_role &&
		    strcmp (windowRole, ptr->window_role) == 0);
	    }
	    else
	    {
		/*
		 * Compare WM_CLASS + only compare WM_NAME if the
		 * WM_NAME in the saved file is non-NULL.  If the
		 * WM_NAME in the saved file is NULL, this means that
		 * the client changed the value of WM_NAME during the
		 * session, and we can not use it as a criteria for
		 * our search.  For example, with xmh, at save time
		 * the window name might be "xmh: folderY".  However,
		 * if xmh does not properly restore state when it is
		 * restarted, the initial window name might be
		 * "xmh: folderX".  This would cause the window manager
		 * to fail in finding the saved window configuration.
		 * The best we can do is ignore WM_NAME if its value
		 * changed in the previous session.
		 */

		if (strcmp (theWindow->class.res_name,
			ptr->class.res_name) == 0 &&
		    strcmp (theWindow->class.res_class,
			ptr->class.res_class) == 0 &&
		   (ptr->wm_name == NULL ||
		    strcmp (theWindow->name, ptr->wm_name) == 0))
		{
		    if (clientId)
		    {
			/*
			 * If a client ID was present, we should not check
			 * WM_COMMAND because Xt will put a -xtsessionID arg
			 * on the command line.
			 */

			found = 1;
		    }
		    else
		    {
			/*
			 * For non-XSMP clients, also check WM_COMMAND.
			 */

			char **wm_command = NULL;
			int wm_command_count = 0, i;

			XGetCommand (dpy, theWindow->w,
			    &wm_command, &wm_command_count);

			if (wm_command_count == ptr->wm_command_count)
			{
			    for (i = 0; i < wm_command_count; i++)
				if (strcmp (wm_command[i],
				    ptr->wm_command[i]) != 0)
				    break;

			    if (i == wm_command_count)
				found = 1;
			}
		    }
		}
	    }
	}

	if (!found)
	    ptr = ptr->next;
    }

    if (found)
    {
	*x = ptr->x;
	*y = ptr->y;
	*width = ptr->width;
	*height = ptr->height;
	*iconified = ptr->iconified;
	*icon_info_present = ptr->icon_info_present;
	*width_ever_changed_by_user = ptr->width_ever_changed_by_user;
	*height_ever_changed_by_user = ptr->height_ever_changed_by_user;

	if (*icon_info_present)
	{
	    *icon_x = ptr->icon_x;
	    *icon_y = ptr->icon_y;
	}

	*occupation = ptr->occupation; /* <== [ Matthew McNeill Feb 1997 ] == */

	ptr->tag = 1;
    }
    else
	*iconified = 0;

    if (clientId)
	XFree (clientId);

    if (windowRole)
	XFree (windowRole);

    return found;
}

/*===[ Unique Filename Generator ]===========================================*/

static FILE *unique_file (char **filename, char *path, char *prefix)
/* this function attempts to allocate a temporary file to store the 
 * information of the windows
 */
{
    int fd;
    char tmp[PATH_MAX], template[PATH_MAX];
    FILE *fp;

    snprintf(tmp, sizeof(tmp), "%s/%sXXXXXX", path, prefix);
#ifndef HAVE_MKSTEMP
    do {
        if (fd == -1)
            strcpy(template, tmp);
        if ((mktemp(template) == NULL) || (template[0] == '\0'))
            return NULL;
        fd = open(template, O_RDWR | O_CREAT | O_EXCL, 0600);
    } while ((fd == -1) && (errno == EEXIST || errno == EINTR));
#else
    if ((fd = mkstemp(tmp)) == -1)
	return NULL;
#endif
    if ((fp = fdopen(fd, "wb")) == NULL)
	close(fd);
    *filename = strdup(template);
    return fp;
}

/*===[ SAVE WINDOW INFORMATION ]=============================================*/

#ifndef PATH_MAX
#  define PATH_MAX 1023
#endif

void SaveYourselfPhase2CB (SmcConn smcCon, SmPointer clientData)
/* this is where all the work is done in saving the state of the windows.
 * it is not done in Phase One because phase one is used for the other clients
 * to make sure that all the property information on their windows is correct
 * and up to date
 */
{
    int scrnum;
    ScreenInfo *theScreen;
    TwmWindow *theWindow;
    char *clientId, *windowRole;
    FILE *configFile = NULL;
    char *path;
    char *filename = NULL;
    Bool success = False;
    SmProp prop1, prop2, prop3, *props[3];
    SmPropValue prop1val, prop2val, prop3val;
    char discardCommand[PATH_MAX + 4];
    int numVals, i;
    static int first_time = 1;

    if (first_time)
    {
	char userId[20];
	char hint = SmRestartIfRunning;

	prop1.name = SmProgram;
	prop1.type = SmARRAY8;
	prop1.num_vals = 1;
	prop1.vals = &prop1val;
	prop1val.value = Argv[0];
	prop1val.length = strlen (Argv[0]);

	sprintf (userId, "%d", getuid());
	prop2.name = SmUserID;
	prop2.type = SmARRAY8;
	prop2.num_vals = 1;
	prop2.vals = &prop2val;
	prop2val.value = (SmPointer) userId;
	prop2val.length = strlen (userId);
	
	prop3.name = SmRestartStyleHint;
	prop3.type = SmCARD8;
	prop3.num_vals = 1;
	prop3.vals = &prop3val;
	prop3val.value = (SmPointer) &hint;
	prop3val.length = 1;
	
	props[0] = &prop1;
	props[1] = &prop2;
	props[2] = &prop3;

	SmcSetProperties (smcCon, 3, props);

	first_time = 0;
    }

    path = getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = getenv ("HOME");
	if (!path)
	    path = ".";
    }
    /*==============[ Matthew McNeill Feb 1997 ]==============*
     *        changed the unique name to CTWM rather than TWM 
     *        this is tidier and more functional and prevents
     *        TWM picking up CTWM config files. The format is 
     *        no longer the same since the new format supports
     *        virtaul workspaces.
     *========================================================*/
    if ((configFile = unique_file (&filename, path, ".ctwm")) == NULL)
	goto bad;

    if (!write_ushort (configFile, SAVEFILE_VERSION))
	goto bad;

    success = True;

    for (scrnum = 0; scrnum < NumScreens && success; scrnum++)
    {
	if (ScreenList[scrnum] != NULL)
	{
	    theScreen = ScreenList[scrnum];
	    theWindow = theScreen->FirstWindow;

	    while (theWindow && success)
	    {
		clientId = GetClientID (theWindow->w);
		windowRole = GetWindowRole (theWindow->w);

		if (!WriteWinConfigEntry (configFile, theWindow,
		    clientId, windowRole))
		    success = False;

		if (clientId)
		    XFree (clientId);

		if (windowRole)
		    XFree (windowRole);

		theWindow = theWindow->next;
	    }
	}
    }
    
    prop1.name = SmRestartCommand;
    prop1.type = SmLISTofARRAY8;

    prop1.vals = (SmPropValue *) malloc (
	(Argc + 4) * sizeof (SmPropValue));

    if (!prop1.vals)
    {
	success = False;
	goto bad;
    }

    numVals = 0;

    for (i = 0; i < Argc; i++)
    {
	if (strcmp (Argv[i], "-clientId") == 0 ||
	    strcmp (Argv[i], "-restore") == 0)
	{
	    i++;
	}
	else
	{
	    prop1.vals[numVals].value = (SmPointer) Argv[i];
	    prop1.vals[numVals++].length = strlen (Argv[i]);
	}
    }

    prop1.vals[numVals].value = (SmPointer) "-clientId";
    prop1.vals[numVals++].length = 9;

    prop1.vals[numVals].value = (SmPointer) twm_clientId;
    prop1.vals[numVals++].length = strlen (twm_clientId);

    prop1.vals[numVals].value = (SmPointer) "-restore";
    prop1.vals[numVals++].length = 8;

    prop1.vals[numVals].value = (SmPointer) filename;
    prop1.vals[numVals++].length = strlen (filename);

    prop1.num_vals = numVals;

    sprintf (discardCommand, "rm %s", filename);
    prop2.name = SmDiscardCommand;
    prop2.type = SmARRAY8;
    prop2.num_vals = 1;
    prop2.vals = &prop2val;
    prop2val.value = (SmPointer) discardCommand;
    prop2val.length = strlen (discardCommand);

    props[0] = &prop1;
    props[1] = &prop2;

    SmcSetProperties (smcCon, 2, props);
    free ((char *) prop1.vals);

 bad:
    SmcSaveYourselfDone (smcCon, success);
    sent_save_done = 1;

    if (configFile)
	fclose (configFile);

    if (filename)
	free (filename);
}

/*===[ Save Yourself SM CallBack ]===========================================*/

void SaveYourselfCB (SmcConn smcCon, SmPointer clientData,
		     int saveType, Bool shutdown, int interactStyle, Bool fast)
/* this procedure is called by the session manager when requesting the 
 * window manager to save its status, ie all the window configurations 
 */
{
    if (!SmcRequestSaveYourselfPhase2 (smcCon, SaveYourselfPhase2CB, NULL))
    {  
        SmcSaveYourselfDone (smcCon, False); 
        sent_save_done = 1;
    }
    else
	sent_save_done = 0;
}

/*===[ Die SM Call Back ]====================================================*/

void DieCB (SmcConn smcCon, SmPointer clientData)
/* this procedure is called by the session manager when requesting that the 
 * application shut istelf down
 */
{
    SmcCloseConnection (smcCon, 0, NULL);
    XtRemoveInput (iceInputId);
    Done(0);
}

/*===[ Save Complete SM Call Back ]==========================================*/

void SaveCompleteCB (SmcConn smcCon, SmPointer clientData)
/* This function is called to say that the save has been completed and that
 * the program can continue its operation
 */
{
    ;
}

/*===[ Shutdown Cancelled SM Call Back ]=====================================*/

void ShutdownCancelledCB (SmcConn smcCon, SmPointer clientData)

{
    if (!sent_save_done)
    {
	SmcSaveYourselfDone (smcCon, False);
	sent_save_done = 1;
    }
}

/*===[ Process ICE Message ]=================================================*/

void ProcessIceMsgProc (XtPointer client_data, int *source, XtInputId *id)

{
    IceConn	ice_conn = (IceConn) client_data;
    IceProcessMessages (ice_conn, NULL, NULL);
}


/*===[ Connect To Session Manager ]==========================================*/

void ConnectToSessionManager (char *previous_id)
/* This procedure attempts to connect to the session manager and setup the
 * interclent exchange via the Call Backs
 */
{
    char errorMsg[256];
    unsigned long mask;
    SmcCallbacks callbacks;
    IceConn iceConn;

    mask = SmcSaveYourselfProcMask | SmcDieProcMask |
	SmcSaveCompleteProcMask | SmcShutdownCancelledProcMask;

    callbacks.save_yourself.callback = SaveYourselfCB;
    callbacks.save_yourself.client_data = (SmPointer) NULL;

    callbacks.die.callback = DieCB;
    callbacks.die.client_data = (SmPointer) NULL;

    callbacks.save_complete.callback = SaveCompleteCB;
    callbacks.save_complete.client_data = (SmPointer) NULL;

    callbacks.shutdown_cancelled.callback = ShutdownCancelledCB;
    callbacks.shutdown_cancelled.client_data = (SmPointer) NULL;

    smcConn = SmcOpenConnection (
	NULL, 			/* use SESSION_MANAGER env */
	(SmPointer) appContext,
	SmProtoMajor,
	SmProtoMinor,
	mask,
	&callbacks,
	previous_id,
	&twm_clientId,
	256, errorMsg);

    if (smcConn == NULL)
      {
#ifdef DEBUG
       /* == [ Matthew McNeill Feb 1997 ] == */
       fprintf (stderr, "%s : Connection to session manager failed. (%s)",ProgramName, errorMsg);
#endif
       return;
      }

    iceConn = SmcGetIceConnection (smcConn);

    iceInputId = XtAppAddInput (
	    appContext,
	    IceConnectionNumber (iceConn),
            (XtPointer) XtInputReadMask,
	    ProcessIceMsgProc,
	    (XtPointer) iceConn);
}
