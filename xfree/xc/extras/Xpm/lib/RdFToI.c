/*
 * Copyright (C) 1989-95 GROUPE BULL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * GROUPE BULL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of GROUPE BULL shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from GROUPE BULL.
 */

/*****************************************************************************\
*  RdFToI.c:                                                                  *
*                                                                             *
*  XPM library                                                                *
*  Parse an XPM file and create the image and possibly its mask               *
*                                                                             *
*  Developed by Arnaud Le Hors                                                *
\*****************************************************************************/
/* $XFree86: xc/extras/Xpm/lib/RdFToI.c,v 1.3 2004/11/18 21:30:51 herrb Exp $ */

/* October 2004, source code review by Thomas Biege <thomas@suse.de> */

#include "XpmI.h"
#ifndef NO_ZPIPE
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

LFUNC(OpenReadFile, int, (char *filename, xpmData *mdata));
LFUNC(xpmDataClose, void, (xpmData *mdata));

#ifndef CXPMPROG
int
XpmReadFileToImage(display, filename,
		   image_return, shapeimage_return, attributes)
    Display *display;
    char *filename;
    XImage **image_return;
    XImage **shapeimage_return;
    XpmAttributes *attributes;
{
    XpmImage image;
    XpmInfo info;
    int ErrorStatus;
    xpmData mdata;

    xpmInitXpmImage(&image);
    xpmInitXpmInfo(&info);

    /* open file to read */
    if ((ErrorStatus = OpenReadFile(filename, &mdata)) != XpmSuccess)
	return (ErrorStatus);

    /* create the XImage from the XpmData */
    if (attributes) {
	xpmInitAttributes(attributes);
	xpmSetInfoMask(&info, attributes);
	ErrorStatus = xpmParseDataAndCreate(display, &mdata,
					    image_return, shapeimage_return,
					    &image, &info, attributes);
    } else
	ErrorStatus = xpmParseDataAndCreate(display, &mdata,
					    image_return, shapeimage_return,
					    &image, NULL, attributes);
    if (attributes) {
	if (ErrorStatus >= 0)		/* no fatal error */
	    xpmSetAttributes(attributes, &image, &info);
	XpmFreeXpmInfo(&info);
    }

    xpmDataClose(&mdata);
    /* free the XpmImage */
    XpmFreeXpmImage(&image);

    return (ErrorStatus);
}

int
XpmReadFileToXpmImage(filename, image, info)
    char *filename;
    XpmImage *image;
    XpmInfo *info;
{
    xpmData mdata;
    int ErrorStatus;

    /* init returned values */
    xpmInitXpmImage(image);
    xpmInitXpmInfo(info);

    /* open file to read */
    if ((ErrorStatus = OpenReadFile(filename, &mdata)) != XpmSuccess)
	return (ErrorStatus);

    /* create the XpmImage from the XpmData */
    ErrorStatus = xpmParseData(&mdata, image, info);

    xpmDataClose(&mdata);

    return (ErrorStatus);
}
#endif /* CXPMPROG */

#ifndef NO_ZPIPE
/* Do not depend on errno after read_through */
FILE*
xpmPipeThrough(fd, cmd, arg1, mode)
    int fd;
    const char* cmd;
    const char* arg1;
    const char* mode;
{
    FILE* fp;
    int status, fds[2], in = 0, out = 1;
    pid_t pid;
    if ( 'w' == *mode )
	out = 0, in = 1;
    if ( pipe(fds) < 0 )
	return NULL;
    pid = fork();
    if ( pid < 0 )
	goto fail1;
    if ( 0 == pid )
    {
	close(fds[in]);
	if ( dup2(fds[out], out) < 0 )
	    goto err;
	close(fds[out]);
	if ( dup2(fd, in) < 0 )
	    goto err;
	close(fd);
	pid = fork();
	if ( pid < 0 )
	    goto err;
	if ( 0 == pid )
	{
	    execlp(cmd, cmd, arg1, NULL);
	    perror(cmd);
	    goto err;
	}
	_exit(0);
    err:
	_exit(1);
    }
    close(fds[out]);
    /* calling process: wait for first child */
    while ( waitpid(pid, &status, 0) < 0 && EINTR == errno )
	;
    if ( WIFSIGNALED(status) ||
	 (WIFEXITED(status) && WEXITSTATUS(status) != 0) )
	goto fail2;
    fp = fdopen(fds[in], mode);
    if ( !fp )
	goto fail2;
    close(fd); /* still open in 2nd child */
    return fp;
fail1:
    close(fds[out]);
fail2:
    close(fds[in]);
    return NULL;
}
#endif

