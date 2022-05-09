/*	$NetBSD: bdfload.c,v 1.2 2022/05/09 15:47:27 uwe Exp $	*/

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

void
interpret(FILE *foo)
{
	char line[128], *arg, name[64] = "foop", *buffer, buflen = -1;
	int len, in_char = 0, current = -1, stride = 0, charsize = 0;
	int width, height, x, y, num;
	int first = 255, last = 0;
	int left, top, lines;
	int bl = 255, bt = 255, br = -1, bb = -1;
	struct wsdisplay_font f;
	int fdev;

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
			printf("name: %s\n", name);
		} else if (strcmp(line, "FONTBOUNDINGBOX") == 0) {
			int res;
			res = sscanf(arg, "%d %d %d %d",
					  &width, &height, &x, &y);
			stride = (width + 7) >> 3;
			printf("box %d x %d\n", width, height);
			if (stride > 2) {
				printf("no fonts wider than 16 work for now\n");
				exit(1);
			}
			charsize = height * stride;
			buflen = 256 * charsize;
			buffer = malloc(buflen);
			if (buffer == NULL) {
				printf("failed to allocate %dKB for glyphs\n",
				    buflen);
				exit(1);
			}
		} else if (strcmp(line, "CHARS") == 0) {
			if (sscanf(arg, "%d", &num) == 1)
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
	printf("range %d to %d\n", first, last);
	printf("actual box: %d %d %d %d\n", bl, bt, br, bb);
	/* now stuff it into a something wsfont understands */
	f.fontwidth = width /*(width + 3) & ~3*/;
	f.fontheight = height;
	f.firstchar = first;
	f.numchars = last - first;
	f.stride = stride;
	f.encoding = WSDISPLAY_FONTENC_ISO;
	f.name = name;
	f.bitorder = WSDISPLAY_FONTORDER_L2R;
	f.byteorder = WSDISPLAY_FONTORDER_L2R;
	f.data = &buffer[first * charsize];

	fdev = open("/dev/wsfont", O_RDWR, 0);
	if (fdev < 0) errx(1, "unable to open /dev/wsfont");
	ioctl(fdev, WSDISPLAYIO_LDFONT, &f);
	close(fdev);
}

void
main(int argc, char *argv[])
{
	FILE *foo;
	if (argc > 1) {
		foo = fopen(argv[1], "r");
		if (foo == NULL) {
			printf("fopen error %d\n", errno);
			return;
		}
		interpret(foo);
	} else {
		printf("usage: bdfload <arg>\n");
	}
}
