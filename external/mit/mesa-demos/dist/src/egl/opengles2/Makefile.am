# Copyright © 2009 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.
#
# Authors:
#    Eric Anholt <eric@anholt.net>

AM_CFLAGS = \
	$(GLESV2_CFLAGS) \
	$(EGL_CFLAGS) \
	$(X11_CFLAGS) \
	-I$(top_srcdir)/src/egl/eglut

AM_LDFLAGS = \
	$(GLESV2_LIBS) \
	$(EGL_LIBS) \
	-lm

if HAVE_EGL
if HAVE_GLESV2
bin_PROGRAMS = \
	es2_info \
	es2gears_screen \
	es2gears_x11 \
	es2tri
if HAVE_WAYLAND
bin_PROGRAMS += es2gears_wayland
endif
endif
endif

es2_info_LDADD = $(X11_LIBS)
es2tri_LDADD = $(X11_LIBS)

es2gears_screen_SOURCES = es2gears.c

es2gears_x11_SOURCES = es2gears.c

es2gears_screen_LDADD = ../eglut/libeglut_screen.la

es2gears_x11_LDADD = ../eglut/libeglut_x11.la

es2gears_wayland_SOURCES = es2gears.c
es2gears_wayland_LDADD = ../eglut/libeglut_wayland.la
