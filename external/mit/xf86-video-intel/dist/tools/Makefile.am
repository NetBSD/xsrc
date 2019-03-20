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

AM_CFLAGS = \
	@CWARNFLAGS@ \
	@NOWARNFLAGS@ \
	$(NULL)

drivermandir = $(DRIVER_MAN_DIR)
policydir = $(datarootdir)/polkit-1/actions

bin_PROGRAMS =
noinst_PROGRAMS =
libexec_PROGRAMS =

if BUILD_TOOLS
bin_PROGRAMS += intel-virtual-output
driverman_DATA = intel-virtual-output.$(DRIVER_MAN_SUFFIX)
endif

if BUILD_TOOL_CURSOR
noinst_PROGRAMS += cursor
cursor_CFLAGS = $(TOOL_CURSOR_CFLAGS)
cursor_LDADD = $(TOOL_CURSOR_LIBS)
endif

if X11_DRI3
noinst_PROGRAMS += dri3info
dri3info_SOURCES = dri3info.c
dri3info_CFLAGS = $(X11_DRI3_CFLAGS) $(DRI_CFLAGS)
dri3info_LDADD = $(X11_DRI3_LIBS) $(DRI_LIBS)
endif

if BUILD_BACKLIGHT_HELPER
libexec_PROGRAMS += xf86-video-intel-backlight-helper
nodist_policy_DATA = org.x.xf86-video-intel.backlight-helper.policy

backlight_helper = $(libexecdir)/xf86-video-intel-backlight-helper
install-exec-hook:
	-chown root $(DESTDIR)$(backlight_helper) && chmod u+s $(DESTDIR)$(backlight_helper)
endif

intel_virtual_output_CFLAGS = \
	@CWARNFLAGS@ \
	$(IVO_CFLAGS) \
	@NOWARNFLAGS@ \
	$(NULL)
intel_virtual_output_SOURCES = \
	virtual.c \
	$(NULL)
intel_virtual_output_LDADD = \
	$(IVO_LIBS) \
	$(NULL)

xf86_video_intel_backlight_helper_SOURCES = \
	backlight_helper.c \
	$(NULL)

EXTRA_DIST = intel-virtual-output.man org.x.xf86-video-intel.backlight-helper.policy.in
CLEANFILES = $(driverman_DATA) $(nodist_policy_DATA)

# String replacements in MAN_SUBSTS now come from xorg-macros.m4 via configure
SUFFIXES = .$(DRIVER_MAN_SUFFIX) .man
.man.$(DRIVER_MAN_SUFFIX):
	$(AM_V_GEN)$(SED) $(MAN_SUBSTS) < $< > $@
