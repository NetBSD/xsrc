
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
 * 
 * Author: David Dawes <dawes@x-oz.com>.
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86AutoConfig.c,v 1.8 2005/02/19 01:02:34 dawes Exp $ */

#include "xf86.h"
#include "xf86Parser.h"
#include "xf86tokens.h"
#include "xf86Config.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86Bus.h"

#if defined(__sparc__) && !defined(__OpenBSD__)
#define SBUS_SUPPORT
#endif

/*
 * Sections for the default built-in configuration.
 */

#define BUILTIN_MODULE_SECTION \
	"Section \"Module\"\n" \
	"\tIdentifier\t\"Builtin Default Modules\"\n" \
	"\tLoad\t\"extmod\"\n" \
	"\tLoad\t\"dbe\"\n" \
	"\tLoad\t\"glx\"\n" \
	"\tLoad\t\"freetype\"\n" \
	"EndSection\n\n"

#define BUILTIN_DEVICE_NAME \
	"\"Builtin Default %s Device %d\""

#define BUILTIN_DEVICE_SECTION_PRE \
	"Section \"Device\"\n" \
	"\tIdentifier\t" BUILTIN_DEVICE_NAME "\n" \
	"\tDriver\t\"%s\"\n"

#define BUILTIN_DEVICE_SECTION_POST \
	"EndSection\n\n"

#define BUILTIN_DEVICE_SECTION \
	BUILTIN_DEVICE_SECTION_PRE \
	BUILTIN_DEVICE_SECTION_POST

#define BUILTIN_MONITOR_NAME \
	"\"Builtin Default Monitor\""

#define BUILTIN_MONITOR_SECTION \
	"Section \"Monitor\"\n" \
	"\tIdentifier\t" BUILTIN_MONITOR_NAME "\n" \
	"\tOption\t\"TargetRefresh\"\t\"75.0\"\n" \
	"EndSection\n\n"

#define BUILTIN_SCREEN_NAME \
	"\"Builtin Default %s Screen %d\""

#define BUILTIN_SCREEN_SECTION \
	"Section \"Screen\"\n" \
	"\tIdentifier\t" BUILTIN_SCREEN_NAME "\n" \
	"\tDevice\t" BUILTIN_DEVICE_NAME "\n" \
	"\tMonitor\t" BUILTIN_MONITOR_NAME "\n" \
	"EndSection\n\n"

#define BUILTIN_LAYOUT_SECTION_PRE \
	"Section \"ServerLayout\"\n" \
	"\tIdentifier\t\"Builtin Default Layout\"\n"

#define BUILTIN_LAYOUT_SCREEN_LINE \
	"\tScreen\t" BUILTIN_SCREEN_NAME "\n"

#define BUILTIN_LAYOUT_SECTION_POST \
	"EndSection\n\n"


#ifndef GET_CONFIG_CMD
#define GET_CONFIG_CMD			"getconfig"
#endif

#ifndef GETCONFIG_DIR
#define GETCONFIG_DIR			PROJECTROOT "/lib/X11/getconfig"
#endif

#define GETCONFIG_WHITESPACE		" \t\n"

static const char **builtinConfig = NULL;
static int builtinLines = 0;
static const char *deviceList[] = {
	"fbdev",
	"vesa",
	"vga",
	NULL
};

/*
 * A built-in config file is stored as an array of strings, with each string
 * representing a single line.  AppendToConfig() breaks up the string "s"
 * into lines, and appends those lines it to builtinConfig.
 */

static void
AppendToList(const char *s, const char ***list, int *lines)
{
    char *str, *newstr, *p;

    str = xnfstrdup(s);
    for (p = strtok(str, "\n"); p; p = strtok(NULL, "\n")) {
	(*lines)++;
	*list = xnfrealloc(*list, (*lines + 1) * sizeof(**list));
	newstr = xnfalloc(strlen(p) + 2);
	strcpy(newstr, p);
	strcat(newstr, "\n");
	(*list)[*lines - 1] = newstr;
	(*list)[*lines] = NULL;
    }
    xfree(str);
}

static void
FreeList(const char ***list, int *lines)
{
    int i;

    for (i = 0; i < *lines; i++) {
	if ((*list)[i])
	    xfree((*list)[i]);
    }
    xfree(*list);
    *list = NULL;
    *lines = 0;
}

static void
FreeConfig(void)
{
    FreeList(&builtinConfig, &builtinLines);
}

