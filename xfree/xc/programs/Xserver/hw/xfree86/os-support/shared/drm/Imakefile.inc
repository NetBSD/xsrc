XCOMM $XFree86: xc/programs/Xserver/hw/xfree86/os-support/shared/drm/Imakefile.inc,v 1.7 2005/03/01 03:48:56 dawes Exp $
/*
 * Copyright (c) 1994-2005 by The XFree86 Project, Inc.
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

#if !MakeHasPosixVariableSubstitutions
#error Check if your platform's 'make' has posix variable substitution
#endif

#ifndef DRMCopySubdir
#define DRMCopySubdir kernel_source
#endif

DRM_COPY_DIR = DRMCopySubdir

HostProgramTargetName(revpath):
	@echo "checking $@ over in $(TOP)/config/util first..."; \
	cd $(TOP)/config/util && $(MAKE) HostProgramTargetName(revpath); \
	echo "okay, continuing in $(CURRENT_DIR)"

#ifndef LinkSourceFileListToSubdir
#define LinkSourceFileListToSubdir(step,list,srcdir,subdir)		@@\
step:: HostProgramTargetName(revpath)					@@\
	MakeDir(subdir)							@@\
	@MakeFlagsToShellFlags(i,set +e); \				@@\
	UP="`$(REVPATH) subdir`"; export UP; \				@@\
	for i in list; do ( set -x; $(RM) subdir/$$i; \			@@\
			   $(LN) $${UP}srcdir/$$i subdir/$$i); done
#endif

clean::
	$(RM) -r $(DRM_COPY_DIR)

/* Need to add SRC_*_BSD definitions */

SRCS_DRM_SHARED = \
	drm.h \
	drm_sarea.h

SRCS_DRM_LINUX = \
	drmP.h \
	drm_agpsupport.h \
	drm_auth.h \
	drm_bufs.h \
	drm_context.h \
	drm_dma.h \
	drm_drawable.h \
	drm_drv.h \
	drm_fops.h \
	drm_init.h \
	drm_ioctl.h \
	drm_irq.h \
	drm_lock.h \
	drm_memory.h \
	drm_memory_debug.h \
	drm_os_linux.h \
	drm_proc.h \
	drm_scatter.h \
	drm_stub.h \
	drm_vm.h

SRCS_DRM_BSD = \
	drmP.h \
	drm_agpsupport.h \
	drm_auth.h \
	drm_bufs.h \
	drm_context.h \
	drm_dma.h \
	drm_drawable.h \
	drm_drv.h \
	drm_fops.h \
	drm_ioctl.h \
	drm_irq.h \
	drm_lock.h \
	drm_memory.h \
	drm_memory_debug.h \
	drm_os_freebsd.h \
	drm_os_netbsd.h \
	drm_pci.h \
	drm_scatter.h \
	drm_sysctl.h \
	drm_vm.h

SRCS_GAMMA_LINUX = \
	gamma.h \
	gamma_context.h \
	gamma_drm.h \
	gamma_drv.h \
	gamma_lists.h \
	gamma_lock.h \
	gamma_old_dma.h \
	gamma_dma.c \
	gamma_drv.c

SRCS_I810_LINUX = \
	i810.h \
	i810_drm.h \
	i810_drv.h \
	i810_dma.c \
	i810_drv.c

SRCS_I830_LINUX = \
	i830.h \
	i830_drm.h \
	i830_drv.h \
	i830_dma.c \
	i830_drv.c \
	i830_irq.c

SRCS_R128_SHARED = \
	r128.h \
	r128_drm.h \
	r128_drv.h \
	r128_cce.c \
	r128_irq.c \
	r128_state.c

SRCS_R128_LINUX = \
	r128_drv.c

SRCS_R128_BSD = \
	r128_drv.c

SRCS_RADEON_SHARED = \
	radeon.h \
	radeon_drm.h \
	radeon_drv.h \
	radeon_cp.c \
	radeon_irq.c \
	radeon_mem.c \
	radeon_state.c \

SRCS_RADEON_LINUX = \
	ati_pcigart.h \
	radeon_drv.c

SRCS_RADEON_BSD = \
	ati_pcigart.h \
	radeon_drv.c

SRCS_SIS_SHARED = \
	sis.h \
	sis_drm.h \
	sis_drv.h \
	sis_ds.h \
	sis_ds.c \
	sis_mm.c

SRCS_SIS_LINUX = \
	sis_drv.c

SRCS_SIS_BSD = \
	sis_drv.c

SRCS_MGA_SHARED = \
	mga.h \
	mga_drm.h \
	mga_drv.h \
	mga_ucode.h \
	mga_dma.c \
	mga_irq.c \
	mga_state.c \
	mga_warp.c

SRCS_MGA_LINUX = \
	mga_drv.c

SRCS_MGA_BSD = \
	mga_drv.c

SRCS_TDFX_SHARED = \
	tdfx.h

SRCS_TDFX_LINUX = \
	tdfx_drv.c

SRCS_TDFX_BSD = \
	tdfx_drv.c

SRCS_BUILD_LINUX = \
	Config.in \
	Kconfig \
	Makefile \
	Makefile.kernel

SRCS_BUILD_BSD = \
	Makefile

DRMLINUXINSTSRCDIR = $(INSTSRCDIR)/drm/linux/kernel
DRMBSDINSTSRCDIR = $(INSTSRCDIR)/drm/bsd/kernel

SRCS_SHARED = $(SRCS_DRM_SHARED) $(SRCS_GAMMA_SHARED) $(SRCS_I810_SHARED) \
	$(SRCS_I830_SHARED) $(SRCS_R128_SHARED) \
	$(SRCS_RADEON_SHARED) $(SRCS_SIS_SHARED) \
	$(SRCS_MGA_SHARED) $(SRCS_TDFX_SHARED)

SRCS_LINUX = $(SRCS_DRM_LINUX) $(SRCS_GAMMA_LINUX) $(SRCS_I810_LINUX) \
	$(SRCS_I830_LINUX) $(SRCS_R128_LINUX) \
	$(SRCS_RADEON_LINUX) $(SRCS_SIS_LINUX) \
	$(SRCS_MGA_LINUX) $(SRCS_TDFX_LINUX) \
	$(SRCS_BUILD_LINUX)

SRCS_BSD = $(SRCS_DRM_BSD) $(SRCS_GAMMA_BSD) $(SRCS_I810_BSD) \
	$(SRCS_I830_BSD) $(SRCS_R128_BSD) \
	$(SRCS_RADEON_BSD) $(SRCS_SIS_BSD) \
	$(SRCS_MGA_BSD) $(SRCS_TDFX_BSD) \
	$(SRCS_BUILD_BSD)


BSD_SUBDIRS = mga r128 radeon sis tdfx
SRCS_BSD_SUBS = $(BSD_SUBDIRS:%=%/Makefile)

INSTALL_SHARED = $(SRCS_SHARED:%=DRMCopySubdir/%)
INSTALL_LINUX = $(SRCS_LINUX:%=DRMCopySubdir/%)
INSTALL_BSD = $(SRCS_BSD:%=DRMCopySubdir/%)
INSTALL_BSD_SUBS = $(SRCS_BSD_SUBS:%=DRMCopySubdir/%)

DRMSHAREDDIR = $(XF86OSSRC)/shared/drm/kernel
DRMLINUXDIR = $(XF86OSSRC)/linux/drm/kernel
DRMBSDDIR = $(XF86OSSRC)/bsd/drm/kernel

