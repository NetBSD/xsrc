## Process this file with automake to produce Makefile.in

#AUTOMAKE_OPTIONS = no-dependencies

x3dnow_files = \
	3dnow.c \
	3dnow.h \
	3dnow_norm_raw.S \
	3dnow_xform_masked1.S \
	3dnow_xform_masked2.S \
	3dnow_xform_masked3.S \
	3dnow_xform_masked4.S \
	3dnow_xform_raw1.S \
	3dnow_xform_raw2.S \
	3dnow_xform_raw3.S \
	3dnow_xform_raw4.S \
	vertex_3dnow.S

katmai_files = \
	katmai.c \
	katmai.h \
	katmai_norm_raw.S \
	katmai_xform_raw1.S \
	katmai_xform_masked1.S \
	katmai_xform_raw2.S \
	katmai_xform_masked2.S \
	katmai_xform_raw3.S \
	katmai_xform_masked3.S \
	katmai_xform_raw4.S \
	katmai_xform_masked4.S \
	vertex_katmai.S

mmx_files = mmx.h mmx_blend.S

if HAVE_3DNOW
x3dnow_sources = $(x3dnow_files)
CFLAGS_3DNOW = -DUSE_3DNOW_ASM
endif

if HAVE_SSE
katmai_sources = $(katmai_files)
CFLAGS_KATMAI = -DUSE_KATMAI_ASM
endif

if HAVE_MMX
mmx_sources = $(mmx_files)
CFLAGS_MMX = -DUSE_MMX_ASM
endif

if HAVE_X86
# Build a libtool convenience library.
noinst_LTLIBRARIES = libMesaX86.la
CFLAGS_X86 = -DUSE_X86_ASM
endif

libMesaX86_la_SOURCES = x86a.S assyntax.h \
	common_x86.c common_x86_asm.S common_x86_asm.h \
	common_x86_features.h glapi_x86.S \
	x86.c x86.h vertex.S $(mmx_sources) $(x3dnow_sources) $(katmai_sources)

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/src $(CFLAGS_X86) $(CFLAGS_MMX) $(CFLAGS_3DNOW) $(CFLAGS_KATMAI)

EXTRA_DIST = x86flatregs.m4 x86a.S.m4 $(x3dnow_files) $(mmx_files) $(katmai_files)

x86a.S: $(srcdir)/x86flatregs.m4
x86a.S: $(srcdir)/x86a.S.m4
	m4 $< >$@

x86a.asm: x86a.S
	gcc -E -P -DNASM_ASSEMBLER $< >$@
