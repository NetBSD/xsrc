XCOMM $XFree86: xc/lib/GL/mesa/sparc/Imakefile.inc,v 1.3 2005/02/06 23:48:15 dawes Exp $

#ifndef MesaSPARCBuildDir
#define MesaSPARCBuildDir $(GLXLIBSRC)/mesa/sparc/
#endif

MESASPARCBUILDDIR = MesaSPARCBuildDir

MESA_SPARC_SRCS = $(MESASPARCBUILDDIR)sparc.c \
		  $(MESASPARCBUILDDIR)xform.S \
		  $(MESASPARCBUILDDIR)clip.S \
		  $(MESASPARCBUILDDIR)norm.S

#ifdef NeedToLinkMesaSrc
/* If $(MESASRCDIR)/src/mesa/sparc isn't in the INCLUDES, it should be. */
LinkSourceFile(sparc.c, $(MESASRCDIR)/src/mesa/sparc)
LinkSourceFile(xform.S, $(MESASRCDIR)/src/mesa/sparc)
LinkSourceFile(clip.S, $(MESASRCDIR)/src/mesa/sparc)
LinkSourceFile(norm.S, $(MESASRCDIR)/src/mesa/sparc)
#endif

MESA_SPARC_OBJS = $(MESASPARCBUILDDIR)sparc.o \
		  $(MESASPARCBUILDDIR)xform.o \
		  $(MESASPARCBUILDDIR)clip.o \
		  $(MESASPARCBUILDDIR)norm.o

MESA_SPARC_EOBJS = $(MESASPARCBUILDDIR)$(EXTRASUBDIR)/sparc.o \
		   $(MESASPARCBUILDDIR)$(EXTRASUBDIR)/xform.o \
		   $(MESASPARCBUILDDIR)$(EXTRASUBDIR)/clip.o \
		   $(MESASPARCBUILDDIR)$(EXTRASUBDIR)/norm.o

#if defined(DoSharedLib) && DoSharedLib
MESA_SPARC_UOBJS = $(MESASPARCBUILDDIR)unshared/sparc.o \
		   $(MESASPARCBUILDDIR)xform.o \
		   $(MESASPARCBUILDDIR)clip.o \
		   $(MESASPARCBUILDDIR)norm.o
#else
MESA_SPARC_UOBJS = $(MESA_SPARC_OBJS)
#endif

MESA_SPARC_DOBJS = $(MESASPARCBUILDDIR)debugger/sparc.o \
		   $(MESASPARCBUILDDIR)xform.o \
		   $(MESASPARCBUILDDIR)clip.o \
		   $(MESASPARCBUILDDIR)norm.o

MESA_SPARC_POBJS = $(MESAX86BUILDDIR)profiled/sparc.o \
		   $(MESAX86BUILDDIR)xform.o \
		   $(MESAX86BUILDDIR)clip.o \
		   $(MESAX86BUILDDIR)norm.o

MESA_SPARC_DEFS = -DUSE_SPARC_ASM

MESA_ASM_SRCS = $(MESA_SPARC_SRCS)

MESA_ASM_OBJS = $(MESA_SPARC_OBJS)

MESA_ASM_EOBJS = $(MESA_SPARC_EOBJS)

MESA_ASM_UOBJS = $(MESA_SPARC_UOBJS)

MESA_ASM_DOBJS = $(MESA_SPARC_DOBJS)

MESA_ASM_POBJS = $(MESA_SPARC_POBJS)

MESA_ASM_DEFINES = $(MESA_SPARC_DEFS)

