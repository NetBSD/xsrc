/* $XFree86: xc/programs/Xserver/hw/xfree86/parser/xf86Parser.h,v 1.37 2005/01/26 05:31:50 dawes Exp $ */
/* 
 * 
 * Copyright (c) 1997  Metro Link Incorporated
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of the Metro Link shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Metro Link.
 * 
 */
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


/* 
 * This file contains the external interfaces for the XFree86 configuration
 * file parser.
 */
#ifndef _xf86Parser_h_
#define _xf86Parser_h_

#include "xf86Optrec.h"

/*
 * Define these here so that consumers of the parser data can use the same
 * alloc/free mechanism as the parser when manipulating parser data.
 */
#define xf86confmalloc malloc
#define xf86confrealloc realloc
#define xf86confcalloc calloc
#define xf86conffree free

typedef struct
{
	GenericListRec list;
	char *file_logfile;
	char *file_rgbpath;
	char *file_modulepath;
	char *file_inputdevs;
	char *file_fontpath;
	char *file_comment;
	char *file_identifier;
	XF86OptionPtr file_option_lst;
}
XF86ConfFilesRec, *XF86ConfFilesPtr;

/* Values for load_type */
#define XF86_LOAD_MODULE	0
#define XF86_LOAD_DRIVER	1

typedef struct
{
	GenericListRec list;
	int load_type;
	char *load_name;
	XF86OptionPtr load_opt;
	char *load_comment;
}
XF86LoadRec, *XF86LoadPtr;

typedef struct
{
	GenericListRec list;
	XF86LoadPtr mod_load_lst;
	char *mod_comment;
	char *mod_identifier;
	XF86OptionPtr mod_option_lst;
}
XF86ConfModuleRec, *XF86ConfModulePtr;

#define CONF_IMPLICIT_KEYBOARD	"Implicit Core Keyboard"

#define CONF_IMPLICIT_POINTER	"Implicit Core Pointer"

#define XF86CONF_PHSYNC    0x0001
#define XF86CONF_NHSYNC    0x0002
#define XF86CONF_PVSYNC    0x0004
#define XF86CONF_NVSYNC    0x0008
#define XF86CONF_INTERLACE 0x0010
#define XF86CONF_DBLSCAN   0x0020
#define XF86CONF_CSYNC     0x0040
#define XF86CONF_PCSYNC    0x0080
#define XF86CONF_NCSYNC    0x0100
#define XF86CONF_HSKEW     0x0200	/* hskew provided */
#define XF86CONF_BCAST     0x0400
#define XF86CONF_CUSTOM    0x0800	/* timing numbers customized by editor */
#define XF86CONF_VSCAN     0x1000

typedef struct
{
	GenericListRec list;
	char *ml_identifier;
	int ml_clock;
	int ml_hdisplay;
	int ml_hsyncstart;
	int ml_hsyncend;
	int ml_htotal;
	int ml_vdisplay;
	int ml_vsyncstart;
	int ml_vsyncend;
	int ml_vtotal;
	int ml_vscan;
	int ml_flags;
	int ml_hskew;
	char *ml_comment;
}
XF86ConfModeLineRec, *XF86ConfModeLinePtr;

typedef struct
{
	GenericListRec list;
	char *vp_identifier;
	XF86OptionPtr vp_option_lst;
	char *vp_comment;
}
XF86ConfVideoPortRec, *XF86ConfVideoPortPtr;

typedef struct
{
	GenericListRec list;
	char *va_identifier;
	char *va_vendor;
	char *va_board;
	char *va_busid;
	char *va_driver;
	XF86OptionPtr va_option_lst;
	XF86ConfVideoPortPtr va_port_lst;
	char *va_fwdref;
	char *va_comment;
}
XF86ConfVideoAdaptorRec, *XF86ConfVideoAdaptorPtr;

#define CONF_MAX_HSYNC 8
#define CONF_MAX_VREFRESH 8

typedef struct
{
	float hi, lo;
}
parser_range;

typedef struct
{
	int red, green, blue;
}
parser_rgb;

