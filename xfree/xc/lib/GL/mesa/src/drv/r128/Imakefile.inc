XCOMM $XFree86: xc/lib/GL/mesa/src/drv/r128/Imakefile.inc,v 1.3 2001/05/02 15:06:04 dawes Exp $

#ifndef MesaDrvSrcDir
#define MesaDrvSrcDir $(GLXLIBSRC)/mesa/src/drv
#endif

MESADRVSRCDIR = MesaDrvSrcDir

#ifndef MesaDrvR128BuildDir
#define MesaDrvR128BuildDir /**/
#endif
MESADRVR128BUILDDIR = MesaDrvR128BuildDir

#if Malloc0ReturnsNull
ALLOC_DEFINES = -DMALLOC_0_RETURNS_NULL
#endif

#if BuildXF86DRI
  DRI_DEFINES = GlxDefines -DX_BYTE_ORDER=ByteOrder
 DRI_INCLUDES = -I$(GLXLIBSRC)/dri -I$(GLXLIBSRC)/glx \
		-I$(INCLUDESRC) -I$(INCLUDESRC)/GL \
		-I$(GLXLIBSRC)/mesa/dri \
		-I$(SERVERSRC)/GL/dri \
		-I$(XF86OSSRC) \
		-I$(XF86DRIVERSRC)/ati \
		-I$(XF86COMSRC) \
		-I$(GLXLIBSRC)/dri/drm
#endif

MESA_INCLUDES = -I$(MESASRCDIR)/src -I$(MESADRVSRCDIR)/common \
		-I$(MESADRVSRCDIR)/r128
   X_INCLUDES = -I$(XINCLUDESRC) -I$(EXTINCSRC)

     R128SRCS = $(MESADRVR128BUILDDIR)r128_context.c \
		$(MESADRVR128BUILDDIR)r128_dd.c \
		$(MESADRVR128BUILDDIR)r128_fastpath.c \
		$(MESADRVR128BUILDDIR)r128_eltpath.c \
		$(MESADRVR128BUILDDIR)r128_ioctl.c \
		$(MESADRVR128BUILDDIR)r128_lock.c \
		$(MESADRVR128BUILDDIR)r128_pipeline.c \
                $(MESADRVR128BUILDDIR)r128_screen.c \
		$(MESADRVR128BUILDDIR)r128_span.c \
		$(MESADRVR128BUILDDIR)r128_state.c \
		$(MESADRVR128BUILDDIR)r128_tex.c \
		$(MESADRVR128BUILDDIR)r128_tris.c \
		$(MESADRVR128BUILDDIR)r128_vb.c \
		$(MESADRVR128BUILDDIR)r128_xmesa.c

     R128OBJS = $(MESADRVR128BUILDDIR)r128_context.o \
		$(MESADRVR128BUILDDIR)r128_dd.o \
		$(MESADRVR128BUILDDIR)r128_fastpath.o \
		$(MESADRVR128BUILDDIR)r128_eltpath.o \
		$(MESADRVR128BUILDDIR)r128_ioctl.o \
		$(MESADRVR128BUILDDIR)r128_lock.o \
		$(MESADRVR128BUILDDIR)r128_pipeline.o \
                $(MESADRVR128BUILDDIR)r128_screen.o \
		$(MESADRVR128BUILDDIR)r128_span.o \
		$(MESADRVR128BUILDDIR)r128_state.o \
		$(MESADRVR128BUILDDIR)r128_tex.o \
		$(MESADRVR128BUILDDIR)r128_tris.o \
		$(MESADRVR128BUILDDIR)r128_vb.o \
		$(MESADRVR128BUILDDIR)r128_xmesa.o

     R128UOBJS = $(MESADRVR128BUILDDIR)unshared/r128_context.o \
		$(MESADRVR128BUILDDIR)unshared/r128_dd.o \
		$(MESADRVR128BUILDDIR)unshared/r128_fastpath.o \
		$(MESADRVR128BUILDDIR)unshared/r128_eltpath.o \
		$(MESADRVR128BUILDDIR)unshared/r128_ioctl.o \
		$(MESADRVR128BUILDDIR)unshared/r128_lock.o \
		$(MESADRVR128BUILDDIR)unshared/r128_pipeline.o \
                $(MESADRVR128BUILDDIR)unshared/r128_screen.o \
		$(MESADRVR128BUILDDIR)unshared/r128_span.o \
		$(MESADRVR128BUILDDIR)unshared/r128_state.o \
		$(MESADRVR128BUILDDIR)unshared/r128_tex.o \
		$(MESADRVR128BUILDDIR)unshared/r128_tris.o \
		$(MESADRVR128BUILDDIR)unshared/r128_vb.o \
		$(MESADRVR128BUILDDIR)unshared/r128_xmesa.o

     R128DOBJS = $(MESADRVR128BUILDDIR)debugger/r128_context.o \
		$(MESADRVR128BUILDDIR)debugger/r128_dd.o \
		$(MESADRVR128BUILDDIR)debugger/r128_fastpath.o \
		$(MESADRVR128BUILDDIR)debugger/r128_eltpath.o \
		$(MESADRVR128BUILDDIR)debugger/r128_ioctl.o \
		$(MESADRVR128BUILDDIR)debugger/r128_lock.o \
		$(MESADRVR128BUILDDIR)debugger/r128_pipeline.o \
                $(MESADRVR128BUILDDIR)debugger/r128_screen.o \
		$(MESADRVR128BUILDDIR)debugger/r128_span.o \
		$(MESADRVR128BUILDDIR)debugger/r128_state.o \
		$(MESADRVR128BUILDDIR)debugger/r128_tex.o \
		$(MESADRVR128BUILDDIR)debugger/r128_tris.o \
		$(MESADRVR128BUILDDIR)debugger/r128_vb.o \
		$(MESADRVR128BUILDDIR)debugger/r128_xmesa.o

     R128POBJS = $(MESADRVR128BUILDDIR)profiled/r128_context.o \
		$(MESADRVR128BUILDDIR)profiled/r128_dd.o \
		$(MESADRVR128BUILDDIR)profiled/r128_fastpath.o \
		$(MESADRVR128BUILDDIR)profiled/r128_eltpath.o \
		$(MESADRVR128BUILDDIR)profiled/r128_ioctl.o \
		$(MESADRVR128BUILDDIR)profiled/r128_lock.o \
		$(MESADRVR128BUILDDIR)profiled/r128_pipeline.o \
                $(MESADRVR128BUILDDIR)profiled/r128_screen.o \
		$(MESADRVR128BUILDDIR)profiled/r128_span.o \
		$(MESADRVR128BUILDDIR)profiled/r128_state.o \
		$(MESADRVR128BUILDDIR)profiled/r128_tex.o \
		$(MESADRVR128BUILDDIR)profiled/r128_tris.o \
		$(MESADRVR128BUILDDIR)profiled/r128_vb.o \
		$(MESADRVR128BUILDDIR)profiled/r128_xmesa.o

#ifdef NeedToLinkMesaSrc
LinkSourceFile(r128_context.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_dd.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_fastpath.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_eltpath.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_ioctl.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_lock.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_pipeline.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_screen.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_span.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_state.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_tex.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_tris.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_vb.c, $(MESADRVSRCDIR)/r128)
LinkSourceFile(r128_xmesa.c, $(MESADRVSRCDIR)/r128)
#endif

