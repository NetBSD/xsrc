/* $XFree86: xc/programs/Xserver/hw/xfree86/etc/mmapr.c,v 1.10 2004/12/31 16:07:09 tsi Exp $ */
/*
 * Copyright 2002 through 2005 by Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#define _LARGEFILE_SOURCE 1
#define _FILE_OFFSET_BITS 64
#undef  __STRICT_ANSI__
#include <errno.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef MAP_FAILED
# define MAP_FAILED ((void *)(-1))
#endif

#if defined(_SCO_DS) && !defined(_SCO_DS_LL)
#define strtoull (unsigned long long)strtoul
#endif

#if !defined(strtoull) && \
    (defined(CSRG_BASED) || \
     (defined(__GNU_LIBRARY__) && \
       (__GNU_LIBRARY__ < 6)))
# define strtoull strtouq
#endif

static unsigned char datab;
static unsigned short dataw;
static unsigned int datal;
static unsigned long dataL;
static unsigned long long dataq;

static const char hextab[] = "0123456789ABCDEF";

static void
usage(void)
{
    fprintf(stderr, "\n"
        "mmapr [-p] [-{bwlqL}] <file> <offset> <length>\n\n"
        " -p   pretty-print output\n\n"
        "access size flags:\n\n"
        " -b   output one byte at a time\n"
        " -w   output up to two aligned bytes at a time\n"
        " -l   output up to four aligned bytes at a time (default)\n"
        " -q   output up to eight aligned bytes at a time\n");
    switch (sizeof(dataL))
    {
        case sizeof(datab):
            fprintf(stderr, " -L   same as -b\n\n");
            break;

        case sizeof(dataw):
            fprintf(stderr, " -L   same as -w\n\n");
            break;

        case sizeof(datal):
            fprintf(stderr, " -L   same as -l\n\n");
            break;

        case sizeof(dataq):
            fprintf(stderr, " -L   same as -q\n\n");
            break;

        default:
            fprintf(stderr, "\n");
            break;
    }

    exit(1);
}

int
main(int argc, char **argv)
{
    off_t Offset = 0, offset, End;
    size_t Length = 0, length, size;
    char *BadString, *data;
    void *buffer;
    int fd, pagesize, prettyprint = 0;
    char Address[20], Hex[36], Glyph[17];
    char Size = sizeof(datal), Format = 0;

    while (argv[1] && (argv[1][0] == '-') && argv[1][1])
    {
        for (;  argv[1][1];  argv[1]++)
        {
            switch (argv[1][1])
            {
                case 'p':
                    prettyprint = 1;
                    break;

                case 'b':
                    Size = sizeof(datab);
                    break;

                case 'w':
                    Size = sizeof(dataw);
                    break;

                case 'l':
                    Size = sizeof(datal);
                    break;

                case 'L':
                    Size = sizeof(dataL);
                    break;

                case 'q':
                    Size = sizeof(dataq);
                    break;

                default:
                    usage();
            }
        }

        argc--;
        argv++;
    }

    if (argc != 4)
        usage();

    BadString = (char *)0;
    Offset = strtoull(argv[2], &BadString, 0);
    if (errno || (BadString && *BadString))
        usage();

    BadString = (char *)0;
    Length = strtoul(argv[3], &BadString, 0);
    if (errno || (BadString && *BadString))
        usage();

    if (Length <= 0)
        return 0;

    if ((fd = open(argv[1], O_RDONLY)) < 0)
    {
        fprintf(stderr, "mmapr:  Unable to open \"%s\":  %s.\n",
            argv[1], strerror(errno));
        exit(1);
    }

    pagesize = getpagesize();
    offset = Offset & (off_t)(-pagesize);
    length = ((Offset + Length + pagesize - 1) & (off_t)(-pagesize)) - offset;
    buffer = mmap((caddr_t)0, length, PROT_READ, MAP_SHARED, fd, offset);
    close(fd);
    if (buffer == MAP_FAILED)
    {
        fprintf(stderr, "mmapr:  Unable to mmap \"%s\":  %s.\n",
            argv[1], strerror(errno));
        exit(1);
    }

    if (prettyprint)
    {
        End = Offset + Length - 1;

        if ((sizeof(Offset) > sizeof(dataL)) &&
            ((unsigned long long)End != (unsigned long)End))
        {
            sprintf(Address, "0x%015llX0", (unsigned long long)Offset >> 4);
            Format = 3;
        }
        else
        if ((sizeof(Offset) > sizeof(dataw)) &&
            ((unsigned long long)End != (unsigned short)End))
        {
            sprintf(Address, "0x%07lX0", (unsigned long)Offset >> 4);
            Format = 2;
        }
        else
        if ((sizeof(Offset) > sizeof(datab)) &&
            ((unsigned long long)End != (unsigned char)End))
        {
            sprintf(Address, "0x%03X0", (unsigned short)Offset >> 4);
            Format = 1;
        }
        else
        {
            sprintf(Address, "0x%01X0", (unsigned char)Offset >> 4);
         /* Format = 0; */
        }

        memset(Hex, ' ', 35);
        Hex[35] = 0;
        memset(Glyph, ' ', 16);
        Glyph[16] = 0;
    }

    Offset -= offset;
    while (Length > 0)
    {
        if ((Offset & sizeof(datab)) ||
            (Length < sizeof(dataw)) ||
            (Size < sizeof(dataw)))
        {
            datab = *(volatile unsigned char *)((char *)buffer + Offset);
            data = (char *)&datab;
            size = sizeof(datab);
        }
        else
        if ((Offset & sizeof(dataw)) ||
            (Length < sizeof(datal)) ||
            (Size < sizeof(datal)))
        {
            dataw = *(volatile unsigned short *)((char *)buffer + Offset);
            data = (char *)&dataw;
            size = sizeof(dataw);
        }
        else
        if ((Offset & sizeof(datal)) ||
            (Length < sizeof(dataL)) ||
            (Size < sizeof(dataL)))
        {
            datal = *(volatile unsigned int *)((char *)buffer + Offset);
            data = (char *)&datal;
            size = sizeof(datal);
        }
        else
        if ((Offset & sizeof(dataL)) ||
            (Length < sizeof(dataq)) ||
            (Size < sizeof(dataq)))
        {
            dataL = *(volatile unsigned long *)((char *)buffer + Offset);
            data = (char *)&dataL;
            size = sizeof(dataL);
        }
        else
        {
            dataq = *(volatile unsigned long long *)((char *)buffer + Offset);
            data = (char *)&dataq;
            size = sizeof(dataq);
        }

        if (prettyprint)
        {
            unsigned int i = (offset + Offset) & 15;

            Offset += size;
            Length -= size;

            for (;  size > 0;  --size, ++i, ++data)
            {
                Hex[((i >> 2) * 9) + ((i & 3) << 1)] =
                    hextab[(unsigned char)*data >> 4];
                Hex[((i >> 2) * 9) + ((i & 3) << 1) + 1] =
                    hextab[(unsigned char)*data & 15];

                if (isprint(*data))
                    Glyph[i] = *data;
                else
                    Glyph[i] = '.';
            }

            if (!Length || !(Offset & 15))
            {
                printf("%s:  %s  |%s|\n", Address, Hex, Glyph);

                if (!Length)
                    break;

                switch(Format)
                {
                    case 0:
                        sprintf(Address, "0x%02X",
                                (unsigned char)(Offset + offset));
                        break;

                    case 1:
                        sprintf(Address, "0x%04X",
                                (unsigned short)(Offset + offset));
                        break;

                    case 2:
                        sprintf(Address, "0x%08lX",
                                (unsigned long)(Offset + offset));
                        break;

                    case 3:  default:
                        sprintf(Address, "0x%016llX",
                                (unsigned long long)(Offset + offset));
                        break;
                }

                memset(Hex, ' ', 35);
                memset(Glyph, ' ', 16);
            }
        }
        else
        {
            Offset += size;
            Length -= size;

            fwrite(data, size, 1, stdout);
        }
    }

    munmap(buffer, length);

    return 0;
}
