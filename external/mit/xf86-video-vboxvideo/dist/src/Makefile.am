#  Copyright 2005 Adam Jackson.
#  Copyright 2017 Oracle Corporation
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
#  FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
#  THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
#  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
#  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
#  USE OR OTHER DEALINGS IN THE SOFTWARE.


# this is obnoxious:
# -module lets us name the module exactly how we want
# -avoid-version prevents gratuitous .0.0.0 version numbers on the end
# _ladir passes a dummy rpath to libtool so the thing will actually link
# TODO: -nostdlib/-Bstatic/-lgcc platform magic, not installing the .a, etc.

AM_CFLAGS = $(BASE_CFLAGS) $(XORG_CFLAGS) $(PCIACCESS_CFLAGS) \
        -DXORG_7X \
        -DIN_RING3 \
        -DPCIACCESS \
        -DVBOXVIDEO_13 \
        -include xorg-server.h

vboxvideo_drv_la_LTLIBRARIES = vboxvideo_drv.la
vboxvideo_drv_la_LDFLAGS = -module -avoid-version
vboxvideo_drv_ladir = @moduledir@/drivers

vboxvideo_drv_la_SOURCES = \
         edid.c \
         getmode.c \
         HGSMIBase.c \
         HGSMIBase.h \
         HGSMIBuffers.c \
         HGSMIChannels.h \
         HGSMIChSetup.h \
         HGSMICommon.c \
         HGSMIContext.h \
         HGSMIDefs.h \
         HGSMI.h \
         hgsmimemalloc.c \
         HGSMIMemAlloc.h \
         Modesetting.c \
         pointer.c \
         setmode.c \
         vboxvideo.c \
         VBoxVideoErr.h \
         VBoxVideoGuest.h \
         vboxvideo_drv.h \
         VBoxVideo.h \
         VBoxVideoIPRT.h \
         VBoxVideoVBE.h \
         VBVABase.c \
         vbva.c \
         version-generated.h
