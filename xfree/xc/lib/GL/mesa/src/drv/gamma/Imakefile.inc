XCOMM $XFree86: xc/lib/GL/mesa/src/drv/gamma/Imakefile.inc,v 1.2 2001/05/02 15:06:03 dawes Exp $

#ifndef MesaDrvSrcDir
#define MesaDrvSrcDir $(GLXLIBSRC)/mesa/src/drv
#endif

MESADRVSRCDIR = MesaDrvSrcDir

#ifndef MesaDrvGammaBuildDir
#define MesaDrvGammaBuildDir /**/
#endif
MESADRVGAMMABUILDDIR = MesaDrvGammaBuildDir

#if Malloc0ReturnsNull
ALLOC_DEFINES = -DMALLOC_0_RETURNS_NULL
#endif

#if BuildXF86DRI
  DRI_DEFINES = GlxDefines
 DRI_INCLUDES = -I$(GLXLIBSRC)/dri -I$(GLXLIBSRC)/glx \
		-I$(INCLUDESRC) -I$(INCLUDESRC)/GL \
		-I$(GLXLIBSRC)/mesa/dri \
		-I$(SERVERSRC)/GL/dri \
		-I$(XF86DRIVERSRC)/glint \
		-I$(XF86OSSRC)
#endif

MESA_INCLUDES = -I$(MESASRCDIR)/src -I$(MESADRVSRCDIR)/gamma
   X_INCLUDES = -I$(XINCLUDESRC) -I$(EXTINCSRC)


    GAMMASRCS = $(MESADRVGAMMABUILDDIR)gamma_gl.c \
		$(MESADRVGAMMABUILDDIR)gamma_xmesa.c \
		$(MESADRVGAMMABUILDDIR)gamma_init.c \
		$(MESADRVGAMMABUILDDIR)gamma_matrix.c \
		$(MESADRVGAMMABUILDDIR)gamma_inithw.c \
		$(MESADRVGAMMABUILDDIR)gamma_texture.c \
		$(MESADRVGAMMABUILDDIR)gamma_dlist.c

    GAMMAOBJS = $(MESADRVGAMMABUILDDIR)gamma_gl.o \
		$(MESADRVGAMMABUILDDIR)gamma_xmesa.o \
		$(MESADRVGAMMABUILDDIR)gamma_init.o \
		$(MESADRVGAMMABUILDDIR)gamma_matrix.o \
		$(MESADRVGAMMABUILDDIR)gamma_inithw.o \
		$(MESADRVGAMMABUILDDIR)gamma_texture.o \
		$(MESADRVGAMMABUILDDIR)gamma_dlist.o

    GAMMAUOBJS = $(MESADRVGAMMABUILDDIR)unshared/gamma_gl.o \
		$(MESADRVGAMMABUILDDIR)unshared/gamma_xmesa.o \
		$(MESADRVGAMMABUILDDIR)unshared/gamma_init.o \
		$(MESADRVGAMMABUILDDIR)unshared/gamma_matrix.o \
		$(MESADRVGAMMABUILDDIR)unshared/gamma_inithw.o \
		$(MESADRVGAMMABUILDDIR)unshared/gamma_texture.o \
		$(MESADRVGAMMABUILDDIR)unshared/gamma_dlist.o

    GAMMADOBJS = $(MESADRVGAMMABUILDDIR)debugger/gamma_gl.o \
		$(MESADRVGAMMABUILDDIR)debugger/gamma_xmesa.o \
		$(MESADRVGAMMABUILDDIR)debugger/gamma_init.o \
		$(MESADRVGAMMABUILDDIR)debugger/gamma_matrix.o \
		$(MESADRVGAMMABUILDDIR)debugger/gamma_inithw.o \
		$(MESADRVGAMMABUILDDIR)debugger/gamma_texture.o \
		$(MESADRVGAMMABUILDDIR)debugger/gamma_dlist.o

    GAMMAPOBJS = $(MESADRVGAMMABUILDDIR)profiled/gamma_gl.o \
		$(MESADRVGAMMABUILDDIR)profiled/gamma_xmesa.o \
		$(MESADRVGAMMABUILDDIR)profiled/gamma_init.o \
		$(MESADRVGAMMABUILDDIR)profiled/gamma_matrix.o \
		$(MESADRVGAMMABUILDDIR)profiled/gamma_inithw.o \
		$(MESADRVGAMMABUILDDIR)profiled/gamma_texture.o \
		$(MESADRVGAMMABUILDDIR)profiled/gamma_dlist.o

#ifdef NeedToLinkMesaSrc
LinkSourceFile(gamma_gl.c, $(MESADRVSRCDIR)/gamma)
LinkSourceFile(gamma_xmesa.c, $(MESADRVSRCDIR)/gamma)
LinkSourceFile(gamma_init.c, $(MESADRVSRCDIR)/gamma)
LinkSourceFile(gamma_matrix.c, $(MESADRVSRCDIR)/gamma)
LinkSourceFile(gamma_inithw.c, $(MESADRVSRCDIR)/gamma)
LinkSourceFile(gamma_texture.c, $(MESADRVSRCDIR)/gamma)
LinkSourceFile(gamma_dlist.c, $(MESADRVSRCDIR)/gamma)
#endif
