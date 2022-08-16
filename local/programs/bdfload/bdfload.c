/*	$NetBSD: bdfload.c,v 1.4 2022/08/16 20:27:33 macallan Exp $	*/

/*
 * Copyright (c) 2018 Michael Lorenz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * a crude BDF loader for wsdisplay
 */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <err.h>

#include <dev/wscons/wsconsio.h>

/*
 * wsdisplay_font but with strings embedded and integer fields in
 * little endian
 */
struct wsfthdr {
	char magic[4];		/* "WSFT" */
	char name[64];
	uint32_t firstchar;
	uint32_t numchars;
	uint32_t encoding;
	uint32_t fontwidth;
	uint32_t fontheight;
	uint32_t stride;
	uint32_t bitorder;
	uint32_t byteorder;
};


const struct encmap {
	const char *name;
	int encoding;
} encmap[] = {
	{ "cp437",	WSDISPLAY_FONTENC_IBM },
	{ "ibm",	WSDISPLAY_FONTENC_IBM },
	{ "iso",	WSDISPLAY_FONTENC_ISO },
	{ "iso-8859-1",	WSDISPLAY_FONTENC_ISO },
	{ "iso-8859-2",	WSDISPLAY_FONTENC_ISO2 },
	{ "iso-8859-7",	WSDISPLAY_FONTENC_ISO7 },
	{ "iso2",	WSDISPLAY_FONTENC_ISO2 },
	{ "iso7",	WSDISPLAY_FONTENC_ISO7 },
	{ "iso8859-1",	WSDISPLAY_FONTENC_ISO },
	{ "iso8859-2",	WSDISPLAY_FONTENC_ISO2 },
	{ "iso8859-7",	WSDISPLAY_FONTENC_ISO7 },
	{ "koi8-r",	WSDISPLAY_FONTENC_KOI8_R },
	{ "koi8r",	WSDISPLAY_FONTENC_KOI8_R },
	{ "latin-1",	WSDISPLAY_FONTENC_ISO },
	{ "latin-2",	WSDISPLAY_FONTENC_ISO2 },
	{ "latin1",	WSDISPLAY_FONTENC_ISO },
	{ "latin2",	WSDISPLAY_FONTENC_ISO2 },
	{ "pcvt",	WSDISPLAY_FONTENC_PCVT },
	{ NULL, -1 }
};

const char * const encname[] = {
#define _ENC(_e) [_e] = #_e
	_ENC(WSDISPLAY_FONTENC_ISO),
	_ENC(WSDISPLAY_FONTENC_IBM),
	_ENC(WSDISPLAY_FONTENC_PCVT),
	_ENC(WSDISPLAY_FONTENC_ISO7),
	_ENC(WSDISPLAY_FONTENC_ISO2),
	_ENC(WSDISPLAY_FONTENC_KOI8_R),
};


const char *ofile = NULL;
int encoding = -1;
int verbose = 0;