typedef struct
{
	GenericListRec list;
	char *modes_identifier;
	XF86ConfModeLinePtr mon_modeline_lst;
	char *modes_comment;
	XF86OptionPtr modes_option_lst;
}
XF86ConfModesRec, *XF86ConfModesPtr;

typedef struct
{
	GenericListRec list;
	char *ml_modes_str;
	XF86ConfModesPtr ml_modes;
}
XF86ConfModesLinkRec, *XF86ConfModesLinkPtr;

typedef struct
{
	GenericListRec list;
	char *mon_identifier;
	char *mon_vendor;
	char *mon_modelname;
	int mon_width;				/* in mm */
	int mon_height;				/* in mm */
	XF86ConfModeLinePtr mon_modeline_lst;
	int mon_n_hsync;
	parser_range mon_hsync[CONF_MAX_HSYNC];
	int mon_n_vrefresh;
	parser_range mon_vrefresh[CONF_MAX_VREFRESH];
	float mon_gamma_red;
	float mon_gamma_green;
	float mon_gamma_blue;
	XF86OptionPtr mon_option_lst;
	XF86ConfModesLinkPtr mon_modes_sect_lst;
	char *mon_comment;
}
XF86ConfMonitorRec, *XF86ConfMonitorPtr;

typedef struct
{
	GenericListRec list;
	char *monitor_str;
	XF86ConfMonitorPtr monitor;
	int monitor_num;
}
XF86ConfMonitorListRec, *XF86ConfMonitorListPtr;

#define CONF_MAXDACSPEEDS 4
#define CONF_MAXCLOCKS    128

typedef struct
{
	GenericListRec list;
	char *dev_identifier;
	char *dev_vendor;
	char *dev_board;
	char *dev_chipset;
	char *dev_busid;
	char *dev_card;
	char *dev_driver;
	char *dev_ramdac;
	int dev_dacSpeeds[CONF_MAXDACSPEEDS];
	int dev_videoram;
	int dev_textclockfreq;
	unsigned long dev_bios_base;
	unsigned long dev_mem_base;
	unsigned long dev_io_base;
	char *dev_clockchip;
	int dev_clocks;
	int dev_clock[CONF_MAXCLOCKS];
	int dev_chipid;
	int dev_chiprev;
	int dev_irq;
	int dev_screen;
	XF86OptionPtr dev_option_lst;
	char *dev_comment;
}
XF86ConfDeviceRec, *XF86ConfDevicePtr;

typedef struct
{
	GenericListRec list;
	char *mode_name;
}
XF86ModeRec, *XF86ModePtr;

typedef struct
{
	GenericListRec list;
	int disp_frameX0;
	int disp_frameY0;
	int disp_virtualX;
	int disp_virtualY;
	int disp_depth;
	int disp_bpp;
	char *disp_visual;
	parser_rgb disp_weight;
	parser_rgb disp_black;
	parser_rgb disp_white;
	XF86ModePtr disp_mode_lst;
	XF86OptionPtr disp_option_lst;
	char *disp_comment;
	int monitor_num;
}
XF86ConfDisplayRec, *XF86ConfDisplayPtr;

typedef struct
{
	GenericListRec list;
	XF86OptionPtr flg_option_lst;
	char *flg_comment;
	char *flg_identifier;
}
XF86ConfFlagsRec, *XF86ConfFlagsPtr;

typedef struct
{
	GenericListRec list;
	char *al_adaptor_str;
	XF86ConfVideoAdaptorPtr al_adaptor;
}
XF86ConfAdaptorLinkRec, *XF86ConfAdaptorLinkPtr;

typedef struct
{
	GenericListRec list;
	char *scrn_identifier;
	char *scrn_obso_driver;
	int scrn_defaultdepth;
	int scrn_defaultbpp;
	int scrn_defaultfbbpp;
	char *scrn_monitor_str;
	XF86ConfMonitorPtr scrn_monitor;
	XF86ConfMonitorListPtr scrn_monitor_lst;
	char *scrn_device_str;
	XF86ConfDevicePtr scrn_device;
	XF86ConfAdaptorLinkPtr scrn_adaptor_lst;
	XF86ConfDisplayPtr scrn_display_lst;
	XF86OptionPtr scrn_option_lst;
	char *scrn_comment;
}
XF86ConfScreenRec, *XF86ConfScreenPtr;

