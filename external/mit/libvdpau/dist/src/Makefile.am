AM_CFLAGS = \
    -I$(top_srcdir)/include \
    -DVDPAU_MODULEDIR="\"$(moduledir)\"" \
    -DVDPAU_SYSCONFDIR="\"$(sysconfdir)\"" \
    $(X11_CFLAGS) \
    $(XEXT_CFLAGS)

lib_LTLIBRARIES = libvdpau.la

libvdpau_la_SOURCES = \
    vdpau_wrapper.c \
    util.h \
    $(DRI2_SOURCES)

if DRI2
    AM_CFLAGS += \
        $(dri2proto_CFLAGS)
    DRI2_SOURCES = \
        mesa_dri2.c \
        mesa_dri2.h
endif

libvdpau_la_LIBADD = \
    $(DLOPEN_LIBS) \
    $(PTHREAD_LIBS) \
    $(XEXT_LIBS)

libvdpau_la_LDFLAGS = -version-info 1:0:0 -no-undefined

libvdpauincludedir = $(includedir)/vdpau
libvdpauinclude_HEADERS = \
    $(top_srcdir)/include/vdpau/vdpau.h \
    $(top_srcdir)/include/vdpau/vdpau_x11.h

libvdpausysconfdir=$(sysconfdir)
dist_libvdpausysconf_DATA = vdpau_wrapper.cfg