void
interpret(FILE *foo)
{
	char line[128], *arg, name[64] = "foop", *buffer;
	int buflen = -1;
	int len, in_char = 0, current = -1, stride = 0, charsize = 0;
	int width, height, x, y, num;
	int first = 255, last = 0;
	int left, top, lines;
	int bl = 255, bt = 255, br = -1, bb = -1;
	struct wsdisplay_font f;
	int status;

	while (fgets(line, sizeof(line), foo) != NULL) {
		int i = 0;
		/* separate keyword from parameters */
		len = strlen(line);
		while (!isspace(line[i]) && (i < len)) i++;
		line[i] = 0;
		arg = &line[i + 1];
		i = 0;
		len = strlen(arg);
		/* get rid of garbage */
		while ((!iscntrl(arg[i])) && (arg[i] != 0)) {
			i++;
		}
		arg[i] = 0;
		if (strcmp(line, "FAMILY_NAME") == 0) {
			/* cut off quotation marks */
			strncpy(name, arg + 1, 64);
			name[strlen(name) - 1] = 0;
			if (verbose) printf("name: %s\n", name);
		} else if (strcmp(line, "FONTBOUNDINGBOX") == 0) {
			int res;
			res = sscanf(arg, "%d %d %d %d",
					  &width, &height, &x, &y);
			stride = (width + 7) >> 3;
			if (verbose) printf("box %d x %d\n", width, height);
			if (stride > 2) {
				err(EXIT_FAILURE,
				    "no fonts wider than 16 work for now\n");
			}
			charsize = height * stride;
			buflen = 256 * charsize;
			buffer = calloc(1, buflen);
			if (buffer == NULL) {
				err(EXIT_FAILURE, 
				    "failed to allocate %dKB for glyphs\n",
				    buflen);
			}
		} else if (strcmp(line, "CHARS") == 0) {
			if (sscanf(arg, "%d", &num) == 1)
				if (verbose) 
				    printf("number of characters: %d\n", num);
		} else if (strcmp(line, "STARTCHAR") == 0) {
			in_char = 1;
		} else if (strcmp(line, "ENDCHAR") == 0) {
			in_char = 0;
			current = -1;
		} else if (strcmp(line, "ENCODING") == 0) {
			if (sscanf(arg, "%d", &current) == 1) {
				if (current >= 0 && current < 256) {
					if (current < first) first = current;
					if (current > last) last = current;
				}
			}
		} else if (strcmp(line, "BBX") == 0) {
			int cx, cy, cwi, che;
			if (sscanf(arg, "%d %d %d %d", &cwi, &che, &cx, &cy)
			     == 4) {
				left = cx;
				lines = che;
				top = height + y - che - cy;
				if (left < bl) bl = left;
				if (top < bt) bt = top;
				if ((left + cwi) > br) br = left + cwi;
				if ((top + che) > bb) bb = top + che;
			}
		} else if (strcmp(line, "BITMAP") == 0) {
			int i, j, k, l;
			char num[32];
			char *gptr = &buffer[charsize * current];
			char *bptr = gptr + top;
			uint16_t *bptr16 = (uint16_t *)gptr;
			bptr16 += top;
			/* see if the character is in range */
			if ((current < 0) || (current > 255)) continue;
			/* now we read & render the character */
			for (i = 0; i < lines; i++) {
				fgets(num, 32, foo);
				sscanf(num, "%x", &l);
				if ((stride) == 2 && (strlen(num) < 4))
					l = l << 8;
				l = l >> left;
				if (stride == 1) {
					*gptr = l;
					gptr++;
				} else {
					*bptr16 = htobe16(l);
					bptr16++;
				}
			}
		}
	}
	if (verbose) {
		printf("range %d to %d\n", first, last);
		printf("encoding: %s\n", encname[encoding]);
		printf("actual box: %d %d %d %d\n", bl, bt, br, bb);
	}

	/* now stuff it into a something wsfont understands */
	f.fontwidth = width /*(width + 3) & ~3*/;
	f.fontheight = height;
	f.firstchar = first;
	f.numchars = last - first + 1;
	f.stride = stride;
	f.encoding = encoding;
	f.name = name;
	f.bitorder = WSDISPLAY_FONTORDER_L2R;
	f.byteorder = WSDISPLAY_FONTORDER_L2R;
	f.data = &buffer[first * charsize];

	if (ofile == NULL) {
		int fdev = open("/dev/wsfont", O_RDWR, 0);
		if (fdev < 0)
			err(EXIT_FAILURE, "/dev/wsfont");
		status = ioctl(fdev, WSDISPLAYIO_LDFONT, &f);
		if (status != 0)
			err(EXIT_FAILURE, "WSDISPLAYIO_LDFONT");
		close(fdev);
	}
	else {
		struct wsfthdr h;

		memset(&h, 0, sizeof(h));
		strncpy(h.magic, "WSFT", sizeof(h.magic));
		strncpy(h.name, f.name, sizeof(h.name));
		h.firstchar = htole32(f.firstchar);
		h.numchars = htole32(f.numchars);
		h.encoding = htole32(f.encoding);
		h.fontwidth = htole32(f.fontwidth);
		h.fontheight = htole32(f.fontheight);
		h.stride = htole32(f.stride);
		h.bitorder = htole32(f.bitorder);
		h.byteorder = htole32(f.byteorder);

		int wsfd = open(ofile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (wsfd < 0)
			err(EXIT_FAILURE, "%s", ofile);

		ssize_t nwritten;
		nwritten = write(wsfd, &h, sizeof(h));
		if (nwritten < 0)
			err(EXIT_FAILURE, "%s", ofile);
		if (nwritten != sizeof(h))
			errx(EXIT_FAILURE, "%s: partial write", ofile);

		nwritten = write(wsfd, buffer, buflen);
		if (nwritten < 0)
			err(EXIT_FAILURE, "%s", ofile);
		if (nwritten != buflen)
			errx(EXIT_FAILURE, "%s: partial write", ofile);
		close(wsfd);
	}
}


__dead void
usage()
{
	fprintf(stderr, "usage: bdfload [-v] [-e encoding] [-o ofile.wsf] font.bdf\n");
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	FILE *foo;
	const char *encname = NULL;

	int c;
	while ((c = getopt(argc, argv, "e:o:v")) != -1) {
		switch (c) {

		/* font encoding */
		case 'e':
			if (encname != NULL)
				usage();
			encname = optarg;
			break;

		/* output file name */
		case 'o':
			if (ofile != NULL)
				usage();
			ofile = optarg;
			break;

		case 'v':
			verbose = 1;
			break;

		case '?':	/* FALLTHROUGH */
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (encname == NULL) {
		encoding = WSDISPLAY_FONTENC_ISO;
	}
	else {
		for (const struct encmap *e = encmap; e->name; ++e) {
			if (strcmp(e->name, encname) == 0) {
				encoding = e->encoding;
				break;
			}
		}
	}

	/* get encoding from the bdf file? */
	if (encoding == -1)
		encoding = WSDISPLAY_FONTENC_ISO;

	if (argc == 0)
		usage();

	const char *bdfname = argv[0];
	foo = fopen(bdfname, "r");
	if (foo == NULL)
		err(EXIT_FAILURE, "%s", bdfname);

	interpret(foo);
	return EXIT_SUCCESS;
}
