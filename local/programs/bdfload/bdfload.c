/*	$NetBSD: bdfload.c,v 1.22 2024/03/12 09:36:06 macallan Exp $	*/

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
	{ "iso8859",	WSDISPLAY_FONTENC_ISO },
	{ "iso10646",	WSDISPLAY_FONTENC_ISO },
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
int dump = 0;
int header = 0;
int force = 0;
int scale = 0;
int smoothe = 0;
char commentbuf[2048] = "";
int commentptr = 0;
char fontname[64] = "";
char *names[256];

void
dump_line(char *gptr, int stride)
{
	int i, j, msk, c;

	for (i = 0; i < stride; i++) {
		c = gptr[i];
		msk = 0x80;
		for (j = 0; j < 8; j++) {
			putchar((c & msk) != 0 ? '#' : ' ');
			msk = msk >> 1;
		}
	}
	printf("\n");
}
 
void
write_wsf(const char *oname, struct wsdisplay_font *f)
{
	struct wsfthdr h;
	uint8_t *buffer = f->data;
	int buflen = f->numchars * f->stride * f->fontheight;

	memset(&h, 0, sizeof(h));
	strncpy(h.magic, "WSFT", sizeof(h.magic));
	strncpy(h.name, f->name, sizeof(h.name));
	h.firstchar = htole32(f->firstchar);
	h.numchars = htole32(f->numchars);
	h.encoding = htole32(f->encoding);
	h.fontwidth = htole32(f->fontwidth);
	h.fontheight = htole32(f->fontheight);
	h.stride = htole32(f->stride);
	h.bitorder = htole32(f->bitorder);
	h.byteorder = htole32(f->byteorder);

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

int
write_header(const char *filename, struct wsdisplay_font *f)
{
	FILE *output;
	uint8_t *buffer = f->data;
	uint8_t c, msk;
	int i, j, x, y, idx, pxls, left;
	char name[64];
	
	/* now output as a header file */
	snprintf(name, sizeof(name), "%s_%dx%d", f->name, 
	    f->fontwidth, f->fontheight);
	for (i = 0; i < strlen(name); i++) {
		if (isblank((unsigned char)name[i]))
			name[i] = '_';
	}
	if ((output = fopen(filename, "w")) == NULL) {
		warn("Can't open output file `%s'", filename);
		return -1;
	}
	if (commentptr > 0) {
		fprintf(output, "/*\n");
		fputs(commentbuf, output);
		fprintf(output, "*/\n\n");
	}

	fprintf(output, "static u_char %s_data[];\n", name);
	fprintf(output, "\n");
	fprintf(output, "static struct wsdisplay_font %s = {\n", name);
	fprintf(output, "\t\"%s\",\t\t\t/* typeface name */\n", f->name);
	fprintf(output, "\t%d,\t\t\t\t/* firstchar */\n", f->firstchar);
	fprintf(output, "\t%d,\t\t\t\t/* numchars */\n", f->numchars);
	fprintf(output, "\t%d,\t\t\t\t/* encoding */\n", f->encoding);
	fprintf(output, "\t%d,\t\t\t\t/* fontwidth */\n", f->fontwidth);
	fprintf(output, "\t%d,\t\t\t\t/* fontheight */\n", f->fontheight);
	fprintf(output, "\t%d,\t\t\t\t/* stride */\n", f->stride);
	fprintf(output, "\tWSDISPLAY_FONTORDER_L2R,\t/* bit order */\n");
	fprintf(output, "\tWSDISPLAY_FONTORDER_L2R,\t/* byte order */\n");
	fprintf(output, "\t%s_data\t\t/* data */\n", name);
	fprintf(output, "};\n\n");
	fprintf(output, "static u_char %s_data[] = {\n", name);
	for (i = 0; i < f->numchars; i++) {
		if (names[i] != NULL) {
			fprintf(output, "\t/* %d %s */\n", i + f->firstchar, names[i]);
		} else			
			fprintf(output, "\t/* %d */\n", i + f->firstchar);
		idx = i * f->stride * f->fontheight;
		for (y = 0; y < f->fontheight; y++) {
			for (x = 0; x < f->stride; x++) {
				fprintf(output, "0x%02x, ",buffer[idx + x]);
			}
			fprintf(output, "/* ");
			pxls = f->fontwidth;
			for (x = 0; x < f->stride; x++) {
				c = buffer[idx + x];
				msk = 0x80;
				left = pxls > 8 ? 8 : pxls;
				for (j = 0; j < left; j++) {
					fprintf(output, "%s",
					    (c & msk) != 0 ? "[]" : ". ");
					msk = msk >> 1;
				}
				pxls -= 8;
			}
			fprintf(output, " */\n");

			idx += f->stride;
		}
	}
	fprintf(output, "};\n");
	fclose(output);
	return 0;
}

void
double_pixels(uint8_t *inbuf, uint16_t *outbuf, int bytes)
{
	int i, j;
	uint16_t outmask, out;
	uint8_t in, inmask;

	for (i = 0; i < bytes; i++) {
		inmask = 0x80;
		outmask = 0xc000;
		out = 0;
		in = inbuf[i];
		for (j = 0; j < 8; j++) {
			if (in & inmask) {
				out |= outmask;
			}
			inmask = inmask >> 1;
			outmask = outmask >> 2;
		}
		outbuf[i * 2] = htobe16(out);
	}
}

void fill_dup(uint16_t *buf, int lines)
{
	int i;
	for (i = 0; i < lines; i++) {
		buf[2 * i + 1] = buf[2 * i];
	}
}

void smoothe_pixels(uint16_t *buf, int lines)
{
	int i, j, topright, topleft, botright, botleft;
	uint16_t pmask, in, prev, next, out;
	for (i = 0; i < lines; i++) {
		pmask = 0xc000;
		in = be16toh(buf[i]);
		out = in;
		prev = next = 0;
		if (i > 1) prev = be16toh(buf[i - 2]);
		if (i < (lines - 2)) next = be16toh(buf[i + 2]);
		for (j = 0; j < 8; j++) {
			if ((in & pmask) == 0) {
				/* empty pixel, check surroundings */
				topright = topleft = botright = botleft = 0;
				if (((i & 1) == 0) && (j < 6))
					topright = (((prev & pmask) == pmask) &&
						    ((prev & (pmask >> 2)) != 0) &&
						    ((in & (pmask >> 2)) != 0));
				if (((i & 1) == 0) && (j > 0))
					topleft = (((prev & pmask) == pmask) &&
						    ((prev & (pmask << 2)) != 0) &&
						    ((in & (pmask << 2)) != 0));
				if ((i & 1) && (j < 6))
					botright = (((next & pmask) == pmask) &&
						    ((next & (pmask >> 2)) != 0) &&
						    ((in & (pmask >> 2)) != 0));
				if ((i & 1) && (j > 0))
					botleft = (((next & pmask) == pmask) &&
						    ((next & (pmask << 2)) != 0) &&
						    ((in & (pmask << 2)) != 0));
				if ((topright + topleft + botright + botleft) == 1) {
					if (topleft || botleft) out |= pmask << 1;
					if (topright || botright) out |= pmask >> 1;
				}
			}
			pmask = pmask >> 2;
		}
		buf[i] = htobe16(out);
	}
}

void
interpret(FILE *foo)
{
	char line[128], *arg, name[64] = "foo", *buffer, *cbitmap;
	char charname[65], *charnamebuf;
	int buflen = -1, charnamebufptr = 0, j;
	int in_char = 0, current = -1, stride = 0, charsize = 0;
	int width, height, x, y, num;
	int first = 255, last = 0;
	int left, top, lines;
	int bl = 255, bt = 255, br = -1, bb = -1;
	struct wsdisplay_font f;
	int status;

	charnamebuf = malloc(64 * 256);
	if (charnamebuf == 0) err(EXIT_FAILURE, "failed to allocate memory\n");
	memset(charnamebuf, 0, 64 * 256);
	for (j = 0; j < 256; j++) names[j] = NULL;

	while (fgets(line, sizeof(line), foo) != NULL) {
		size_t i = 0, len;
		/* separate keyword from parameters */
		len = strlen(line);
		while (!isspace((unsigned char)line[i]) && i < len) i++;
		line[i] = 0;
		arg = &line[i + 1];
		i = 0;
		len = strlen(arg);
		/* get rid of garbage */
		while ((!iscntrl((unsigned char)arg[i])) && (arg[i] != 0)) {
			i++;
		}
		arg[i] = 0;
		if (strcmp(line, "FAMILY_NAME") == 0) {
			char *q;
			/* cut off quotation marks */
			strlcpy(name, arg + 1, 64);
			/* remove trailing " */
			if ((q = strnstr(name, "\"", 64)) != NULL)
				*q = 0;
			if (verbose) printf("name: %s\n", name);
		} else if (strcmp(line, "COMMENT") == 0) {
			commentptr += snprintf(&commentbuf[commentptr],
			    sizeof(commentbuf) - commentptr, "%s\n", arg);
		} else if (strcmp(line, "SPACING") == 0) {
			char spc[16];
			int res;
			res = sscanf(arg, "%s", spc);
			if (res > 0) {
				if (verbose) printf("spacing %s\n", spc);
				if ((spc[1] == 'P') && (force == 0)) {
					warnx("This is a proportional font, "
					   "results are probably not suitable "
					   "for console use.");
					errx(EXIT_FAILURE, "Use -f to override "
					    "if you want to try it anyway.");
				}
			}
		} else if (strcmp(line, "FONTBOUNDINGBOX") == 0) {
			int res;
			res = sscanf(arg, "%d %d %d %d",
					  &width, &height, &x, &y);
			stride = (width + 7) >> 3;
			if (verbose) printf("box %d x %d\n", width, height);
			if (stride > 2) {
				errx(EXIT_FAILURE,
				    "no fonts wider than 16 work for now\n");
			}
			charsize = height * stride;
			buflen = 257 * charsize;
			buffer = calloc(1, buflen);
			if (buffer == NULL) {
				err(EXIT_FAILURE, 
				    "failed to allocate %dKB for glyphs\n",
				    buflen);
			}
			cbitmap = buffer + 256 * charsize;
		} else if (strcmp(line, "CHARS") == 0) {
			if (sscanf(arg, "%d", &num) == 1)
				if (verbose) 
				    printf("number of characters: %d\n", num);
		} else if (strcmp(line, "STARTCHAR") == 0) {
			in_char = 1;
			if (charsize <= 1) err(EXIT_FAILURE,
			    "syntax error - no valid FONTBOUNDINGBOX\n");
			memset(cbitmap, 0, charsize);
			strlcpy(charname, arg, 64);
			if (dump && (strlen(charname) > 0))
				printf("name: %s\n", charname);

		} else if (strcmp(line, "ENDCHAR") == 0) {
			in_char = 0;
			/* only commit the glyph if it's in range */
			if ((current >= 0) && (current < 256)) {
				memcpy(&buffer[charsize * current],
				    cbitmap, charsize);
				if ((strlen(charname) > 0) &&
				    (charnamebufptr < 255 * 64)) {
				    	char *cur;
					int len;
					/* copy name into buffer, keep a
					 * pointer to it for later */
					cur = &charnamebuf[charnamebufptr];					
					len = strlcpy(cur, charname, 64);
					charnamebufptr += len + 1;
					names[current] = cur;
				}
			}
			current = -1;
		} else if (strcmp(line, "ENCODING") == 0) {
			if (sscanf(arg, "%d", &current) == 1) {
				if (current >= 0 && current < 256) {
					if (current < first) first = current;
					if (current > last) last = current;
					if (dump) printf("glyph %d\n", current);
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
				if (dump && verbose)
					printf("top %d left %d\n", top, left);
			}
		} else if (strcmp(line, "BITMAP") == 0) {
			int i, j, k, l;
			char num[32];
			char *gptr = cbitmap;
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
					*bptr = l;
					bptr++;
				} else {
					*bptr16 = htobe16(l);
					bptr16++;
				}
			}
			if (dump) {
				gptr = cbitmap;
				for (i = 0; i < height; i++) {
					dump_line(gptr, stride);
					gptr += stride;
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
	f.firstchar = first;
	f.numchars = last - first + 1;
	f.encoding = encoding;
	if (fontname[0] == 0) {
		f.name = name;
	} else f.name = fontname;
	f.bitorder = WSDISPLAY_FONTORDER_L2R;
	f.byteorder = WSDISPLAY_FONTORDER_L2R;

	if (scale) {
		uint16_t *outbuf;
		uint8_t *inbuf;
		int i;

		if (stride != 1) err(EXIT_FAILURE,
		    "scaling works only on fonts up to 8 pixels wide\n");
		f.fontwidth = width * 2 /*(width + 3) & ~3*/;
		f.fontheight = height * 2;
		f.stride = stride * 2;
		outbuf = calloc(1, f.numchars * charsize * 4);
		if (outbuf == NULL) err(EXIT_FAILURE, 
		    "failed to allocete memory for scale buffer\n");
		f.data = outbuf;
		inbuf = &buffer[first * charsize];
		for (i = 0; i < f.numchars; i++) {
			double_pixels(inbuf, outbuf, charsize);
			fill_dup(outbuf, charsize);
			if (smoothe) smoothe_pixels(outbuf, charsize * 2);
			inbuf += charsize;
			outbuf += charsize * 2;
		}
		
	} else {
		f.fontwidth = width /*(width + 3) & ~3*/;
		f.fontheight = height;
		f.stride = stride;
		f.data = &buffer[first * charsize];
	}
if (0) {
	int i;
	uint16_t pixbuf[16];
	double_pixels(&buffer[charsize * 'Q'], pixbuf, charsize);
	fill_dup(pixbuf, charsize);
	for (i = 0; i < charsize * 2; i++) {	
		printf("%2d: ", i);
		dump_line((char *)&pixbuf[i], 2); 
	}
	smoothe_pixels(pixbuf, charsize * 2);
	for (i = 0; i < charsize * 2; i++) {	
		printf("%2d: ", i);
		dump_line((char *)&pixbuf[i], 2); 
	}
}

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
		if (header == 0)
			write_wsf(ofile, &f);
		else
			write_header(ofile, &f);
	}
}

__dead void
usage()
{
	fprintf(stderr, "Usage: %s [-vdhf2s] [-e encoding] [-N name] "
	    "[-o ofile.wsf] font.bdf\n", getprogname());
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	FILE *foo;
	const char *encname = NULL;

	int c;
	while ((c = getopt(argc, argv, "e:o:N:vdhf2s")) != -1) {
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

		case 'd':
			dump = 1;
			break;

		case 'h':
			header = 1;
			break;
		case 'f':
			force = 1;
			break;
		case '2':
			scale = 1;
			break;
		case 's':
			smoothe = 1;
			break;
		case 'N':
			strncpy(fontname, optarg, 64);
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
