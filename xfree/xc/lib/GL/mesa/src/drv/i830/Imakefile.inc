XCOMM $XFree86: xc/lib/GL/mesa/src/drv/i830/Imakefile.inc,v 1.1 2001/10/04 18:28:21 alanh Exp $

#ifndef MesaDrvSrcDir
#define MesaDrvSrcDir $(GLXLIBSRC)/mesa/src/drv
#endif

MESADRVSRCDIR = MesaDrvSrcDir

#ifndef MesaDrvI830BuildDir
#define MesaDrvI830BuildDir /**/
#endif
MESADRVI830BUILDDIR = MesaDrvI830BuildDir

#if Malloc0ReturnsNull
ALLOC_DEFINES = -DMALLOC_0_RETURNS_NULL
#endif

#if BuildXF86DRI
  DRI_DEFINES = GlxDefines
 DRI_INCLUDES = -I$(GLXLIBSRC)/dri -I$(GLXLIBSRC)/glx \
		-I$(INCLUDESRC) -I$(INCLUDESRC)/GL \
		-I$(GLXLIBSRC)/mesa/dri \
		-I$(SERVERSRC)/GL/dri \
		-I$(XF86OSSRC) \
		-I$(XF86DRIVERSRC)/i810 \
		-I$(GLXLIBSRC)/dri/drm
#endif

MESA_INCLUDES = -I$(MESASRCDIR)/src -I$(MESADRVSRCDIR)/common \
		-I$(MESADRVSRCDIR)/i830
   X_INCLUDES = -I$(XINCLUDESRC) -I$(EXTINCSRC)

      I830SRCS = $(MESADRVI830BUILDDIR)i830_xmesa.c \
		$(MESADRVI830BUILDDIR)i830_dd.c \
		$(MESADRVI830BUILDDIR)i830_pipeline.c \
		$(MESADRVI830BUILDDIR)i830_span.c \
		$(MESADRVI830BUILDDIR)i830_state.c \
		$(MESADRVI830BUILDDIR)i830_tex.c \
		$(MESADRVI830BUILDDIR)i830_tris.c \
		$(MESADRVI830BUILDDIR)i830_vb.c \
		$(MESADRVI830BUILDDIR)i830_fastpath.c \
		$(MESADRVI830BUILDDIR)i830_ioctl.c

      I830OBJS = $(MESADRVI830BUILDDIR)i830_xmesa.o \
		$(MESADRVI830BUILDDIR)i830_dd.o \
		$(MESADRVI830BUILDDIR)i830_pipeline.o \
		$(MESADRVI830BUILDDIR)i830_span.o \
		$(MESADRVI830BUILDDIR)i830_state.o \
		$(MESADRVI830BUILDDIR)i830_tex.o \
		$(MESADRVI830BUILDDIR)i830_tris.o \
		$(MESADRVI830BUILDDIR)i830_vb.o \
		$(MESADRVI830BUILDDIR)i830_fastpath.o \
		$(MESADRVI830BUILDDIR)i830_ioctl.o

     I830UOBJS = $(MESADRVI830BUILDDIR)unshared/i830_xmesa.o \
		$(MESADRVI830BUILDDIR)unshared/i830_dd.o \
		$(MESADRVI830BUILDDIR)unshared/i830_pipeline.o \
		$(MESADRVI830BUILDDIR)unshared/i830_span.o \
		$(MESADRVI830BUILDDIR)unshared/i830_state.o \
		$(MESADRVI830BUILDDIR)unshared/i830_tex.o \
		$(MESADRVI830BUILDDIR)unshared/i830_tris.o \
		$(MESADRVI830BUILDDIR)unshared/i830_vb.o \
		$(MESADRVI830BUILDDIR)unshared/i830_fastpath.o \
		$(MESADRVI830BUILDDIR)unshared/i830_ioctl.o

     I830DOBJS = $(MESADRVI830BUILDDIR)unshared/i830_xmesa.o \
		$(MESADRVI830BUILDDIR)unshared/i830_dd.o \
		$(MESADRVI830BUILDDIR)unshared/i830_pipeline.o \
		$(MESADRVI830BUILDDIR)unshared/i830_span.o \
		$(MESADRVI830BUILDDIR)unshared/i830_state.o \
		$(MESADRVI830BUILDDIR)unshared/i830_tex.o \
		$(MESADRVI830BUILDDIR)unshared/i830_tris.o \
		$(MESADRVI830BUILDDIR)unshared/i830_vb.o \
		$(MESADRVI830BUILDDIR)unshared/i830_fastpath.o \
		$(MESADRVI830BUILDDIR)unshared/i830_ioctl.o

     I830POBJS = $(MESADRVI830BUILDDIR)unshared/i830_xmesa.o \
		$(MESADRVI830BUILDDIR)unshared/i830_dd.o \
		$(MESADRVI830BUILDDIR)unshared/i830_pipeline.o \
		$(MESADRVI830BUILDDIR)unshared/i830_span.o \
		$(MESADRVI830BUILDDIR)unshared/i830_state.o \
		$(MESADRVI830BUILDDIR)unshared/i830_tex.o \
		$(MESADRVI830BUILDDIR)unshared/i830_tris.o \
		$(MESADRVI830BUILDDIR)unshared/i830_vb.o \
		$(MESADRVI830BUILDDIR)unshared/i830_fastpath.o \
		$(MESADRVI830BUILDDIR)unshared/i830_ioctl.o

#ifdef NeedToLinkMesaSrc
LinkSourceFile(i830_xmesa.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_dd.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_pipeline.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_span.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_state.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_tex.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_tris.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_vb.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_fastpath.c, $(MESADRVSRCDIR)/i830)
LinkSourceFile(i830_ioctl.c, $(MESADRVSRCDIR)/i830)
#endif

