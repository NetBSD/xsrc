AM_CXXFLAGS = \
    -I$(top_srcdir)/include \
    $(X11_CFLAGS)

module_LTLIBRARIES = libvdpau_trace.la

libvdpau_trace_la_SOURCES = \
    vdpau_trace.cpp

libvdpau_trace_la_LIBADD = \
    $(DLOPEN_LIBS)

libvdpau_trace_la_LDFLAGS = -version-info 1:0:0 -module -no-undefined

libvdpau_traceincludedir = $(includedir)/vdpau
libvdpau_traceinclude_HEADERS = \
    $(top_srcdir)/include/vdpau/vdpau.h \
    $(top_srcdir)/include/vdpau/vdpau_x11.h
