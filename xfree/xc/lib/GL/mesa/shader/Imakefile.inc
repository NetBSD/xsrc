XCOMM $XFree86: xc/lib/GL/mesa/shader/Imakefile.inc,v 1.3 2004/12/11 01:39:50 alanh Exp $

#ifndef MesaShaderBuildDir
#define MesaShaderBuildDir $(GLXLIBSRC)/mesa/shader/
#endif

MESASHADERBUILDDIR = MesaShaderBuildDir

  MESA_SHADER_SRCS = $(MESASHADERBUILDDIR)arbfragparse.c \
		     $(MESASHADERBUILDDIR)arbprogparse.c \
		     $(MESASHADERBUILDDIR)arbprogram.c \
		     $(MESASHADERBUILDDIR)arbvertparse.c \
		     $(MESASHADERBUILDDIR)grammar_mesa.c \
		     $(MESASHADERBUILDDIR)nvfragparse.c \
		     $(MESASHADERBUILDDIR)nvprogram.c \
		     $(MESASHADERBUILDDIR)nvvertexec.c \
		     $(MESASHADERBUILDDIR)nvvertparse.c \
		     $(MESASHADERBUILDDIR)program.c

#ifdef NeedToLinkMesaSrc
LinkSourceFile(arbfragparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbfragparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbprogram_syn.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbvertparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(arbvertparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar_mesa.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar_mesa.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(grammar_syn.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvfragprog.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvprogram.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvprogram.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertexec.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertexec.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertparse.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertparse.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(nvvertprog.h, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(program.c, $(MESASRCDIR)/src/mesa/shader)
LinkSourceFile(program.h, $(MESASRCDIR)/src/mesa/shader)
#endif

  MESA_SHADER_OBJS = $(MESASHADERBUILDDIR)arbfragparse.o \
		     $(MESASHADERBUILDDIR)arbprogparse.o \
		     $(MESASHADERBUILDDIR)arbprogram.o \
		     $(MESASHADERBUILDDIR)arbvertparse.o \
		     $(MESASHADERBUILDDIR)grammar_mesa.o \
		     $(MESASHADERBUILDDIR)nvfragparse.o \
		     $(MESASHADERBUILDDIR)nvprogram.o \
		     $(MESASHADERBUILDDIR)nvvertexec.o \
		     $(MESASHADERBUILDDIR)nvvertparse.o \
		     $(MESASHADERBUILDDIR)program.o

 MESA_SHADER_EOBJS = $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/arbfragparse.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/arbprogparse.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/arbprogram.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/arbvertparse.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/grammar_mesa.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/nvfragparse.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/nvprogram.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/nvvertexec.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/nvvertparse.o \
		     $(MESASHADERBUILDDIR)$(EXTRASUBDIR)/program.o

#if defined(DoSharedLib) && DoSharedLib
 MESA_SHADER_UOBJS = $(MESASHADERBUILDDIR)unshared/arbfragparse.o \
		     $(MESASHADERBUILDDIR)unshared/arbprogparse.o \
		     $(MESASHADERBUILDDIR)unshared/arbprogram.o \
		     $(MESASHADERBUILDDIR)unshared/arbvertparse.o \
		     $(MESASHADERBUILDDIR)unshared/grammar_mesa.o \
		     $(MESASHADERBUILDDIR)unshared/nvfragparse.o \
		     $(MESASHADERBUILDDIR)unshared/nvprogram.o \
		     $(MESASHADERBUILDDIR)unshared/nvvertexec.o \
		     $(MESASHADERBUILDDIR)unshared/nvvertparse.o \
		     $(MESASHADERBUILDDIR)unshared/program.o
#else
 MESA_SHADER_UOBJS = $(MESA_SHADER_OBJS)
#endif

 MESA_SHADER_DOBJS = $(MESASHADERBUILDDIR)debugger/arbfragparse.o \
		     $(MESASHADERBUILDDIR)debugger/arbprogparse.o \
		     $(MESASHADERBUILDDIR)debugger/arbprogram.o \
		     $(MESASHADERBUILDDIR)debugger/arbvertparse.o \
		     $(MESASHADERBUILDDIR)debugger/grammar_mesa.o \
		     $(MESASHADERBUILDDIR)debugger/nvfragparse.o \
		     $(MESASHADERBUILDDIR)debugger/nvprogram.o \
		     $(MESASHADERBUILDDIR)debugger/nvvertexec.o \
		     $(MESASHADERBUILDDIR)debugger/nvvertparse.o \
		     $(MESASHADERBUILDDIR)debugger/program.o

 MESA_SHADER_POBJS = $(MESASHADERBUILDDIR)profiled/arbfragparse.o \
		     $(MESASHADERBUILDDIR)profiled/arbprogparse.o \
		     $(MESASHADERBUILDDIR)profiled/arbprogram.o \
		     $(MESASHADERBUILDDIR)profiled/arbvertparse.o \
		     $(MESASHADERBUILDDIR)profiled/grammar_mesa.o \
		     $(MESASHADERBUILDDIR)profiled/nvfragparse.o \
		     $(MESASHADERBUILDDIR)profiled/nvprogram.o \
		     $(MESASHADERBUILDDIR)profiled/nvvertexec.o \
		     $(MESASHADERBUILDDIR)profiled/nvvertparse.o \
		     $(MESASHADERBUILDDIR)profiled/program.o
