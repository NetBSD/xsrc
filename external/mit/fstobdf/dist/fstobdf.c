/*

Copyright 1990, 1998  The Open Group

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

 * Copyright 1990 Network Computing Devices;
 * Portions Copyright 1987 by Digital Equipment Corporation
 *
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Network Computing Devices, or Digital
 * not be used in advertising or publicity pertaining to distribution
 * of the software without specific, written prior permission.
 *
 * NETWORK COMPUTING DEVICES, AND DIGITAL DISCLAIM ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL NETWORK COMPUTING DEVICES,
 * OR DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include	<stdio.h>
#include	<stdlib.h>
#include        <string.h>
#include	"fstobdf.h"


static void _X_NORETURN _X_COLD
usage(const char *progName, const char *msg)
{
    if (msg)
        fprintf(stderr, "%s: %s\n", progName, msg);
    fprintf(stderr,
            "Usage: %s [-server <font server>] -fn <font name>\n"
            "	or: %s -version\n", progName, progName);
    exit(0);
}

static void _X_NORETURN _X_COLD
Fail(const char *progName)
{
    fprintf(stderr, "%s: unable to dump font\n", progName);
    exit(1);
}

int
main(int argc, char *argv[])
{
    FSServer *fontServer;
    Font fontID, dummy;
    FSBitmapFormat bitmapFormat;
    FSXFontInfoHeader fontHeader;
    FSPropInfo propInfo;
    FSPropOffset *propOffsets;
    unsigned char *propData;

    FILE *outFile;
    char *fontName;
    char *serverName;
    int i;

    fontName = NULL;
    serverName = NULL;
    outFile = stdout;

    for (i = 1; i < argc; i++) {
        if (!strncmp(argv[i], "-s", 2)) {
            if (argv[++i])
                serverName = argv[i];
            else
                usage(argv[0], "-server requires an argument");
        }
        else if (!strncmp(argv[i], "-fn", 3)) {
            if (argv[++i])
                fontName = argv[i];
            else
                usage(argv[0], "-fn requires an argument");
        }
        else if (!strcmp(argv[i], "-version")) {
            printf("%s\n", PACKAGE_STRING);
            exit(0);
        }
        else {
            fprintf(stderr, "%s: unrecognized option '%s'\n", argv[0], argv[i]);
            usage(argv[0], NULL);
        }
    }

    if (fontName == NULL)
        usage(argv[0], "No font name specified");

    fontServer = FSOpenServer(serverName);
    if (!fontServer) {
        const char *sn = FSServerName(serverName);

        if (sn)
            fprintf(stderr, "%s: can't open font server \"%s\"\n", argv[0], sn);
        else
            usage(argv[0], "No font server specified.");
        exit(0);
    }
    bitmapFormat = 0;
    fontID = FSOpenBitmapFont(fontServer, bitmapFormat, (FSBitmapFormatMask) 0,
                              fontName, &dummy);
    if (!fontID) {
        printf("can't open font \"%s\"\n", fontName);
        exit(0);
    }
    FSQueryXInfo(fontServer, fontID, &fontHeader, &propInfo, &propOffsets,
                 &propData);

    if (!EmitHeader(outFile, &fontHeader, &propInfo, propOffsets, propData))
        Fail(argv[0]);
    if (!EmitProperties(outFile, &fontHeader, &propInfo, propOffsets, propData))
        Fail(argv[0]);
    if (!EmitCharacters(outFile, fontServer, &fontHeader, fontID))
        Fail(argv[0]);
    fprintf(outFile, "ENDFONT\n");

    FSFree((char *) propOffsets);
    FSFree((char *) propData);
    exit(0);
}