typedef struct
{
	GenericListRec list;
	char *inp_identifier;
	char *inp_driver;
	XF86OptionPtr inp_option_lst;
	char *inp_comment;
}
XF86ConfInputRec, *XF86ConfInputPtr;

typedef struct
{
	GenericListRec list;
	XF86ConfInputPtr iref_inputdev;
	char *iref_inputdev_str;
	XF86OptionPtr iref_option_lst;
}
XF86ConfInputrefRec, *XF86ConfInputrefPtr;

/* Values for adj_where */
#define CONF_ADJ_OBSOLETE	-1
#define CONF_ADJ_ABSOLUTE	0
#define CONF_ADJ_RIGHTOF	1
#define CONF_ADJ_LEFTOF		2
#define CONF_ADJ_ABOVE		3
#define CONF_ADJ_BELOW		4
#define CONF_ADJ_RELATIVE	5

typedef struct
{
	GenericListRec list;
	int adj_scrnum;
	XF86ConfScreenPtr adj_screen;
	char *adj_screen_str;
	XF86ConfScreenPtr adj_top;
	char *adj_top_str;
	XF86ConfScreenPtr adj_bottom;
	char *adj_bottom_str;
	XF86ConfScreenPtr adj_left;
	char *adj_left_str;
	XF86ConfScreenPtr adj_right;
	char *adj_right_str;
	int adj_where;
	int adj_x;
	int adj_y;
	char *adj_refscreen;
}
XF86ConfAdjacencyRec, *XF86ConfAdjacencyPtr;

typedef struct
{
	GenericListRec list;
	char *inactive_device_str;
	XF86ConfDevicePtr inactive_device;
}
XF86ConfInactiveRec, *XF86ConfInactivePtr;

typedef struct
{
	GenericListRec list;
	char *lay_identifier;
	XF86ConfAdjacencyPtr lay_adjacency_lst;
	XF86ConfInactivePtr lay_inactive_lst;
	XF86ConfInputrefPtr lay_input_lst;
	XF86OptionPtr lay_option_lst;
	char *lay_comment;
}
XF86ConfLayoutRec, *XF86ConfLayoutPtr;

typedef struct 
{ 
	GenericListRec list; 
	char *vs_name;
	char *vs_identifier;
	XF86OptionPtr vs_option_lst;
	char *vs_comment;
}
XF86ConfVendSubRec, *XF86ConfVendSubPtr;

typedef struct
{
	GenericListRec list;
	char *vnd_identifier;
	char *vnd_name;
	XF86OptionPtr vnd_option_lst;
	XF86ConfVendSubPtr vnd_sub_lst;
	char *vnd_comment;
}
XF86ConfVendorRec, *XF86ConfVendorPtr;

typedef struct
{
	GenericListRec list;
	int buf_count;
	int buf_size;
	char *buf_flags;
	char *buf_comment;
}
XF86ConfBuffersRec, *XF86ConfBuffersPtr;

typedef struct
{
	GenericListRec list;
	char *dri_group_name;
	int dri_group;
	int dri_mode;
	XF86ConfBuffersPtr dri_buffers_lst;
	char *dri_comment;
	char *dri_identifier;
	XF86OptionPtr dri_option_lst;
}
XF86ConfDRIRec, *XF86ConfDRIPtr;

typedef struct
{
	XF86ConfFilesPtr conf_files_lst;
	XF86ConfModulePtr conf_modules_lst;
	XF86ConfFlagsPtr conf_flags_lst;
	XF86ConfVideoAdaptorPtr conf_videoadaptor_lst;
	XF86ConfModesPtr conf_modes_lst;
	XF86ConfMonitorPtr conf_monitor_lst;
	XF86ConfDevicePtr conf_device_lst;
	XF86ConfScreenPtr conf_screen_lst;
	XF86ConfInputPtr conf_input_lst;
	XF86ConfLayoutPtr conf_layout_lst;
	XF86ConfVendorPtr conf_vendor_lst;
	XF86ConfDRIPtr conf_dri_lst;
	char *conf_comment;
}
XF86ConfigRec, *XF86ConfigPtr;