/*
 * open the given file to be read as an xpmData which is returned.
 */
#ifndef NO_ZPIPE
	FILE *s_popen(char *cmd, const char *type);
#else
#	define s_popen popen
#endif

static int
OpenReadFile(filename, mdata)
    char *filename;
    xpmData *mdata;
{
    if (!filename) {
	mdata->stream.file = (stdin);
	mdata->type = XPMFILE;
    } else {
#ifndef NO_ZPIPE
	size_t len = strlen(filename);

	if(len == 0                        ||
	   filename[len-1] == '/')
		return(XpmOpenFailed);
	if ((len > 2) && !strcmp(".Z", filename + (len - 2))) {
	    mdata->type = XPMPIPE;
	    snprintf(buf, sizeof(buf), "uncompress -c \"%s\"", filename);
	    if (!(mdata->stream.file = s_popen(buf, "r")))
		return (XpmOpenFailed);

	} else if ((len > 3) && !strcmp(".gz", filename + (len - 3))) {
	    mdata->type = XPMPIPE;
	    snprintf(buf, sizeof(buf), "gunzip -qc \"%s\"", filename);
	    if (!(mdata->stream.file = s_popen(buf, "r")))
		return (XpmOpenFailed);

	} else {
# ifdef STAT_ZFILE
	    if (!(compressfile = (char *) XpmMalloc(len + 4)))
		return (XpmNoMemory);

	    snprintf(compressfile, len+4, "%s.Z", filename);
	    if (!stat(compressfile, &status)) {
		snprintf(buf, sizeof(buf), "uncompress -c \"%s\"", compressfile);
		if (!(mdata->stream.file = s_popen(buf, "r"))) {
		    XpmFree(compressfile);
		    return (XpmOpenFailed);
		}
		mdata->type = XPMPIPE;
	    } else {
		snprintf(compressfile, len+4, "%s.gz", filename);
		if (!stat(compressfile, &status)) {
		    snprintf(buf, sizeof(buf), "gunzip -c \"%s\"", compressfile);
		    if (!(mdata->stream.file = s_popen(buf, "r"))) {
			XpmFree(compressfile);
			return (XpmOpenFailed);
		    }
		    mdata->type = XPMPIPE;
		} else {
# endif
#endif
		    if (!(mdata->stream.file = fopen(filename, "r"))) {
#if !defined(NO_ZPIPE) && defined(STAT_ZFILE)
			XpmFree(compressfile);
#endif
			return (XpmOpenFailed);
		    }
		    mdata->type = XPMFILE;
#ifndef NO_ZPIPE
# ifdef STAT_ZFILE
		}
	    }
	    XpmFree(compressfile);
	}
#endif
	if ( ext && !strcmp(ext, ".Z") )
	{
	    mdata->type = XPMPIPE;
	    mdata->stream.file = xpmPipeThrough(fd, "uncompress", "-c", "r");
	}
	else if ( ext && !strcmp(ext, ".gz") )
	{
	    mdata->type = XPMPIPE;
	    mdata->stream.file = xpmPipeThrough(fd, "gunzip", "-qc", "r");
	}
	else
#endif /* z-files */
	{
	    mdata->type = XPMFILE;
	    mdata->stream.file = fdopen(fd, "r");
	}
	if (!mdata->stream.file)
	{
	    close(fd);
	    return (XpmOpenFailed);
	}
    }
    mdata->CommentLength = 0;
#ifdef CXPMPROG
    mdata->lineNum = 0;
    mdata->charNum = 0;
#endif
    return (XpmSuccess);
}

/*
 * close the file related to the xpmData if any
 */
static void
xpmDataClose(mdata)
    xpmData *mdata;
{
    switch (mdata->type) {
    case XPMFILE:
	if (mdata->stream.file != (stdin))
	    fclose(mdata->stream.file);
	break;
#ifndef NO_ZPIPE
    case XPMPIPE:
	fclose(mdata->stream.file);
	break;
#endif
    }
}
