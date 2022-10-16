/******************************************************************************

Copyright 1994, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

Author:  Ralph Mor, X Consortium
******************************************************************************/

#include "smproxy.h"
#ifdef HAVE_MKSTEMP
#include <unistd.h>
#endif


static ProxyFileEntry *proxyFileHead = NULL;

static int write_byte ( FILE *file, unsigned char b );
static int write_short ( FILE *file, unsigned short s );
static int write_counted_string ( FILE *file, char *string );
static int read_byte ( FILE *file, unsigned char *bp );
static int read_short ( FILE *file, unsigned short *shortp );
static int read_counted_string ( FILE *file, char **stringp );

#ifndef HAVE_ASPRINTF
# include <stdarg.h>

/* sprintf variant found in newer libc's which allocates string to print to */
_X_HIDDEN int _X_ATTRIBUTE_PRINTF(2,3)
asprintf(char ** ret, const char *format, ...)
{
    char buf[256];
    int len;
    va_list ap;

    va_start(ap, format);
    len = vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    if (len < 0)
	return -1;

    if (len < sizeof(buf))
    {
	*ret = strdup(buf);
    }
    else
    {
	*ret = malloc(len + 1); /* snprintf doesn't count trailing '\0' */
	if (*ret != NULL)
	{
	    va_start(ap, format);
	    len = vsnprintf(*ret, len + 1, format, ap);
	    va_end(ap);
	    if (len < 0) {
		free(*ret);
		*ret = NULL;
	    }
	}
    }

    if (*ret == NULL)
	return -1;

    return len;
}
#endif



static int
write_byte (FILE *file, unsigned char b)
{
    if (fwrite (&b, 1, 1, file) != 1)
	return 0;
    return 1;
}


static int
write_short (FILE *file, unsigned short s)
{
    unsigned char   file_short[2];

    file_short[0] = (s & (unsigned)0xff00) >> 8;
    file_short[1] = s & 0xff;
    if (fwrite (file_short, sizeof (file_short), 1, file) != 1)
	return 0;
    return 1;
}


static int
write_counted_string(FILE *file, char *string)
{
    if (string)
    {
	size_t count = strlen (string);

	if (write_byte (file, (unsigned char)count) == 0)
	    return 0;
	if (fwrite (string, sizeof (char), count, file) != count)
	    return 0;
    }
    else
    {
	if (write_byte (file, 0) == 0)
	    return 0;
    }

    return 1;
}



static int
read_byte(FILE *file, unsigned char *bp)
{
    if (fread (bp, 1, 1, file) != 1)
	return 0;
    return 1;
}


static int
read_short(FILE *file, unsigned short *shortp)
{
    unsigned char   file_short[2];

    if (fread (file_short, sizeof (file_short), 1, file) != 1)
	return 0;
    *shortp = file_short[0] * 256 + file_short[1];
    return 1;
}


static int
read_counted_string(FILE *file, char **stringp)
{
    unsigned char  len;
    char	   *data;

    if (read_byte (file, &len) == 0)
	return 0;
    if (len == 0) {
	data = NULL;
    } else {
	data = malloc ((size_t) len + 1);
    	if (!data)
	    return 0;
	if (fread (data, sizeof (char), (size_t) len, file) != len) {
	    free (data);
	    return 0;
    	}
	data[len] = '\0';
    }
    *stringp = data;
    return 1;
}



/*
 * An entry in the .smproxy file looks like this:
 *
 * FIELD				BYTES
 * -----                                ----
 * client ID len			1
 * client ID				LIST of bytes
 * WM_CLASS "res name" length		1
 * WM_CLASS "res name"			LIST of bytes
 * WM_CLASS "res class" length          1
 * WM_CLASS "res class"                 LIST of bytes
 * WM_NAME length			1
 * WM_NAME				LIST of bytes
 * WM_COMMAND arg count			1
 * For each arg in WM_COMMAND
 *    arg length			1
 *    arg				LIST of bytes
 */

int
WriteProxyFileEntry(FILE *proxyFile, WinInfo *theWindow)
{
    if (!write_counted_string (proxyFile, theWindow->client_id))
	return 0;
    if (!write_counted_string (proxyFile, theWindow->class.res_name))
	return 0;
    if (!write_counted_string (proxyFile, theWindow->class.res_class))
	return 0;
    if (!write_counted_string (proxyFile, theWindow->wm_name))
	return 0;
    
    if (!theWindow->wm_command || theWindow->wm_command_count == 0)
    {
	if (!write_byte (proxyFile, 0))
	    return 0;
    }
    else
    {
	if (!write_byte (proxyFile, (char) theWindow->wm_command_count))
	    return 0;
	for (int i = 0; i < theWindow->wm_command_count; i++)
	    if (!write_counted_string (proxyFile, theWindow->wm_command[i]))
		return 0;
    }

    return 1;
}