static void
AppendToConfig(const char *s)
{
    AppendToList(s, &builtinConfig, &builtinLines);
}

Bool
xf86AutoConfig(void)
{
    const char **p;
    char buf[1024];
    pciVideoPtr *pciptr, info = NULL;
    char *driver = NULL;
    FILE *gp = NULL;
    XF86ConfigPtr pConfig;
    Bool foundDev = FALSE;
#ifdef SBUS_SUPPORT
    char *promPath;
#endif

    /* Find the primary device, and get some information about it. */
    if (xf86PciVideoInfo) {
	for (pciptr = xf86PciVideoInfo; (info = *pciptr); pciptr++) {
	    if (xf86IsPrimaryPci(info)) {
		foundDev = TRUE;
		break;
	    }
	}
	if (!info) {
	    xf86MsgVerb(X_INFO, 3, "AutoConfig: Primary device is not PCI.\n");
	}
    } else {
	xf86MsgVerb(X_INFO, 3, "AutoConfig: xf86PciVideoInfo is not set.\n");
    }
#ifdef SBUS_SUPPORT
    if (!foundDev) {
	sbusDevicePtr psdp, *psdpp;
	Bool useProm = FALSE;

	if (xf86SbusInfo) {
	    if (sparcPromInit() >= 0)
		useProm = TRUE;
	    for (psdpp = xf86SbusInfo; (psdp = *psdpp); psdpp++) {
		if (psdp->fd == -2)
		    continue;
		foundDev = TRUE;
		if (useProm && psdp->node.node)
		    promPath = sparcPromNode2Pathname(&psdp->node);
		else
		    xasprintf(&promPath, "fb%d", psdp->fbNum);
		break;
	    }
	    if (useProm)
		sparcPromClose();
	} else {
	    xf86MsgVerb(X_INFO, 3, "AutoConfig: xf86SbusInfo is not set.\n");
	}
    }
#endif

    if (!foundDev)
	xf86Msg(X_WARNING,
		"AutoConfig: Cannot detect the primary video device.\n");

    if (foundDev) {
	char *tmp;
	char *path = NULL, *a, *b;
	char *searchPath = NULL;

	/*
	 * Look for the getconfig program first in the
	 * xf86FilePaths->modulePath directories, then in BINDIR.
	 * If it isn't found in any of those locations, just use the normal
	 * search path.
	 */

	a = xstrdup(xf86FilePaths->modulePath);
	if (a) {
	    b = strtok(a, ",");
	    while (b) {
		if (path) {
		    xfree(path);
		    path = NULL;
		}
		xasprintf(&path, "%s/%s", b, GET_CONFIG_CMD);
		if (path && access(path, X_OK) == 0)
		    break;
		b = strtok(NULL, ",");
	    }
	    if (!b && path) {
		xfree(path);
		path = NULL;
	    }
	    xfree(a);
	}

#ifdef BINDIR
	if (!path) {
	    path = xnfstrdup(BINDIR "/" GET_CONFIG_CMD);
	    if (access(path, X_OK) != 0) {
		xfree(path);
		path = NULL;
	    }
	}
#endif

	if (!path)
	    path = xnfstrdup(GET_CONFIG_CMD);

	/*
	 * Build up the config file directory search path:
	 *
	 * /etc/X11
	 * PROJECTROOT/etc/X11
	 * xf86FilePaths->modulePath
	 * PROJECTROOT/lib/X11/getconfig  (GETCONFIG_DIR)
	 */

	searchPath = xnfalloc(strlen("/etc/X11") + 1 +
			      strlen(PROJECTROOT "/etc/X11") + 1 +
			      (xf86FilePaths->modulePath ?
				    strlen(xf86FilePaths->modulePath) : 0)
				+ 1 +
			      strlen(GETCONFIG_DIR) + 1);
	strcpy(searchPath, "/etc/X11," PROJECTROOT "/etc/X11,");
	if (xf86FilePaths->modulePath && *xf86FilePaths->modulePath) {
	    strcat(searchPath, xf86FilePaths->modulePath);
	    strcat(searchPath, ",");
	}
	strcat(searchPath, GETCONFIG_DIR);

	if (info) {
	    xf86MsgVerb(X_INFO, 3, "AutoConfig: Primary PCI is %d:%d:%d\n",
			info->bus, info->device, info->func);
	}
#ifdef SBUS_SUPPORT
	else if (promPath) {
	    xf86MsgVerb(X_INFO, 3, "AutoConfig: Primary SBUS is %s\n",
			promPath);
	}
#endif

	if (info) {
	    snprintf(buf, sizeof(buf), "%s"
#ifdef DEBUG
		 " -D"
#endif
		 " -X %d"
		 " -I %s"
		 " -v 0x%04x -d 0x%04x -r 0x%02x -s 0x%04x"
		 " -b 0x%04x -c 0x%04x",
		 path,
		 (unsigned int)xf86GetVersion(),
		 searchPath,
		 info->vendor, info->chipType, info->chipRev,
		 info->subsysVendor, info->subsysCard,
		 info->class << 8 | info->subclass);
	}
#ifdef SBUS_SUPPORT
	else if (promPath) {
	    snprintf(buf, sizeof(buf), "%s"
#ifdef DEBUG
		 " -D"
#endif
		 " -X %d"
		 " -I %s"
		 " -S %s",
		 path,
		 (unsigned int)xf86GetVersion(),
		 searchPath,
		 promPath);
	    xfree(promPath);
	}
#endif
	xf86MsgVerb(X_INFO, 3, "AutoConfig: Running \"%s\".\n", buf);
	gp = Popen(buf, "r");
	if (gp) {
	    if (fgets(buf, sizeof(buf) - 1, gp)) {
		buf[strlen(buf) - 1] = '\0';
		tmp = strtok(buf, GETCONFIG_WHITESPACE);
		if (tmp)
		    driver = xnfstrdup(tmp);
	    }
	}
	xfree(path);
	xfree(searchPath);
    }

    AppendToConfig(BUILTIN_MODULE_SECTION);
    AppendToConfig(BUILTIN_MONITOR_SECTION);

    if (driver) {
	snprintf(buf, sizeof(buf), BUILTIN_DEVICE_SECTION_PRE,
		 driver, 0, driver);
	AppendToConfig(buf);
	xf86MsgVerb(X_INFO, 3, "AutoConfig: New driver is \"%s\".\n", driver);
	buf[0] = '\t';
	while (fgets(buf + 1, sizeof(buf) - 2, gp)) {
	    AppendToConfig(buf);
	    xf86MsgVerb(X_INFO, 3, "AutoConfig: Extra line: '%.*s'.\n",
			(int)strlen(buf) - 2, buf + 1);
	}
	AppendToConfig(BUILTIN_DEVICE_SECTION_POST);
	snprintf(buf, sizeof(buf), BUILTIN_SCREEN_SECTION,
		 driver, 0, driver, 0);
	AppendToConfig(buf);
    }

    if (gp)
	Pclose(gp);

    for (p = deviceList; *p; p++) {
	snprintf(buf, sizeof(buf), BUILTIN_DEVICE_SECTION, *p, 0, *p);
	AppendToConfig(buf);
	snprintf(buf, sizeof(buf), BUILTIN_SCREEN_SECTION, *p, 0, *p, 0);
	AppendToConfig(buf);
    }

    AppendToConfig(BUILTIN_LAYOUT_SECTION_PRE);
    if (driver) {
	snprintf(buf, sizeof(buf), BUILTIN_LAYOUT_SCREEN_LINE, driver, 0);
	AppendToConfig(buf);
    }
    for (p = deviceList; *p; p++) {
	snprintf(buf, sizeof(buf), BUILTIN_LAYOUT_SCREEN_LINE, *p, 0);
	AppendToConfig(buf);
    }
    AppendToConfig(BUILTIN_LAYOUT_SECTION_POST);

#ifdef BUILTIN_EXTRA
    AppendToConfig(BUILTIN_EXTRA);
#endif

    if (driver)
	xfree(driver);

    xf86MsgVerb(X_DEFAULT, 0,
		"Using default built-in configuration (%d lines)\n",
		builtinLines);

    xf86MsgVerb(X_DEFAULT, 3, "--- Start of built-in configuration ---\n");
    for (p = builtinConfig; *p; p++)
	xf86ErrorFVerb(3, "\t%s", *p);
    xf86MsgVerb(X_DEFAULT, 3, "--- End of built-in configuration ---\n");
    
    xf86setBuiltinConfig(builtinConfig);
    pConfig = (XF86ConfigPtr)(xf86Info.config);
    xf86Info.config = xf86parseConfigFile(pConfig);
    FreeConfig();
    if (!xf86Info.config) {
	xf86Msg(X_ERROR, "Error parsing the built-in default configuration.\n");
	return FALSE;
    }
    return TRUE;
}