/* For build-time compatibility.  This will be removed at some point. */
#define conf_flags conf_flags_lst
#define conf_files conf_files_lst
#define conf_modules conf_modules_lst
#define conf_dri conf_dri_lst

typedef struct
{
	int token;			/* id of the token */
	char *name;			/* pointer to the LOWERCASED name */
}
xf86ConfigSymTabRec, *xf86ConfigSymTabPtr;

/*
 * prototypes for public functions
 */
const char *xf86openConfigFile (const char *, const char *, const char *);
void xf86setBuiltinConfig(const char *config[]);
XF86ConfigPtr xf86readConfigFile (void);
int xf86validateConfig(XF86ConfigPtr p);
XF86ConfigPtr xf86parseConfigFile (XF86ConfigPtr ptr);
void xf86closeConfigFile (void);
void xf86freeConfig (XF86ConfigPtr p);
int xf86writeConfigFile (const char *, XF86ConfigPtr);
XF86ConfDevicePtr xf86findDevice(const char *ident, XF86ConfDevicePtr p);
XF86ConfLayoutPtr xf86findLayout(const char *name, XF86ConfLayoutPtr list);
XF86ConfMonitorPtr xf86findMonitor(const char *ident, XF86ConfMonitorPtr p);
XF86ConfModesPtr xf86findModes(const char *ident, XF86ConfModesPtr p);
XF86ConfModeLinePtr xf86findModeLine(const char *ident, XF86ConfModeLinePtr p);
XF86ConfScreenPtr xf86findScreen(const char *ident, XF86ConfScreenPtr p);
XF86ConfInputPtr xf86findInput(const char *ident, XF86ConfInputPtr p);
XF86ConfInputPtr xf86findInputByDriver(const char *driver, XF86ConfInputPtr p);
XF86ConfVendorPtr xf86findVendor(const char *name, XF86ConfVendorPtr list);
XF86ConfVideoAdaptorPtr xf86findVideoAdaptor(const char *ident,
						XF86ConfVideoAdaptorPtr p);

void xf86freeDeviceList(XF86ConfDevicePtr ptr);
void xf86freeFilesList(XF86ConfFilesPtr ptr);
void xf86freeFlagsList(XF86ConfFlagsPtr ptr);
void xf86freeInputList(XF86ConfInputPtr ptr);
void xf86freeLayoutList(XF86ConfLayoutPtr ptr);
void xf86freeAdjacencyList(XF86ConfAdjacencyPtr ptr);
void xf86freeInputrefList(XF86ConfInputrefPtr ptr);
void xf86freeModulesList(XF86ConfModulePtr ptr);
void xf86freeMonitorList(XF86ConfMonitorPtr ptr);
void xf86freeModesList(XF86ConfModesPtr ptr);
void xf86freeModeLineList(XF86ConfModeLinePtr ptr);
void xf86freeScreenList(XF86ConfScreenPtr ptr);
void xf86freeAdaptorLinkList(XF86ConfAdaptorLinkPtr ptr);
void xf86freeDisplayList(XF86ConfDisplayPtr ptr);
void xf86freeModeList(XF86ModePtr ptr);
void xf86freeVendorList(XF86ConfVendorPtr p);
void xf86freeVendorSubList (XF86ConfVendSubPtr ptr);
void xf86freeVideoAdaptorList(XF86ConfVideoAdaptorPtr ptr);
void xf86freeVideoPortList(XF86ConfVideoPortPtr ptr);
void xf86freeBuffersList (XF86ConfBuffersPtr ptr);
void xf86freeDRIList (XF86ConfDRIPtr ptr);

GenericListPtr xf86addListItem(GenericListPtr head, GenericListPtr c_new);
int xf86itemNotSublist(GenericListPtr list_1, GenericListPtr list_2);

int xf86pathIsAbsolute(const char *path);
int xf86pathIsSafe(const char *path);
char *xf86addComment(char *cur, char *add);

#endif /* _xf86Parser_h_ */
