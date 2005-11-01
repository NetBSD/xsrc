XCOMM $XFree86: xc/programs/Xserver/cfb/Imakefile.inc,v 1.8 2005/02/09 19:41:59 dawes Exp $
/*
 * Copyright (c) 1994-2004 by The XFree86 Project, Inc.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 *   1.  Redistributions of source code must retain the above copyright
 *       notice, this list of conditions, and the following disclaimer.
 *
 *   2.  Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer
 *       in the documentation and/or other materials provided with the
 *       distribution, and in the same place and form as other copyright,
 *       license and disclaimer information.
 *
 *   3.  The end-user documentation included with the redistribution,
 *       if any, must include the following acknowledgment: "This product
 *       includes software developed by The XFree86 Project, Inc
 *       (http://www.xfree86.org/) and its contributors", in the same
 *       place and form as other third-party acknowledgments.  Alternately,
 *       this acknowledgment may appear in the software itself, in the
 *       same form and location as other such third-party acknowledgments.
 *
 *   4.  Except as contained in this notice, the name of The XFree86
 *       Project, Inc shall not be used in advertising or otherwise to
 *       promote the sale, use or other dealings in this Software without
 *       prior written authorization from The XFree86 Project, Inc.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if DoLoadableServer
#if !BuildModuleInSubdir
#define IHaveModules
#elif !defined(IHaveModules)
#define IHaveSubdirs
SUBDIRS = module
#endif
#endif

#include <Server.tmpl>

#ifndef PixelSize
#define PixelSize 8
#endif

#if PixelSize == 8
PSZSRCS = cfb8bit.c cfbteblt8.c cfbglrop8.c cfbpush8.c cfbrctstp8.c
PSZOBJS = cfb8bit.o cfbteblt8.o cfbglrop8.o cfbpush8.o cfbrctstp8.o
#if defined(MipsArchitecture) && \
    !defined(ArcArchitecture) && \
    !defined(Mips64Architecture)
STIPPLESRC = stipmips.s stipmipste.s
STIPPLEOBJ = stipmips.o stipmipste.o
STIPPLEDEF = -DHAS_STIPPLE_CODE
#endif
#if defined(SparcArchitecture) && !defined(__arch64__)
STIPPLESRC = stipsparc.s stipsparcte.s
STIPPLEOBJ = stipsparc.o stipsparcte.o
STIPPLEDEF = -DHAS_STIPPLE_CODE
#endif
#endif

#if PixelSize == 32
#if defined(SparcArchitecture) && !defined(__arch64__)
STIPPLESRC = stipsprc32.s stipsprcte32.s
STIPPLEOBJ = stipsprc32.o stipsprcte32.o
STIPPLEDEF = -DHAS_STIPPLE_CODE
#endif
#endif

#if defined(IHaveModules)
XFMODSRC = cfbmodule.c
XFMODOBJ = cfbmodule.o
#endif

#if !(defined(IHaveModules) && PixelSize != 8)
CFBCMAP = cfbcmap.o
#endif

SRCS =	 cfbgc.c cfbrrop.c cfbwindow.c \
	 cfbpntwin.c cfbmskbits.c cfbpixmap.c cfbbitblt.c \
	 cfbfillsp.c cfbsetsp.c cfbscrinit.c cfballpriv.c \
	 cfbgetsp.c cfbfillrct.c \
	 cfbsolidC.c cfbsolidX.c cfbsolidG.c \
	 cfbtileoddC.c cfbtileoddG.c cfbtile32C.c cfbtile32G.c \
	 cfbcmap.c cfbzerarcC.c cfbzerarcX.c cfbzerarcG.c \
	 cfbfillarcC.c cfbfillarcG.c \
	 cfbigblt8.c cfbglblt8.c cfbtegblt.c cfbbstore.c \
	 cfbbltC.c cfbbltX.c cfbbltO.c cfbbltG.c \
	 cfbpolypnt.c \
	 cfbbres.c cfbline.c cfbhrzvert.c cfbbresd.c cfbimage.c cfbseg.c \
	 cfb8lineCO.c cfb8lineX.c cfb8lineG.c cfb8lineCP.c \
	 cfb8segC.c cfb8segX.c cfb8segG.c cfb8segCS.c \
	 cfbply1rctC.c cfbply1rctG.c cfbcppl.c $(PSZSRCS) $(XFMODSRC)

OBJS =   cfbgc.o cfbrrop.o cfbwindow.o \
	 cfbgetsp.o cfbfillrct.o \
	 cfbsolidC.o cfbsolidX.o cfbsolidG.o \
	 cfbtileoddC.o cfbtileoddG.o cfbtile32C.o cfbtile32G.o \
	 cfbfillsp.o cfbsetsp.o cfbscrinit.o cfballpriv.o \
	 cfbpntwin.o cfbmskbits.o cfbpixmap.o \
	 $(CFBCMAP) cfbzerarcC.o cfbzerarcX.o cfbzerarcG.o \
	 cfbfillarcC.o cfbfillarcG.o \
	 cfbigblt8.o cfbglblt8.o cfbtegblt.o cfbbstore.o \
	 cfbpolypnt.o \
	 cfbbres.o cfbline.o cfbhrzvert.o cfbbresd.o cfbimage.o cfbseg.o \
	 cfb8lineCO.o cfb8lineX.o cfb8lineG.o cfb8lineCP.o \
	 cfb8segCS.o cfb8segX.o cfb8segG.o cfb8segC.o \
	 cfbbitblt.o cfbbltC.o cfbbltX.o cfbbltO.o cfbbltG.o \
	 cfbply1rctC.o cfbply1rctG.o cfbcppl.o $(PSZOBJS) $(STIPPLEOBJ) \
	 $(XFMODOBJ)

#ifdef XFree86Version
PLATFORMDEFS = -DXFREE86
#endif

#if BuildLowMem
  LMDEFINES = -ULOWMEMFTPT
#endif

   INCLUDES = -I$(SERVERSRC)/cfb -I$(SERVERSRC)/mfb -I$(SERVERSRC)/mi \
	      -I$(SERVERSRC)/cfb24 -I$(SERVERSRC)/include -I$(XINCLUDESRC) \
	      -I$(FONTINCSRC) -I$(EXTINCSRC) -I$(SERVERSRC)/Xext
   LINTLIBS = $(SERVERSRC)/dix/llib-ldix.ln $(SERVERSRC)/os/llib-los.ln \
	     $(SERVERSRC)/mfb/llib-lmfb.ln $(SERVERSRC)/mi/llib-lmi.ln
    DEFINES = $(LMDEFINES) -DPSZ=PixelSize $(PLATFORMDEFS)

#if PixelSize == 8
CFBNAME = cfb
#elif PixelSize == 16
CFBNAME = cfb16
#elif PixelSize == 24
CFBNAME = cfb24
#elif PixelSize == 32
CFBNAME = cfb32
#endif

#ifdef IHaveModules
ModuleObjectRule()
LibraryModuleTarget($(CFBNAME),$(OBJS),.)
#else
NormalLibraryObjectRule()
NormalLibraryTarget($(CFBNAME),$(OBJS))
#endif

LintLibraryTarget($(CFBNAME),$(SRCS))

NormalLintTarget($(SRCS))

#ifdef LinkDirectory

LinkSourceFile(cfballpriv.c,LinkDirectory)
LinkSourceFile(cfbbitblt.c,LinkDirectory)
LinkSourceFile(cfbblt.c,LinkDirectory)
LinkSourceFile(cfbbres.c,LinkDirectory)
LinkSourceFile(cfbbresd.c,LinkDirectory)
LinkSourceFile(cfbbstore.c,LinkDirectory)
LinkSourceFile(cfbcmap.c,LinkDirectory)
LinkSourceFile(cfbcppl.c,LinkDirectory)
LinkSourceFile(cfbfillarc.c,LinkDirectory)
LinkSourceFile(cfbfillrct.c,LinkDirectory)
LinkSourceFile(cfbfillsp.c,LinkDirectory)
LinkSourceFile(cfbgc.c,LinkDirectory)
LinkSourceFile(cfbgetsp.c,LinkDirectory)
LinkSourceFile(cfbhrzvert.c,LinkDirectory)
LinkSourceFile(cfbimage.c,LinkDirectory)
LinkSourceFile(cfbline.c,LinkDirectory)
LinkSourceFile(cfb8line.c,LinkDirectory)
LinkSourceFile(cfbmskbits.c,LinkDirectory)
LinkSourceFile(cfbpixmap.c,LinkDirectory)
LinkSourceFile(cfbply1rct.c,LinkDirectory)
LinkSourceFile(cfbpntwin.c,LinkDirectory)
LinkSourceFile(cfbpolypnt.c,LinkDirectory)
LinkSourceFile(cfbrrop.c,LinkDirectory)
LinkSourceFile(cfbscrinit.c,LinkDirectory)
LinkSourceFile(cfbsetsp.c,LinkDirectory)
LinkSourceFile(cfbsolid.c,LinkDirectory)
LinkSourceFile(cfbigblt8.c,LinkDirectory)
LinkSourceFile(cfbglblt8.c,LinkDirectory)
LinkSourceFile(cfbtegblt.c,LinkDirectory)
LinkSourceFile(cfbtileodd.c,LinkDirectory)
LinkSourceFile(cfbtile32.c,LinkDirectory)
LinkSourceFile(cfbwindow.c,LinkDirectory)
LinkSourceFile(cfbzerarc.c,LinkDirectory)
#if PixelSize == 8
LinkSourceFile(cfb8bit.c,LinkDirectory)
LinkSourceFile(cfbpush8.c,LinkDirectory)
LinkSourceFile(cfbrctstp8.c,LinkDirectory)
LinkSourceFile(cfbteblt8.c,LinkDirectory)
#if defined(MipsArchitecture) && \
    !defined(ArcArchitecture) && \
    !defined(Mips64Architecture)
LinkSourceFile(stipmips.s,LinkDirectory)
#endif
#ifdef SparcArchitecture
LinkSourceFile(stipsparc.s,LinkDirectory)
#endif
#endif
#if PixelSize == 32
#ifdef SparcArchitecture
LinkSourceFile(stipsprc32.s,LinkDirectory)
#endif
#endif
#endif

#if defined(IHaveModules) && BuildModuleInSubdir
LinkSourceFile(cfbmodule.c,..)
#endif

ObjectFromSpecialSource(cfbseg,cfbline,-DPOLYSEGMENT)

ObjectFromSpecialSource(cfbfillarcC,cfbfillarc,-DRROP=GXcopy)
ObjectFromSpecialSource(cfbfillarcG,cfbfillarc,-DRROP=GXset)

ObjectFromSpecialSource(cfbzerarcC,cfbzerarc,-DRROP=GXcopy)
ObjectFromSpecialSource(cfbzerarcX,cfbzerarc,-DRROP=GXxor)
ObjectFromSpecialSource(cfbzerarcG,cfbzerarc,-DRROP=GXset)

ObjectFromSpecialSource(cfbbltC,cfbblt,-DMROP=Mcopy)
ObjectFromSpecialSource(cfbbltX,cfbblt,-DMROP=Mxor)
ObjectFromSpecialSource(cfbbltO,cfbblt,-DMROP=Mor)
ObjectFromSpecialSource(cfbbltG,cfbblt,-DMROP=0)

ObjectFromSpecialSource(cfbsolidC,cfbsolid,-DRROP=GXcopy)
ObjectFromSpecialSource(cfbsolidX,cfbsolid,-DRROP=GXxor)
ObjectFromSpecialSource(cfbsolidG,cfbsolid,-DRROP=GXset)

ObjectFromSpecialSource(cfbtileoddC,cfbtileodd,-DMROP=Mcopy)
ObjectFromSpecialSource(cfbtileoddG,cfbtileodd,-DMROP=0)

ObjectFromSpecialSource(cfbtile32C,cfbtile32,-DMROP=Mcopy)
ObjectFromSpecialSource(cfbtile32G,cfbtile32,-DMROP=0)

ObjectFromSpecialSource(cfb8lineCO,cfb8line,-DRROP=GXcopy)
ObjectFromSpecialSource(cfb8lineCP,cfb8line,-DRROP=GXcopy -DPREVIOUS)
ObjectFromSpecialSource(cfb8lineX,cfb8line,-DRROP=GXxor)
ObjectFromSpecialSource(cfb8lineG,cfb8line,-DRROP=GXset)

ObjectFromSpecialSource(cfb8segCS,cfb8line,-DRROP=GXcopy -DPOLYSEGMENT -DWIDTH_SHIFT)
ObjectFromSpecialSource(cfb8segC,cfb8line,-DRROP=GXcopy -DPOLYSEGMENT)
ObjectFromSpecialSource(cfb8segX,cfb8line,-DRROP=GXxor -DPOLYSEGMENT)
ObjectFromSpecialSource(cfb8segG,cfb8line,-DRROP=GXset -DPOLYSEGMENT)

ObjectFromSpecialSource(cfbply1rctC,cfbply1rct,-DRROP=GXcopy)
ObjectFromSpecialSource(cfbply1rctG,cfbply1rct,-DRROP=GXset)

SpecialCObjectRule(cfbglblt8,$(_NOOP_),$(STIPPLEDEF))
SpecialCObjectRule(cfbpixmap,$(ICONFIGFILES),$(_NOOP_))
SpecialCObjectRule(cfbcmap,$(ICONFIGFILES),$(EXT_DEFINES) $(GLX_DEFINES))

#if PixelSize == 8
ObjectFromSpecialSource(cfbglrop8,cfbglblt8,-DGLYPHROP)

#if defined(MipsArchitecture) && \
    !defined(ArcArchitecture) && \
    !defined(Mips64Architecture)
stipmipste.s: stipmips.s
	$(RM) $@
	$(LN) stipmips.s stipmipste.s
clean::
	$(RM) stipmipste.s

stipmips.o: stipmips.s
	$(CPP) -I$(XINCLUDESRC) -DX_BYTE_ORDER=$(X_BYTE_ORDER) AsmDefines \
		stipmips.s | $(AS) -o $@ -

stipmipste.o: stipmipste.s
	$(CPP) -DTETEXT -I$(XINCLUDESRC) -DX_BYTE_ORDER=$(X_BYTE_ORDER) \
		AsmDefines stipmipste.s | $(AS) -o $@ -
#endif

#ifdef SparcArchitecture
stipsparcte.s: stipsparc.s
	$(RM) $@
	$(LN) stipsparc.s stipsparcte.s
clean::
	$(RM) stipsparcte.s

stipsparc.o: stipsparc.s
	$(CPP) stipsparc.s AsmDefines | $(AS) -o $@ -

stipsparcte.o: stipsparcte.s
	$(CPP) -DTETEXT stipsparcte.s AsmDefines | $(AS) -o $@ -
#endif
#endif

#if PixelSize == 32
#ifdef SparcArchitecture
stipsprcte32.s: stipsprc32.s
	$(RM) $@
	$(LN) stipsprc32.s stipsprcte32.s
clean::
	$(RM) stipsprcte32.s

stipsprc32.o: stipsprc32.s
	$(CPP) stipsprc32.s AsmDefines | $(AS) -o $@ -

stipsprcte32.o: stipsprcte32.s
	$(CPP) -DTETEXT stipsprcte32.s AsmDefines | $(AS) -o $@ -
#endif
#endif

#ifdef IHaveModules
InstallLibraryModule($(CFBNAME),$(MODULEDIR),.)
#endif

DependTarget()

#ifdef IHaveSubdirs
MakeSubdirs($(SUBDIRS))
DependSubdirs($(SUBDIRS))
#endif

InstallDriverSDKLibraryModule($(CFBNAME),$(DRIVERSDKMODULEDIR),.)

#if PixelSize == 8
InstallDriverSDKNonExecFile(cfb.h,$(DRIVERSDKINCLUDEDIR))
InstallDriverSDKNonExecFile(cfb16.h,$(DRIVERSDKINCLUDEDIR))
InstallDriverSDKNonExecFile(cfb24.h,$(DRIVERSDKINCLUDEDIR))
InstallDriverSDKNonExecFile(cfb32.h,$(DRIVERSDKINCLUDEDIR))
InstallDriverSDKNonExecFile(cfbmap.h,$(DRIVERSDKINCLUDEDIR))
InstallDriverSDKNonExecFile(cfbunmap.h,$(DRIVERSDKINCLUDEDIR))
#endif
