/*	$NetBSD: main.c,v 1.3 2017/06/23 02:15:07 macallan Exp $	*/

/*
 * Copyright (c) 2011 Michael Lorenz
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

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dev/wscons/wsconsio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

char cvr[] = " .-+oaOX";
FT_Library library;
FT_Face face;
int baseline, above = 0, below = 0, advance = 0;

int push_size(int);

int
push_size(int letter)
{
	int glyph_index, error;
	int new_above, new_below, new_advance;

	glyph_index = FT_Get_Char_Index(face, letter);
	printf("idx: %d\n", glyph_index);
	error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
	if (error) {
		printf("wtf?!\n");
		return -1;
	}
	FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	printf("%d x %d\n", face->glyph->bitmap.width, face->glyph->bitmap.rows);
	printf("offset: %d %d\n", face->glyph->bitmap_left, face->glyph->bitmap_top);
	new_advance = (int)(face->glyph->advance.x >> 6);
	printf("advance: %d\n", new_advance);
	new_above = face->glyph->bitmap_top;
	new_below = face->glyph->bitmap.rows - face->glyph->bitmap_top;
	if (new_above > above) above = new_above;
	if (new_below > below) below = new_below;
	if (new_advance > advance) advance = new_advance;
	return 0;
}

int
main(int argc, char *argv[])
{
	int error, glyph_index;
	int x, y, idx, width_in_bytes, height = 22, cell_height;
	int width, datalen, didx, i, start, end, out;
	FILE *output;
	uint8_t *fontdata;
	char fontname[128], filename[128];

	if (argc != 3) {
		printf("usage: ttf2wsfont some_font.ttf height\n");
		return 0;
	}

	sscanf(argv[2], "%d", &height);

	error = FT_Init_FreeType( &library );
	if (error) {
		printf("Failed to initialize freefont2\n");
		return -1;
	}
	error = FT_New_Face(library, argv[1], 0, &face );
	if ( error == FT_Err_Unknown_File_Format ) {
		printf("unsupported font format\n");
		return -1;
	} 
	error = FT_Set_Pixel_Sizes(face, /* handle to face object */
				   0,    /* pixel_width */
				   height - (height / 10) ); /* pixel_height */ 
	if (error) {
		printf("couldn't set character cell size\n");
	}

	push_size('W');
	push_size('g');
	push_size(192);
	printf("above: %d below: %d advance: %d\n", above, below, advance);
	width = advance;
	baseline = above;
	cell_height = above + below;
	datalen = 256 * width * cell_height;
	fontdata = malloc(datalen);


	for (i = 0; i < 256; i++) {
		glyph_index = FT_Get_Char_Index(face, i);
		FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		width_in_bytes =  face->glyph->bitmap.width;
		start = baseline - face->glyph->bitmap_top;
		end = start + face->glyph->bitmap.rows;
		if (end > cell_height)
			end = cell_height;
		idx = 0;
		if (start < 0) {
			idx += (0 - start) * width_in_bytes;
			start = 0;
		}
		didx = i * width * cell_height + /* character cell */
		       start * width + /* pixels above baseline */ 
		       face->glyph->bitmap_left; /* pixels left from border */
		memset(&fontdata[i * width * cell_height], 0, width * cell_height);
		for (y = start; y < end; y++) {
			for (x = 0; x < width_in_bytes; x++) {
				fontdata[didx + x] = face->glyph->bitmap.buffer[idx + x];
			}
			idx += width_in_bytes;
			didx += width;
		}
	}

	/* now output as a header file */
	snprintf(fontname, 128, "%s_%dx%d", face->family_name, width, cell_height);
	for (i = 0; i < strlen(fontname); i++) {
		if (isblank((int)fontname[i]))
			fontname[i]='_';
	}
	snprintf(filename, 128, "%s.h", fontname);
	if ((output = fopen(filename, "w")) == NULL) {
		fprintf(stderr, "Can't open output file %s\n", filename);
		return -1;
	}
	fprintf(output, "static u_char %s_data[];\n", fontname);
	fprintf(output, "\n");
	fprintf(output, "static struct wsdisplay_font %s = {\n", fontname);
	fprintf(output, "\t\"%s\",\t\t\t/* typeface name */\n", face->family_name);
	fprintf(output, "\t32,\t\t\t\t/* firstchar */\n");
	fprintf(output, "\t256 - 32,\t\t\t/* numchar */\n");
	fprintf(output, "\tWSDISPLAY_FONTENC_ISO,\t\t/* encoding */\n");
	fprintf(output, "\t%d,\t\t\t\t/* width */\n", width);
	fprintf(output, "\t%d,\t\t\t\t/* height */\n", cell_height);
	fprintf(output, "\t%d,\t\t\t\t/* stride */\n", width);
	fprintf(output, "\tWSDISPLAY_FONTORDER_L2R,\t/* bit order */\n");
	fprintf(output, "\tWSDISPLAY_FONTORDER_L2R,\t/* byte order */\n");
	fprintf(output, "\t%s_data\t\t/* data */\n", fontname);
	fprintf(output, "};\n\n");
	fprintf(output, "static u_char %s_data[] = {\n", fontname);
	for (i = 32; i < 256; i++) {
		fprintf(output, "\t/* %d */\n", i);
		idx = i * width * cell_height;
		for (y = 0; y < cell_height; y++) {
			for (x = 0; x < width; x++) {
				fprintf(output, "0x%02x, ",fontdata[idx + x]);
			}
			fprintf(output, "/* ");
			for (x = 0; x < width; x++) {
				fprintf(output, "%c",cvr[fontdata[idx + x] >> 5]);
			}
			fprintf(output, " */\n");

			idx += width;
		}
	}
	fprintf(output, "};\n");
	fclose(output);
	/* dump as binary */
	snprintf(filename, 128, "%s.wsf", fontname);	
	if ((out = open(filename, O_RDWR | O_CREAT | O_TRUNC, DEFFILEMODE)) > 0) {
		char nbuf[64];
		uint32_t foo;
		write(out, "WSFT", 4);
		memset(nbuf, 0, 64);
		strncpy(nbuf, face->family_name, 64);
		write(out, nbuf, 64);
		/* firstchar */
		foo = htole32(32);
		write(out, &foo, 4);
		/* numchar */
		foo = htole32(256 - 32);
		write(out, &foo, 4);
		/* encoding */
		foo = htole32(WSDISPLAY_FONTENC_ISO);
		write(out, &foo, 4);
		/* fontwidth */
		foo = htole32(width);
		write(out, &foo, 4);
		/* fontheight */
		foo = htole32(cell_height);
		write(out, &foo, 4);
		/* stride */
		foo = htole32(width);
		write(out, &foo, 4);
		/* bitorder */
		foo = htole32(WSDISPLAY_FONTORDER_L2R);
		write(out, &foo, 4);
		/* byteorder */
		foo = htole32(WSDISPLAY_FONTORDER_L2R);
		write(out, &foo, 4);
		/* now the font data */
		write(out, fontdata + (32 * width * cell_height),
		           (256 - 32) * width * cell_height);
		close(out);
	}
	free(fontdata);
	FT_Done_Face(face);
	FT_Done_FreeType(library);
}