int
ReadProxyFileEntry(FILE *proxyFile, ProxyFileEntry **pentry)
{
    ProxyFileEntry *entry;
    unsigned char byte;
    int i;

    *pentry = entry = malloc (sizeof (ProxyFileEntry));
    if (!*pentry)
	return 0;

    entry->tag = 0;
    entry->client_id = NULL;
    entry->class.res_name = NULL;
    entry->class.res_class = NULL;
    entry->wm_name = NULL;
    entry->wm_command = NULL;
    entry->wm_command_count = 0;

    if (!read_counted_string (proxyFile, &entry->client_id))
	goto give_up;
    if (!read_counted_string (proxyFile, &entry->class.res_name))
	goto give_up;
    if (!read_counted_string (proxyFile, &entry->class.res_class))
	goto give_up;
    if (!read_counted_string (proxyFile, &entry->wm_name))
	goto give_up;
    
    if (!read_byte (proxyFile, &byte))
	goto give_up;
    entry->wm_command_count = byte;

    if (entry->wm_command_count == 0)
	entry->wm_command = NULL;
    else
    {
	entry->wm_command = calloc (entry->wm_command_count, sizeof (char *));

	if (!entry->wm_command)
	    goto give_up;

	for (i = 0; i < entry->wm_command_count; i++)
	    if (!read_counted_string (proxyFile, &entry->wm_command[i]))
		goto give_up;
    }

    return 1;

give_up:

    if (entry->client_id)
	free (entry->client_id);
    if (entry->class.res_name)
	free (entry->class.res_name);
    if (entry->class.res_class)
	free (entry->class.res_class);
    if (entry->wm_name)
	free (entry->wm_name);
    if (entry->wm_command)
    {
        if (entry->wm_command_count)
        {
	    for (i = 0; i < entry->wm_command_count; i++)
	        if (entry->wm_command[i])
		    free (entry->wm_command[i]);
        }
	free (entry->wm_command);
    }

    free (entry);
    *pentry = NULL;

    return 0;
}


void
ReadProxyFile(char *filename)
{
    FILE *proxyFile;
    ProxyFileEntry *entry;
    int done = 0;
    unsigned short version;

    proxyFile = fopen (filename, "rb");
    if (!proxyFile)
	return;

    if (!read_short (proxyFile, &version) ||
	version > SAVEFILE_VERSION)
    {
	done = 1;
    }

    while (!done)
    {
	if (ReadProxyFileEntry (proxyFile, &entry))
	{
	    entry->next = proxyFileHead;
	    proxyFileHead = entry;
	}
	else
	    done = 1;
    }

    fclose (proxyFile);
}



static char *
unique_filename(const char *path, const char *prefix, int *pFd)
{
    char *tempFile = NULL;
    int tempFd = 0;

#if defined(HAVE_MKSTEMP) || defined(HAVE_MKTEMP)
    if (asprintf (&tempFile, "%s/%sXXXXXX", path, prefix) == -1)
	return NULL;
#endif

#ifdef HAVE_MKSTEMP
    tempFd = mkstemp(tempFile);
#else

# ifdef HAVE_MKTEMP
    if (mktemp(tempFile) == NULL)
	tempFd = -1;
# else /* fallback to tempnam */
    tempFile = tempnam (path, prefix);
# endif /* HAVE_MKTEMP */

    if (tempFd != -1 && tempFile != NULL)
	tempFd = open(tempFile, O_RDWR | O_CREAT | O_EXCL, 0600);
#endif

    if (tempFd == -1) {
	free(tempFile);
	return (NULL);
    }

    *pFd = tempFd;
    return tempFile;

}



char *
WriteProxyFile(void)
{
    FILE *proxyFile = NULL;
    char *filename = NULL;
    int fd = -1;
    const char *path;
    WinInfo *winptr;
    Bool success = False;

    path = getenv ("SM_SAVE_DIR");
    if (!path)
    {
	path = getenv ("HOME");
	if (!path)
	    path = ".";
    }

    if ((filename = unique_filename (path, ".prx", &fd)) == NULL)
	goto bad;

    if (!(proxyFile = fdopen(fd, "wb"))) 
	goto bad;

    if (!write_short (proxyFile, SAVEFILE_VERSION))
	goto bad;

    success = True;
    winptr = win_head;

    while (winptr && success)
    {
	if (winptr->client_id)
	    if (!WriteProxyFileEntry (proxyFile, winptr))
	    {
		success = False;
		break;
	    }

	winptr = winptr->next;
    }

 bad:

    if (proxyFile)
	fclose (proxyFile);
    else if (fd != -1)
	close (fd);

    if (success)
	return (filename);
    else
    {
	if (filename)
	    free (filename);
	return (NULL);
    }
}



char *
LookupClientID(WinInfo *theWindow)
{
    ProxyFileEntry *ptr;
    int found = 0;

    ptr = proxyFileHead;
    while (ptr && !found)
    {
	if (!ptr->tag &&
            strcmp (theWindow->class.res_name, ptr->class.res_name) == 0 &&
	    strcmp (theWindow->class.res_class, ptr->class.res_class) == 0 &&
	    strcmp (theWindow->wm_name, ptr->wm_name) == 0)
	{
	    if (theWindow->wm_command_count == ptr->wm_command_count)
	    {
		int i;

		for (i = 0; i < theWindow->wm_command_count; i++)
		    if (strcmp (theWindow->wm_command[i],
			ptr->wm_command[i]) != 0)
			break;

		if (i == theWindow->wm_command_count)
		    found = 1;
	    }
	}

	if (!found)
	    ptr = ptr->next;
    }

    if (found)
    {
	ptr->tag = 1;
	return (ptr->client_id);
    }
    else
	return NULL;
}
