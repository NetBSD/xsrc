XCOMM $XFree86: xc/lib/GL/mesa/src/drv/i810/Imakefile.inc,v 1.2 2001/05/02 15:06:03 dawes Exp $

#ifndef MesaDrvSrcDir
#define MesaDrvSrcDir $(GLXLIBSRC)/mesa/src/drv
#endif

MESADRVSRCDIR = MesaDrvSrcDir

#ifndef MesaDrvI810BuildDir
#define MesaDrvI810BuildDir /**/
#endif
MESADRVI810BUILDDIR = MesaDrvI810BuildDir

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
		-I$(MESADRVSRCDIR)/i810
   X_INCLUDES = -I$(XINCLUDESRC) -I$(EXTINCSRC)

     I810SRCS = $(MESADRVI810BUILDDIR)i810_xmesa.c \
		$(MESADRVI810BUILDDIR)i810dd.c \
		$(MESADRVI810BUILDDIR)i810pipeline.c \
		$(MESADRVI810BUILDDIR)i810span.c \
		$(MESADRVI810BUILDDIR)i810state.c \
		$(MESADRVI810BUILDDIR)i810tex.c \
		$(MESADRVI810BUILDDIR)i810tris.c \
		$(MESADRVI810BUILDDIR)i810vb.c \
		$(MESADRVI810BUILDDIR)i810fastpath.c \
		$(MESADRVI810BUILDDIR)i810ioctl.c

     I810OBJS = $(MESADRVI810BUILDDIR)i810_xmesa.o \
		$(MESADRVI810BUILDDIR)i810dd.o \
		$(MESADRVI810BUILDDIR)i810pipeline.o \
		$(MESADRVI810BUILDDIR)i810span.o \
		$(MESADRVI810BUILDDIR)i810state.o \
		$(MESADRVI810BUILDDIR)i810tex.o \
		$(MESADRVI810BUILDDIR)i810tris.o \
		$(MESADRVI810BUILDDIR)i810vb.o \
		$(MESADRVI810BUILDDIR)i810fastpath.o \
		$(MESADRVI810BUILDDIR)i810ioctl.o

     I810UOBJS = $(MESADRVI810BUILDDIR)unshared/i810_xmesa.o \
		$(MESADRVI810BUILDDIR)unshared/i810dd.o \
		$(MESADRVI810BUILDDIR)unshared/i810pipeline.o \
		$(MESADRVI810BUILDDIR)unshared/i810span.o \
		$(MESADRVI810BUILDDIR)unshared/i810state.o \
		$(MESADRVI810BUILDDIR)unshared/i810tex.o \
		$(MESADRVI810BUILDDIR)unshared/i810tris.o \
		$(MESADRVI810BUILDDIR)unshared/i810vb.o \
		$(MESADRVI810BUILDDIR)unshared/i810fastpath.o \
		$(MESADRVI810BUILDDIR)unshared/i810ioctl.o

     I810DOBJS = $(MESADRVI810BUILDDIR)debugger/i810_xmesa.o \
		$(MESADRVI810BUILDDIR)debugger/i810dd.o \
		$(MESADRVI810BUILDDIR)debugger/i810pipeline.o \
		$(MESADRVI810BUILDDIR)debugger/i810span.o \
		$(MESADRVI810BUILDDIR)debugger/i810state.o \
		$(MESADRVI810BUILDDIR)debugger/i810tex.o \
		$(MESADRVI810BUILDDIR)debugger/i810tris.o \
		$(MESADRVI810BUILDDIR)debugger/i810vb.o \
		$(MESADRVI810BUILDDIR)debugger/i810fastpath.o \
		$(MESADRVI810BUILDDIR)debugger/i810ioctl.o

     I810POBJS = $(MESADRVI810BUILDDIR)profiled/i810_xmesa.o \
		$(MESADRVI810BUILDDIR)profiled/i810dd.o \
		$(MESADRVI810BUILDDIR)profiled/i810pipeline.o \
		$(MESADRVI810BUILDDIR)profiled/i810span.o \
		$(MESADRVI810BUILDDIR)profiled/i810state.o \
		$(MESADRVI810BUILDDIR)profiled/i810tex.o \
		$(MESADRVI810BUILDDIR)profiled/i810tris.o \
		$(MESADRVI810BUILDDIR)profiled/i810vb.o \
		$(MESADRVI810BUILDDIR)profiled/i810fastpath.o \
		$(MESADRVI810BUILDDIR)profiled/i810ioctl.o

#ifdef NeedToLinkMesaSrc
LinkSourceFile(i810_xmesa.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810dd.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810pipeline.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810span.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810state.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810tex.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810tris.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810vb.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810fastpath.c, $(MESADRVSRCDIR)/i810)
LinkSourceFile(i810ioctl.c, $(MESADRVSRCDIR)/i810)
#endif

