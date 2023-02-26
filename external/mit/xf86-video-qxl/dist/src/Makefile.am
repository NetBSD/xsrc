#  Copyright 2008 Red Hat, Inc.
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
#  THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


# this is obnoxious:
# -module lets us name the module exactly how we want
# -avoid-version prevents gratuitous .0.0.0 version numbers on the end
# _ladir passes a dummy rpath to libtool so the thing will actually link
# TODO: -nostdlib/-Bstatic/-lgcc platform magic, not installing the .a, etc.

SUBDIRS = uxa spiceccid

AM_CFLAGS =					\
	$(SPICE_PROTOCOL_CFLAGS)		\
	$(XORG_CFLAGS)				\
	$(PCIACCESS_CFLAGS)			\
	$(CWARNFLAGS)				\
	$(DRM_CFLAGS)				\
	$(LIBUDEV_CFLAGS)

if BUILD_QXL
qxl_drv_la_LTLIBRARIES = qxl_drv.la
qxl_drv_la_LDFLAGS = -module -avoid-version
qxl_drv_ladir = @moduledir@/drivers

qxl_drv_la_LIBADD = uxa/libuxa.la
if LIBUDEV
qxl_drv_la_LIBADD += $(LIBUDEV_LIBS)
endif


qxl_drv_la_SOURCES =			\
	qxl.h				\
	qxl_driver.c			\
	qxl_image.c			\
	qxl_surface.c			\
	qxl_surface_ums.c		\
	qxl_surface.h			\
	qxl_ring.c			\
	qxl_mem.c			\
	mspace.c			\
	mspace.h			\
	murmurhash3.c			\
	murmurhash3.h			\
	qxl_cursor.c			\
	qxl_option_helpers.c		\
	qxl_option_helpers.h		\
	qxl_uxa.c			\
	qxl_ums_mode.c			\
	qxl_io.c			\
	dfps.c				\
	qxl_kms.c			\
	qxl_drmmode.c			\
	qxl_drmmode.h			\
	compat-api.h
endif

if BUILD_XSPICE
spiceqxl_drv_la_LTLIBRARIES = spiceqxl_drv.la
spiceqxl_drv_la_LDFLAGS = -module -avoid-version $(SPICE_LIBS)
spiceqxl_drv_ladir = @moduledir@/drivers

spiceqxl_drv_la_CFLAGS = -DXSPICE $(AM_CFLAGS) $(SPICE_CFLAGS)

spiceqxl_drv_la_LIBADD = uxa/libuxa.la $(XORG_LIBS)

spiceqxl_drv_la_SOURCES =		\
	qxl.h				\
	qxl_option_helpers.c		\
	qxl_option_helpers.h		\
	spiceqxl_util.h			\
	spiceqxl_util.c			\
	spiceqxl_spice_server.c		\
	spiceqxl_spice_server.h		\
	spiceqxl_io_port.c		\
	spiceqxl_io_port.h		\
	spiceqxl_driver.c		\
	spiceqxl_driver.h		\
	spiceqxl_main_loop.c		\
	spiceqxl_main_loop.h		\
	spiceqxl_display.c		\
	spiceqxl_display.h		\
	spiceqxl_vdagent.c		\
	spiceqxl_vdagent.h		\
	spiceqxl_uinput.c		\
	spiceqxl_uinput.h		\
	spiceqxl_audio.c		\
	spiceqxl_smartcard.c		\
	spiceqxl_smartcard.h		\
	spiceqxl_audio.h		\
	spiceqxl_inputs.c		\
	spiceqxl_inputs.h		\
	qxl_driver.c			\
	qxl_image.c			\
	qxl_surface.c			\
	qxl_surface_ums.c		\
	qxl_surface.h			\
	qxl_ring.c			\
	qxl_mem.c			\
	mspace.c			\
	mspace.h			\
	murmurhash3.c			\
	murmurhash3.h			\
	qxl_cursor.c			\
	dfps.c				\
	dfps.h				\
	qxl_uxa.c			\
	qxl_ums_mode.c			\
	qxl_io.c			\
	compat-api.h
endif
