XCOMM $XFree86: xc/lib/GL/mesa/src/drv/mga/Imakefile.inc,v 1.2 2001/05/02 15:06:03 dawes Exp $

#ifndef MesaDrvSrcDir
#define MesaDrvSrcDir $(GLXLIBSRC)/mesa/src/drv
#endif

MESADRVSRCDIR = MesaDrvSrcDir

#ifndef MesaDrvMgaBuildDir
#define MesaDrvMgaBuildDir /**/
#endif
MESADRVMGABUILDDIR = MesaDrvMgaBuildDir

#if Malloc0ReturnsNull
ALLOC_DEFINES = -DMALLOC_0_RETURNS_NULL
#endif

#if BuildXF86DRI
  DRI_DEFINES = GlxDefines
 DRI_INCLUDES = -I$(GLXLIBSRC)/dri \
		-I$(GLXLIBSRC)/glx \
		-I$(INCLUDESRC) \
		-I$(INCLUDESRC)/GL \
		-I$(GLXLIBSRC)/mesa/dri \
		-I$(SERVERSRC)/GL/dri \
		-I$(XF86OSSRC) \
		-I$(XF86DRIVERSRC)/mga \
		-I$(GLXLIBSRC)/dri/drm
#endif

MESA_INCLUDES = -I$(MESASRCDIR)/src \
		-I$(MESADRVSRCDIR)/common \
		-I$(MESADRVSRCDIR)/mga
   X_INCLUDES = -I$(XINCLUDESRC) -I$(EXTINCSRC)

      MGASRCS = $(MESADRVMGABUILDDIR)mgabuffers.c \
		$(MESADRVMGABUILDDIR)mgadd.c \
		$(MESADRVMGABUILDDIR)mgaeltpath.c \
		$(MESADRVMGABUILDDIR)mgafastpath.c \
		$(MESADRVMGABUILDDIR)mgaioctl.c \
		$(MESADRVMGABUILDDIR)mgapipeline.c \
		$(MESADRVMGABUILDDIR)mgaspan.c \
		$(MESADRVMGABUILDDIR)mgastate.c \
		$(MESADRVMGABUILDDIR)mgatex.c \
		$(MESADRVMGABUILDDIR)mgatexcnv.c \
		$(MESADRVMGABUILDDIR)mgatexmem.c \
		$(MESADRVMGABUILDDIR)mgatris.c \
		$(MESADRVMGABUILDDIR)mgavb.c \
		$(MESADRVMGABUILDDIR)mga_xmesa.c \

      MGAOBJS = $(MESADRVMGABUILDDIR)mgabuffers.o \
		$(MESADRVMGABUILDDIR)mgadd.o \
		$(MESADRVMGABUILDDIR)mgaeltpath.o \
		$(MESADRVMGABUILDDIR)mgafastpath.o \
		$(MESADRVMGABUILDDIR)mgaioctl.o \
		$(MESADRVMGABUILDDIR)mgapipeline.o \
		$(MESADRVMGABUILDDIR)mgaspan.o \
		$(MESADRVMGABUILDDIR)mgastate.o \
		$(MESADRVMGABUILDDIR)mgatex.o \
		$(MESADRVMGABUILDDIR)mgatexcnv.o \
		$(MESADRVMGABUILDDIR)mgatexmem.o \
		$(MESADRVMGABUILDDIR)mgatris.o \
		$(MESADRVMGABUILDDIR)mgavb.o \
		$(MESADRVMGABUILDDIR)mga_xmesa.o

      MGAUOBJS = $(MESADRVMGABUILDDIR)unshared/mgabuffers.o \
		$(MESADRVMGABUILDDIR)unshared/mgadd.o \
		$(MESADRVMGABUILDDIR)unshared/mgaeltpath.o \
		$(MESADRVMGABUILDDIR)unshared/mgafastpath.o \
		$(MESADRVMGABUILDDIR)unshared/mgaioctl.o \
		$(MESADRVMGABUILDDIR)unshared/mgapipeline.o \
		$(MESADRVMGABUILDDIR)unshared/mgaspan.o \
		$(MESADRVMGABUILDDIR)unshared/mgastate.o \
		$(MESADRVMGABUILDDIR)unshared/mgatex.o \
		$(MESADRVMGABUILDDIR)unshared/mgatexcnv.o \
		$(MESADRVMGABUILDDIR)unshared/mgatexmem.o \
		$(MESADRVMGABUILDDIR)unshared/mgatris.o \
		$(MESADRVMGABUILDDIR)unshared/mgavb.o \
		$(MESADRVMGABUILDDIR)unshared/mga_xmesa.o

      MGADOBJS = $(MESADRVMGABUILDDIR)debugger/mgabuffers.o \
		$(MESADRVMGABUILDDIR)debugger/mgadd.o \
		$(MESADRVMGABUILDDIR)debugger/mgaeltpath.o \
		$(MESADRVMGABUILDDIR)debugger/mgafastpath.o \
		$(MESADRVMGABUILDDIR)debugger/mgaioctl.o \
		$(MESADRVMGABUILDDIR)debugger/mgapipeline.o \
		$(MESADRVMGABUILDDIR)debugger/mgaspan.o \
		$(MESADRVMGABUILDDIR)debugger/mgastate.o \
		$(MESADRVMGABUILDDIR)debugger/mgatex.o \
		$(MESADRVMGABUILDDIR)debugger/mgatexcnv.o \
		$(MESADRVMGABUILDDIR)debugger/mgatexmem.o \
		$(MESADRVMGABUILDDIR)debugger/mgatris.o \
		$(MESADRVMGABUILDDIR)debugger/mgavb.o \
		$(MESADRVMGABUILDDIR)debugger/mga_xmesa.o

      MGAPOBJS = $(MESADRVMGABUILDDIR)profiled/mgabuffers.o \
		$(MESADRVMGABUILDDIR)profiled/mgadd.o \
		$(MESADRVMGABUILDDIR)profiled/mgaeltpath.o \
		$(MESADRVMGABUILDDIR)profiled/mgafastpath.o \
		$(MESADRVMGABUILDDIR)profiled/mgaioctl.o \
		$(MESADRVMGABUILDDIR)profiled/mgapipeline.o \
		$(MESADRVMGABUILDDIR)profiled/mgaspan.o \
		$(MESADRVMGABUILDDIR)profiled/mgastate.o \
		$(MESADRVMGABUILDDIR)profiled/mgatex.o \
		$(MESADRVMGABUILDDIR)profiled/mgatexcnv.o \
		$(MESADRVMGABUILDDIR)profiled/mgatexmem.o \
		$(MESADRVMGABUILDDIR)profiled/mgatris.o \
		$(MESADRVMGABUILDDIR)profiled/mgavb.o \
		$(MESADRVMGABUILDDIR)profiled/mga_xmesa.o

#ifdef NeedToLinkMesaSrc
LinkSourceFile(mgabuffers.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgadd.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgaeltpath.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgafastpath.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgaioctl.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgapipeline.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgaspan.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgastate.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgatex.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgatexcnv.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgatexmem.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgatris.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mgavb.c, $(MESADRVSRCDIR)/mga)
LinkSourceFile(mga_xmesa.c, $(MESADRVSRCDIR)/mga)
#endif

