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
AM_CFLAGS = @XORG_CFLAGS@ @LIBUDEV_CFLAGS@ @LIBDRM_NOUVEAU_CFLAGS@ @LIBDRM_CFLAGS@
nouveau_drv_la_LTLIBRARIES = nouveau_drv.la
nouveau_drv_la_LDFLAGS = -module -avoid-version @LIBDRM_NOUVEAU_LIBS@ \
			 @LIBUDEV_LIBS@ @LIBDRM_LIBS@
nouveau_drv_ladir = @moduledir@/drivers

nouveau_drv_la_SOURCES = \
			 nouveau_copy.c \
			 nouveau_copy85b5.c \
			 nouveau_copy90b5.c \
			 nouveau_copya0b5.c \
			 nouveau_exa.c nouveau_xv.c nouveau_dri2.c \
			 nouveau_present.c \
			 nouveau_sync.c \
			 nouveau_wfb.c \
			 nv_accel_common.c \
			 nv_driver.c \
			 nv_shadow.c \
			 nv04_exa.c \
			 nv04_xv_ovl.c \
			 nv04_xv_blit.c \
			 nv10_exa.c \
			 nv10_xv_ovl.c \
			 nv30_exa.c \
			 nv30_xv_tex.c \
			 nv40_exa.c \
			 nv40_xv_tex.c \
			 nv50_accel.c \
			 nv50_exa.c \
			 nv50_xv.c \
			 nvc0_accel.c \
			 nvc0_exa.c \
			 nvc0_xv.c \
			 drmmode_display.c \
			 vl_hwmc.c

EXTRA_DIST = hwdefs/nv_3ddefs.xml.h \
	     hwdefs/nv_m2mf.xml.h \
	     hwdefs/nv_object.xml.h \
	     hwdefs/nv01_2d.xml.h \
	     hwdefs/nv10_3d.xml.h \
	     hwdefs/nv30-40_3d.xml.h \
	     hwdefs/nv50_2d.xml.h \
	     hwdefs/nv50_3d.xml.h \
	     hwdefs/nv50_defs.xml.h \
	     hwdefs/nv50_texture.h \
	     hwdefs/nvc0_3d.xml.h \
	     hwdefs/nvc0_m2mf.xml.h \
	     shader/exac8nvc0.fp \
	     shader/exac8nvc0.fpc \
	     shader/exac8nve0.fp \
	     shader/exac8nve0.fpc \
	     shader/exac8nvf0.fp \
	     shader/exac8nvf0.fpc \
	     shader/exacanvc0.fp \
	     shader/exacanvc0.fpc \
	     shader/exacanve0.fp \
	     shader/exacanve0.fpc \
	     shader/exacanvf0.fp \
	     shader/exacanvf0.fpc \
	     shader/exacmnvc0.fp \
	     shader/exacmnvc0.fpc \
	     shader/exacmnve0.fp \
	     shader/exacmnve0.fpc \
	     shader/exacmnvf0.fp \
	     shader/exacmnvf0.fpc \
	     shader/exas8nvc0.fp \
	     shader/exas8nvc0.fpc \
	     shader/exas8nve0.fp \
	     shader/exas8nve0.fpc \
	     shader/exas8nvf0.fp \
	     shader/exas8nvf0.fpc \
	     shader/exasanvc0.fp \
	     shader/exasanvc0.fpc \
	     shader/exasanve0.fp \
	     shader/exasanve0.fpc \
	     shader/exasanvf0.fp \
	     shader/exasanvf0.fpc \
	     shader/exascnvc0.fp \
	     shader/exascnvc0.fpc \
	     shader/exascnve0.fp \
	     shader/exascnve0.fpc \
	     shader/exascnvf0.fp \
	     shader/exascnvf0.fpc \
	     shader/videonvc0.fp \
	     shader/videonvc0.fpc \
	     shader/videonve0.fp \
	     shader/videonve0.fpc \
	     shader/videonvf0.fp \
	     shader/videonvf0.fpc \
	     shader/xfrm2nvc0.vp \
	     shader/xfrm2nvc0.vpc \
	     shader/xfrm2nve0.vp \
	     shader/xfrm2nve0.vpc \
	     shader/xfrm2nvf0.vp \
	     shader/xfrm2nvf0.vpc \
	     shader/Makefile \
	     nouveau_local.h \
	     nouveau_copy.h \
	     nouveau_present.h \
	     nouveau_sync.h \
	     nv_const.h \
	     nv_dma.h \
	     nv_include.h \
	     nv_proto.h \
	     nv_rop.h \
	     nv_type.h \
	     nv04_accel.h \
	     nv50_accel.h \
	     nvc0_accel.h \
	     compat-api.h \
	     vl_hwmc.c \
	     vl_hwmc.h



