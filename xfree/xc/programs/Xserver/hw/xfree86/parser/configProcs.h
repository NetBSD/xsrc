/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/configProcs.h,v 1.20 2005/01/26 05:31:50 dawes Exp $ */
/*
 * Copyright (c) 1997-2005 by The XFree86 Project, Inc.
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
/*
 * Copyright © 2003, 2004, 2005 David H. Dawes.
 * Copyright © 2003, 2004, 2005 X-Oz Technologies.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions, and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 * 
 *  3. The end-user documentation included with the redistribution,
 *     if any, must include the following acknowledgment: "This product
 *     includes software developed by X-Oz Technologies
 *     (http://www.x-oz.com/)."  Alternately, this acknowledgment may
 *     appear in the software itself, if and wherever such third-party
 *     acknowledgments normally appear.
 *
 *  4. Except as contained in this notice, the name of X-Oz
 *     Technologies shall not be used in advertising or otherwise to
 *     promote the sale, use or other dealings in this Software without
 *     prior written authorization from X-Oz Technologies.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL X-OZ TECHNOLOGIES OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Private procs.  Public procs are in xf86Parser.h and xf86Optrec.h */

/* Device.c */
XF86ConfDevicePtr xf86parseDeviceSection(void);
void xf86printDeviceSection(FILE *cf, XF86ConfDevicePtr ptr);
int xf86validateDevice(XF86ConfigPtr p);
/* Files.c */
XF86ConfFilesPtr xf86parseFilesSection(void);
void xf86printFileSection(FILE *cf, XF86ConfFilesPtr ptr);
/* Flags.c */
XF86ConfFlagsPtr xf86parseFlagsSection(void);
void xf86printServerFlagsSection(FILE *f, XF86ConfFlagsPtr flags);
/* Input.c */
XF86ConfInputPtr xf86parseInputSection(void);
void xf86printInputSection(FILE *f, XF86ConfInputPtr ptr);
int xf86validateInput (XF86ConfigPtr p);
/* Keyboard.c */
XF86ConfInputPtr xf86parseKeyboardSection(void);
/* Layout.c */
XF86ConfLayoutPtr xf86parseLayoutSection(void);
void xf86printLayoutSection(FILE *cf, XF86ConfLayoutPtr ptr);
int xf86validateLayout(XF86ConfigPtr p);
/* Module.c */
XF86LoadPtr xf86parseModuleSubSection(XF86LoadPtr head, char *name);
XF86ConfModulePtr xf86parseModuleSection(void);
void xf86printModuleSection(FILE *cf, XF86ConfModulePtr ptr);
XF86LoadPtr xf86addNewLoadDirective(XF86LoadPtr head, char *name, int type, XF86OptionPtr opts);
/* Monitor.c */
XF86ConfModeLinePtr xf86parseModeLine(void);
XF86ConfModeLinePtr xf86parseVerboseMode(void);
XF86ConfMonitorPtr xf86parseMonitorSection(void);
XF86ConfModesPtr xf86parseModesSection(void);
void xf86printMonitorSection(FILE *cf, XF86ConfMonitorPtr ptr);
void xf86printModesSection(FILE *cf, XF86ConfModesPtr ptr);
int xf86validateMonitor(XF86ConfigPtr p, XF86ConfScreenPtr screen);
/* Pointer.c */
XF86ConfInputPtr xf86parsePointerSection(void);
/* Screen.c */
XF86ConfDisplayPtr xf86parseDisplaySubSection(void);
XF86ConfScreenPtr xf86parseScreenSection(void);
void xf86printScreenSection(FILE *cf, XF86ConfScreenPtr ptr);
int xf86validateScreen(XF86ConfigPtr p);
/* Vendor.c */
XF86ConfVendorPtr xf86parseVendorSection(void);
XF86ConfVendSubPtr xf86parseVendorSubSection (XF86ConfVendSubPtr head, char *name);
void xf86printVendorSection(FILE * cf, XF86ConfVendorPtr ptr);
/* Video.c */
XF86ConfVideoPortPtr xf86parseVideoPortSubSection(char *name);
XF86ConfVideoAdaptorPtr xf86parseVideoAdaptorSection(void);
void xf86printVideoAdaptorSection(FILE *cf, XF86ConfVideoAdaptorPtr ptr);
/* scan.c */
unsigned int xf86strToUL(char *str);
int xf86getToken(xf86ConfigSymTabRec *tab);
int xf86getSubToken(char **comment);
int xf86getSubTokenWithTab(char **comment, xf86ConfigSymTabRec *tab);
void xf86unGetToken(int token);
char *xf86tokenString(void);
void xf86parseError(char *format, ...);
void xf86parseWarning(char *format, ...);
void xf86validationError(char *format, ...);
void xf86setSection(char *section);
int xf86getStringToken(xf86ConfigSymTabRec *tab);
/* write.c */
/* DRI.c */
XF86ConfBuffersPtr xf86parseBuffers (void);
XF86ConfDRIPtr xf86parseDRISection (void);
void xf86printDRISection (FILE * cf, XF86ConfDRIPtr ptr);

#ifndef IN_XSERVER
/* Externally provided functions */
void ErrorF(const char *f, ...);
void VErrorF(const char *f, va_list args);
#endif
