XCOMM $XFree86: xc/lib/GL/mesa/src/X86/Imakefile.inc,v 1.2 2001/05/02 15:06:03 dawes Exp $

#ifndef MesaX86BuildDir
#define MesaX86BuildDir $(GLXLIBSRC)/mesa/src/X86/
#endif

MESAX86BUILDDIR = MesaX86BuildDir

MESA_X86_SRCS = $(MESAX86BUILDDIR)common_x86.c \
		$(MESAX86BUILDDIR)common_x86_asm.S \
		$(MESAX86BUILDDIR)glapi_x86.S \
		$(MESAX86BUILDDIR)x86.c \
		$(MESAX86BUILDDIR)x86_cliptest.S \
		$(MESAX86BUILDDIR)x86_vertex.S \
		$(MESAX86BUILDDIR)x86_xform_masked2.S \
		$(MESAX86BUILDDIR)x86_xform_masked3.S \
		$(MESAX86BUILDDIR)x86_xform_masked4.S \
		$(MESAX86BUILDDIR)x86_xform_raw2.S \
		$(MESAX86BUILDDIR)x86_xform_raw3.S \
		$(MESAX86BUILDDIR)x86_xform_raw4.S

#ifdef NeedToLinkMesaSrc
LinkSourceFile(common_x86.c, $(MESASRCDIR)/src/X86)
LinkSourceFile(common_x86_asm.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(glapi_x86.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86.c, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86_cliptest.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86_vertex.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86_xform_masked2.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86_xform_masked3.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86_xform_masked4.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86_xform_raw2.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86_xform_raw3.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(x86_xform_raw4.S, $(MESASRCDIR)/src/X86)
#endif

MESA_X86_OBJS = $(MESAX86BUILDDIR)common_x86.o \
		$(MESAX86BUILDDIR)common_x86_asm.o \
		$(MESAX86BUILDDIR)x86.o \
		$(MESAX86BUILDDIR)x86_cliptest.o \
		$(MESAX86BUILDDIR)x86_vertex.o \
		$(MESAX86BUILDDIR)x86_xform_masked2.o \
		$(MESAX86BUILDDIR)x86_xform_masked3.o \
		$(MESAX86BUILDDIR)x86_xform_masked4.o \
		$(MESAX86BUILDDIR)x86_xform_raw2.o \
		$(MESAX86BUILDDIR)x86_xform_raw3.o \
		$(MESAX86BUILDDIR)x86_xform_raw4.o

#if DoSharedLib
MESA_X86_UOBJS = $(MESAX86BUILDDIR)unshared/common_x86.o \
		$(MESAX86BUILDDIR)common_x86_asm.o \
		$(MESAX86BUILDDIR)unshared/x86.o \
		$(MESAX86BUILDDIR)x86_cliptest.o \
		$(MESAX86BUILDDIR)x86_vertex.o \
		$(MESAX86BUILDDIR)x86_xform_masked2.o \
		$(MESAX86BUILDDIR)x86_xform_masked3.o \
		$(MESAX86BUILDDIR)x86_xform_masked4.o \
		$(MESAX86BUILDDIR)x86_xform_raw2.o \
		$(MESAX86BUILDDIR)x86_xform_raw3.o \
		$(MESAX86BUILDDIR)x86_xform_raw4.o
#else
MESA_X86_UOBJS = $(MESA_X86_OBJS)
#endif

MESA_X86_DOBJS = $(MESAX86BUILDDIR)debugger/common_x86.o \
		$(MESAX86BUILDDIR)common_x86_asm.o \
		$(MESAX86BUILDDIR)debugger/x86.o \
		$(MESAX86BUILDDIR)x86_cliptest.o \
		$(MESAX86BUILDDIR)x86_vertex.o \
		$(MESAX86BUILDDIR)x86_xform_masked2.o \
		$(MESAX86BUILDDIR)x86_xform_masked3.o \
		$(MESAX86BUILDDIR)x86_xform_masked4.o \
		$(MESAX86BUILDDIR)x86_xform_raw2.o \
		$(MESAX86BUILDDIR)x86_xform_raw3.o \
		$(MESAX86BUILDDIR)x86_xform_raw4.o

MESA_X86_POBJS = $(MESAX86BUILDDIR)profiled/common_x86.o \
		$(MESAX86BUILDDIR)common_x86_asm.o \
		$(MESAX86BUILDDIR)profiled/x86.o \
		$(MESAX86BUILDDIR)x86_cliptest.o \
		$(MESAX86BUILDDIR)x86_vertex.o \
		$(MESAX86BUILDDIR)x86_xform_masked2.o \
		$(MESAX86BUILDDIR)x86_xform_masked3.o \
		$(MESAX86BUILDDIR)x86_xform_masked4.o \
		$(MESAX86BUILDDIR)x86_xform_raw2.o \
		$(MESAX86BUILDDIR)x86_xform_raw3.o \
		$(MESAX86BUILDDIR)x86_xform_raw4.o

MESA_X86_DEFS = -DUSE_X86_ASM


#if MesaUseMMX
MESA_MMX_SRCS = $(MESAX86BUILDDIR)mmx_blend.S

#ifdef NeedToLinkMesaSrc
LinkSourceFile(mmx_blend.S, $(MESASRCDIR)/src/X86)
#endif

MESA_MMX_OBJS = $(MESAX86BUILDDIR)mmx_blend.o

MESA_MMX_UOBJS = $(MESA_MMX_OBJS)
MESA_MMX_DOBJS = $(MESA_MMX_OBJS)
MESA_MMX_POBJS = $(MESA_MMX_OBJS)

MESA_MMX_DEFS = -DUSE_MMX_ASM
#endif

#if MesaUse3DNow
MESA_3DNOW_SRCS = $(MESAX86BUILDDIR)3dnow.c \
		$(MESAX86BUILDDIR)3dnow_norm_raw.S \
		$(MESAX86BUILDDIR)3dnow_vertex.S \
		$(MESAX86BUILDDIR)3dnow_xform_masked1.S \
		$(MESAX86BUILDDIR)3dnow_xform_masked2.S \
		$(MESAX86BUILDDIR)3dnow_xform_masked3.S \
		$(MESAX86BUILDDIR)3dnow_xform_masked4.S \
		$(MESAX86BUILDDIR)3dnow_xform_raw1.S \
		$(MESAX86BUILDDIR)3dnow_xform_raw2.S \
		$(MESAX86BUILDDIR)3dnow_xform_raw3.S \
		$(MESAX86BUILDDIR)3dnow_xform_raw4.S

#ifdef NeedToLinkMesaSrc
LinkSourceFile(3dnow.c, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_norm_raw.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_vertex.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_xform_masked1.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_xform_masked2.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_xform_masked3.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_xform_masked4.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_xform_raw1.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_xform_raw2.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_xform_raw3.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(3dnow_xform_raw4.S, $(MESASRCDIR)/src/X86)
#endif
MESA_3DNOW_OBJS = $(MESAX86BUILDDIR)3dnow.o \
		$(MESAX86BUILDDIR)3dnow_norm_raw.o \
		$(MESAX86BUILDDIR)3dnow_vertex.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked1.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked2.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked3.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked4.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw1.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw2.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw3.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw4.o

#ifdef DoSharedLib
MESA_3DNOW_UOBJS = $(MESAX86BUILDDIR)unshared/3dnow.o \
		$(MESAX86BUILDDIR)3dnow_norm_raw.o \
		$(MESAX86BUILDDIR)3dnow_vertex.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked1.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked2.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked3.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked4.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw1.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw2.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw3.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw4.o
#else
MESA_3DNOW_UOBJS = $(MESA_3DNOW_OBJS)
#endif

MESA_3DNOW_DOBJS = $(MESAX86BUILDDIR)debugger/3dnow.o \
		$(MESAX86BUILDDIR)3dnow_norm_raw.o \
		$(MESAX86BUILDDIR)3dnow_vertex.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked1.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked2.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked3.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked4.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw1.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw2.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw3.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw4.o

MESA_3DNOW_POBJS = $(MESAX86BUILDDIR)profiled/3dnow.o \
		$(MESAX86BUILDDIR)3dnow_norm_raw.o \
		$(MESAX86BUILDDIR)3dnow_vertex.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked1.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked2.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked3.o \
		$(MESAX86BUILDDIR)3dnow_xform_masked4.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw1.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw2.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw3.o \
		$(MESAX86BUILDDIR)3dnow_xform_raw4.o

MESA_3DNOW_DEFS = -DUSE_3DNOW_ASM
#endif

#if MesaUseKatmai
MESA_KATMAI_SRCS = $(MESAX86BUILDDIR)katmai.c \
		$(MESAX86BUILDDIR)katmai_norm_raw.S \
		$(MESAX86BUILDDIR)katmai_vertex.S \
		$(MESAX86BUILDDIR)katmai_xform_masked1.S \
		$(MESAX86BUILDDIR)katmai_xform_masked2.S \
		$(MESAX86BUILDDIR)katmai_xform_masked3.S \
		$(MESAX86BUILDDIR)katmai_xform_masked4.S \
		$(MESAX86BUILDDIR)katmai_xform_raw1.S \
		$(MESAX86BUILDDIR)katmai_xform_raw2.S \
		$(MESAX86BUILDDIR)katmai_xform_raw3.S \
		$(MESAX86BUILDDIR)katmai_xform_raw4.S

#ifdef NeedToLinkMesaSrc
LinkSourceFile(katmai.c, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_norm_raw.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_vertex.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_xform_masked1.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_xform_masked2.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_xform_masked3.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_xform_masked4.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_xform_raw1.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_xform_raw2.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_xform_raw3.S, $(MESASRCDIR)/src/X86)
LinkSourceFile(katmai_xform_raw4.S, $(MESASRCDIR)/src/X86)
#endif

MESA_KATMAI_OBJS = $(MESAX86BUILDDIR)katmai.o \
		$(MESAX86BUILDDIR)katmai_norm_raw.o \
		$(MESAX86BUILDDIR)katmai_vertex.o \
		$(MESAX86BUILDDIR)katmai_xform_masked1.o \
		$(MESAX86BUILDDIR)katmai_xform_masked2.o \
		$(MESAX86BUILDDIR)katmai_xform_masked3.o \
		$(MESAX86BUILDDIR)katmai_xform_masked4.o \
		$(MESAX86BUILDDIR)katmai_xform_raw1.o \
		$(MESAX86BUILDDIR)katmai_xform_raw2.o \
		$(MESAX86BUILDDIR)katmai_xform_raw3.o \
		$(MESAX86BUILDDIR)katmai_xform_raw4.o

#if DoSharedLib
MESA_KATMAI_UOBJS = $(MESAX86BUILDDIR)unshared/katmai.o \
		$(MESAX86BUILDDIR)katmai_norm_raw.o \
		$(MESAX86BUILDDIR)katmai_vertex.o \
		$(MESAX86BUILDDIR)katmai_xform_masked1.o \
		$(MESAX86BUILDDIR)katmai_xform_masked2.o \
		$(MESAX86BUILDDIR)katmai_xform_masked3.o \
		$(MESAX86BUILDDIR)katmai_xform_masked4.o \
		$(MESAX86BUILDDIR)katmai_xform_raw1.o \
		$(MESAX86BUILDDIR)katmai_xform_raw2.o \
		$(MESAX86BUILDDIR)katmai_xform_raw3.o \
		$(MESAX86BUILDDIR)katmai_xform_raw4.o
#else
MESA_KATMAI_UOBJS = $(MESA_KATMAI_OBJS)
#endif

MESA_KATMAI_DOBJS = $(MESAX86BUILDDIR)debugger/katmai.o \
		$(MESAX86BUILDDIR)katmai_norm_raw.o \
		$(MESAX86BUILDDIR)katmai_vertex.o \
		$(MESAX86BUILDDIR)katmai_xform_masked1.o \
		$(MESAX86BUILDDIR)katmai_xform_masked2.o \
		$(MESAX86BUILDDIR)katmai_xform_masked3.o \
		$(MESAX86BUILDDIR)katmai_xform_masked4.o \
		$(MESAX86BUILDDIR)katmai_xform_raw1.o \
		$(MESAX86BUILDDIR)katmai_xform_raw2.o \
		$(MESAX86BUILDDIR)katmai_xform_raw3.o \
		$(MESAX86BUILDDIR)katmai_xform_raw4.o

MESA_KATMAI_POBJS = $(MESAX86BUILDDIR)profiled/katmai.o \
		$(MESAX86BUILDDIR)katmai_norm_raw.o \
		$(MESAX86BUILDDIR)katmai_vertex.o \
		$(MESAX86BUILDDIR)katmai_xform_masked1.o \
		$(MESAX86BUILDDIR)katmai_xform_masked2.o \
		$(MESAX86BUILDDIR)katmai_xform_masked3.o \
		$(MESAX86BUILDDIR)katmai_xform_masked4.o \
		$(MESAX86BUILDDIR)katmai_xform_raw1.o \
		$(MESAX86BUILDDIR)katmai_xform_raw2.o \
		$(MESAX86BUILDDIR)katmai_xform_raw3.o \
		$(MESAX86BUILDDIR)katmai_xform_raw4.o

MESA_KATMAI_DEFS = -DUSE_KATMAI_ASM
#endif

MESA_ASM_SRCS = $(MESA_X86_SRCS) $(MESA_MMX_SRCS) $(MESA_3DNOW_SRCS) \
		$(MESA_KATMAI_SRCS)

MESA_ASM_OBJS = $(MESA_X86_OBJS) $(MESA_MMX_OBJS) $(MESA_3DNOW_OBJS) \
		$(MESA_KATMAI_OBJS)

MESA_ASM_UOBJS = $(MESA_X86_UOBJS) $(MESA_MMX_UOBJS) $(MESA_3DNOW_UOBJS) \
		$(MESA_KATMAI_UOBJS)

MESA_ASM_DOBJS = $(MESA_X86_DOBJS) $(MESA_MMX_DOBJS) $(MESA_3DNOW_DOBJS) \
		$(MESA_KATMAI_DOBJS)

MESA_ASM_POBJS = $(MESA_X86_POBJS) $(MESA_MMX_POBJS) $(MESA_3DNOW_POBJS) \
		$(MESA_KATMAI_POBJS)

MESA_ASM_DEFINES = -DUSE_X86_ASM $(MESA_MMX_DEFS) $(MESA_3DNOW_DEFS) $(MESA_KATMAI_DEFS)

