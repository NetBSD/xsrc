#  Copyright 2005 Adam Jackson.
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  on the rights to use, copy, modify, merge, publish, distribute, sub
#  license, and/or sell copies of the Software, and to permit persons to whom
#  the Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice (including the next
#  paragraph) shall be included in all copies or substantial portions of the
#  Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
#  ADAM JACKSON BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

# this is obnoxious:
# -module lets us name the module exactly how we want
# -avoid-version prevents gratuitous .0.0.0 version numbers on the end
# _ladir passes a dummy rpath to libtool so the thing will actually link
# TODO: -nostdlib/-Bstatic/-lgcc platform magic, not installing the .a, etc.
AM_CFLAGS = @XORG_CFLAGS@ @DRI_CFLAGS@
xgi_drv_la_LTLIBRARIES = xgi_drv.la
xgi_drv_la_LDFLAGS = -module -avoid-version
xgi_drv_ladir = @moduledir@/drivers

xgi_drv_la_SOURCES = \
	init.c \
	init.h \
	osdef.h \
	valid_mode.h \
	vb_def.h \
	vb_ext.c \
	vb_ext.h \
	vb_init.c \
	vb_init.h \
	vb_setmode.c \
	vb_setmode.h \
	vb_struct.h \
	vb_table.h \
	vgatypes.h \
	xgi_accel.c \
	xgi_accel.h \
	xgi_common.h \
	xgi_cursor.c \
	xgi_cursor.h \
	xgi_dac.c \
	xgi_dac.h \
	xgi_dga.c \
	xgi_driver.c \
	xgi_driver.h \
	xgi.h \
	xgi_opt.c \
	xgi_pci.h \
	xgi_regs.h \
	xgi_setup.c \
	xgi_vb.c \
	xgi_vb.h \
	xgi_vga.c \
	xgi_video.c \
	xgi_video.h \
	xgi_videohw.c \
	xgi_videohw.h

if DRI
xgi_drv_la_SOURCES += \
	xgi_dri.c \
	xgi_dri.h 
endif
