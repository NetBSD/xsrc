/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86Config.c,v 3.289 2005/03/02 19:17:43 dawes Exp $ */


/*
 * Loosely based on code bearing the following copyright:
 *
 *   Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany.
 */

/*
 * Copyright (c) 1992-2005 by The XFree86 Project, Inc.
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
 * Copyright 1997 by Metro Link, Inc.
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
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the copyright holder(s)
 * and author(s) shall not be used in advertising or otherwise to promote
 * the sale, use or other dealings in this Software without prior written
 * authorization from the copyright holder(s) and author(s).
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
 * 
 */

/*
 *
 * Authors:
 *	Dirk Hohndel <hohndel@XFree86.Org>
 *	David Dawes <dawes@XFree86.Org>
 *      Marc La France <tsi@XFree86.Org>
 *      Egbert Eich <eich@XFree86.Org>
 *      ... and others
 */

#include <sys/types.h>
#include <grp.h>

#ifdef __UNIXOS2__
#define I_NEED_OS2_H
#endif

#include "xf86.h"
#include "xf86Parser.h"
#include "xf86tokens.h"
#include "xf86Config.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#include "globals.h"

#ifdef XKB
#define XKB_IN_SERVER
#include "XKBsrv.h"
#endif

#ifdef RENDER
#include "picture.h"
#endif

#if (defined(i386) || defined(__i386__)) && \
    (defined(__FreeBSD__) || defined(__NetBSD__) || defined(linux) || \
     (defined(SVR4) && !defined(sun)) || defined(__GNU__))
#define SUPPORT_PC98
#endif

/*
 * These paths define the way the config file search is done.  The escape
 * sequences are documented in parser/scan.c.
 */
#ifndef ROOT_CONFIGPATH
#define ROOT_CONFIGPATH	"%A," "%R," \
			"/etc/X11/%R," "%P/etc/X11/%R," \
			"%E," "%F," \
			"/etc/X11/%F," "%P/etc/X11/%F," \
			"%D/%X," \
			"/etc/X11/%X-%M," "/etc/X11/%X," "/etc/%X," \
			"%P/etc/X11/%X.%H," "%P/etc/X11/%X-%M," \
			"%P/etc/X11/%X," \
			"%P/lib/X11/%X.%H," "%P/lib/X11/%X-%M," \
			"%P/lib/X11/%X"
#endif
#ifndef USER_CONFIGPATH
#define USER_CONFIGPATH	"/etc/X11/%S," "%P/etc/X11/%S," \
			"/etc/X11/%G," "%P/etc/X11/%G," \
			"/etc/X11/%X-%M," "/etc/X11/%X," "/etc/%X," \
			"%P/etc/X11/%X.%H," "%P/etc/X11/%X-%M," \
			"%P/etc/X11/%X," \
			"%P/lib/X11/%X.%H," "%P/lib/X11/%X-%M," \
			"%P/lib/X11/%X"
#endif
#ifndef PROJECTROOT
#define PROJECTROOT	"/usr/X11R6"
#endif

/* Forward declarations */
static MonPtr convertMonitor(MonPtr pMonitor,
			     const XF86ConfMonitorRec *confMonitor);
static IDevPtr convertInputDevice(IDevPtr input,
				  const XF86ConfInputRec *confInput);
static DispPtr convertDisplay(DispPtr pDisplay,
			      const XF86ConfDisplayRec *confDisplay);
static confModulesPtr convertModules(confModulesPtr modules,
					 const XF86ConfModuleRec* confModules);
static Bool addDefaultModes(MonPtr pMonitor);
static confDRIPtr configDRI(ConfigHandle handle, const XF86ConfDRIRec *driConf,
			    MessageType from);
static const confXvAdaptorRec *getNextActiveXvAdaptor(ConfigDataHandle prev);
static const confScreenRec *getNextActiveScreen(ConfigDataHandle prev);

static void
xf86ConfigError(const char *msg, ...)
{
    va_list ap;

    ErrorF("\nConfig Error:\n");
    va_start(ap, msg);
    VErrorF(msg, ap);
    va_end(ap);
    ErrorF("\n");
    return;
}

static const XF86ConfigRec *
ConfigHandleToConfigRead(ConfigHandle handle)
{
    /* For now, effectively ignore the pseudo handles. */
    if (handle == DEFAULT_CONFIG ||
	handle == CMDLINE_CONFIG || handle == BUILDTIME_CONFIG ||
	handle == ACTIVE_CONFIG)
	return xf86Info.config;
    else
	return handle;
}

#ifdef NOTYET
static XF86ConfigPtr
ConfigHandleToConfigWrite(ConfigHandle handle)
{
    if (handle == DEFAULT_CONFIG)
	return (XF86ConfigPtr)xf86Info.config;
    else
	return (XF86ConfigPtr)handle;
}
#endif

static void
xf86FreeStringArray(char **s, int n)
{
    int i;

    if (!s)
	return;

    for (i = 0; i < n; i++) {
	xfree(s[i]);
	s[i] = NULL;
    }
    xfree(s);
}

static char **
xf86DupStringArray(char **src, int n)
{
    int i;

    char **dst;

    if (n < 1)
	return NULL;

    dst = xcalloc(n, sizeof(*dst));
    if (!dst)
	return NULL;

    for (i = 0; i < n; i++) {
	dst[i] = xstrdup(src[i]);
	if (src[i] && !dst[i]) {
	    xf86FreeStringArray(dst, n);
	    return NULL;
	}
    }
    return dst;
}

confLoadModulePtr
xf86ConfAllocLoadModule()
{
    confLoadModulePtr pLoad;

    pLoad = xcalloc(1, sizeof(*pLoad));
    return pLoad;
}

void
xf86ConfFreeLoadModuleData(confLoadModulePtr pLoad)
{
    if (!pLoad)
	return;

    if (pLoad->name) {
	xfree(pLoad->name);
	pLoad->name = NULL;
    }
    if (pLoad->options) {
	xf86OptionListFree(pLoad->options);
	pLoad->options = NULL;
    }
}

confLoadModulePtr
xf86ConfDupLoadModule(const confLoadModuleRec *pLoad)
{
    confLoadModulePtr l;

    if (!pLoad)
	return NULL;

    l = xf86ConfAllocLoadModule();
    if (!l)
	return NULL;

    l->name = xstrdup(pLoad->name);
    if (pLoad->name && !l->name) {
	xf86ConfFreeLoadModuleData(l);
	xfree(l);
	return NULL;
    }
    l->options = xf86OptionListDup(pLoad->options);
    if (pLoad->options && !l->options) {
	xf86ConfFreeLoadModuleData(l);
	xfree(l);
	return NULL;
    }
    return l;
}

static confLoadModulePtr
convertLoadModule(confLoadModulePtr pLoad, const XF86LoadRec *loadConf)
{
    if (!pLoad || !loadConf)
	return NULL;

    pLoad->name = xstrdup(loadConf->load_name);
    if (loadConf->load_name && !pLoad->name) {
	xf86ConfFreeLoadModuleData(pLoad);
	return NULL;
    }
    pLoad->options = xf86OptionListDup(loadConf->load_opt);
    if (loadConf->load_opt && !pLoad->options) {
	xf86ConfFreeLoadModuleData(pLoad);
	return NULL;
    }
    return pLoad;
}

confModulesPtr
xf86ConfAllocModules()
{
    confModulesPtr pModules;

    pModules = xcalloc(1, sizeof(*pModules));
    pModules->handle = pModules;
    return pModules;
}

void
xf86ConfFreeModulesData(confModulesPtr pModules)
{
    int i;

    if (!pModules)
	return;

    if (pModules->identifier) {
	xfree(pModules->identifier);
	pModules->identifier = NULL;
    }
    if (pModules->options) {
	xf86OptionListFree(pModules->options);
	pModules->options = NULL;
    }
    if (pModules->modules) {
	for (i = 0; i < pModules->numModules; i++) {
	    xf86ConfFreeLoadModuleData(pModules->modules[i]);
	    xfree(pModules->modules[i]);
	    pModules->modules[i] = NULL;
	}
	xfree(pModules->modules);
	pModules->modules = NULL;
    }
    pModules->numModules = 0;
}

confModulesPtr
xf86ConfDupModules(const confModulesRec *pModules)
{
    confModulesPtr m;
    int i;

    if (!pModules)
	return NULL;

    m = xf86ConfAllocModules();
    if (!m)
	return NULL;

    m->handle = pModules->handle;
    m->identifier = xstrdup(pModules->identifier);
    if (pModules->identifier && !m->identifier) {
	xf86ConfFreeModulesData(m);
	xfree(m);
	return NULL;
    }
    m->options = xf86OptionListDup(pModules->options);
    if (pModules->options && !m->options) {
	xf86ConfFreeModulesData(m);
	xfree(m);
	return NULL;
    }
    m->numModules = pModules->numModules;
    if (m->numModules > 0 && pModules->modules) {
	m->modules = xcalloc(m->numModules, sizeof(confLoadModulePtr));
	if (!m->modules) {
	    xf86ConfFreeModulesData(m);
	    xfree(m);
	    return NULL;
	}
	for (i = 0; i < m->numModules; i++) {
	    m->modules[i] = xf86ConfDupLoadModule(pModules->modules[i]);
	    if (pModules->modules[i] && !m->modules[i]) {
		xf86ConfFreeModulesData(m);
		xfree(m);
		return NULL;
	    }
	}
    }
    return m;
}

static confModulesPtr
convertModules(confModulesPtr pModules, const XF86ConfModuleRec* confModules)
{
    XF86LoadPtr lp;
    int count, i;

    if (!pModules || !confModules)
	return NULL;

    pModules->handle = confModules;
    pModules->identifier = xstrdup(confModules->mod_identifier);
    if (confModules->mod_identifier && !pModules->identifier) {
	xf86ConfFreeModulesData(pModules);
	return NULL;
    }
    pModules->options = xf86OptionListDup(confModules->mod_option_lst);
    if (confModules->mod_option_lst && !pModules->options) {
	xf86ConfFreeModulesData(pModules);
	return NULL;
    }

    /* Count how many module load references there are. */
    lp = confModules->mod_load_lst;
    count = 0;
    while (lp) {
	count++;
	lp = lp->list.next;
    }
    pModules->numModules = count;
    if (count > 0) {
	pModules->modules = xcalloc(count, sizeof(confLoadModulePtr));
	if (!pModules->modules) {
	    xf86ConfFreeModulesData(pModules);
	    return NULL;
	}
	lp = confModules->mod_load_lst;
	for (i = 0; i < count; i++) {
	    pModules->modules[i] = xf86ConfAllocLoadModule();
	    if (!pModules->modules[i]) {
		xf86ConfFreeModulesData(pModules);
		return NULL;
	    }
	    if (!convertLoadModule(pModules->modules[i], lp)) {
		xf86ConfFreeModulesData(pModules);
		return NULL;
	    }
	    lp = lp->list.next;
	}
    }
    return pModules;
}

confModulesPtr
xf86ConfGetModulesByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    const XF86ConfModuleRec *m;
    confModulesPtr pModules = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (m = pConfig->conf_modules_lst; m; m = m->list.next) {
	if (xf86nameCompare(m->mod_identifier, name) == 0)
	    break;
    }
    if (m) {
	pModules = xf86ConfAllocModules();
	if (pModules) {
	    if (convertModules(pModules, m))
		return pModules;
	    else
		xfree(pModules);
	}
    }
    return NULL;
}

confModulesPtr
xf86ConfGetNextModules(ConfigHandle handle,
		       ConfigDataHandle prevModulesHandle)
{
    const XF86ConfigRec *pConfig;
    const XF86ConfModuleRec *m;
    confModulesPtr pModules = NULL;

    /* The active special case. */
    if (handle == ACTIVE_CONFIG) {
	if (!prevModulesHandle)
	    return xf86ConfDupModules(xf86Info.confModules);
	else
	    return NULL;
    }

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevModulesHandle) {
	m = pConfig->conf_modules_lst;
    } else {
	for (m = pConfig->conf_modules_lst; m; m = m->list.next) {
	    if (m == prevModulesHandle) {
		m = m->list.next;
		break;
	    }
	}
    }
    if (m) {
	pModules = xf86ConfAllocModules();
	if (pModules) {
	    if (convertModules(pModules, m))
		return pModules;
	    else
		xfree(pModules);
	}
    }
    return NULL;
}

confModulesPtr
xf86ConfCombineModulesData(const confModulesRec *src1,
			   const confModulesRec *src2)
{
    confModulesPtr dst = NULL;
    pointer opt1 = NULL, opt2 = NULL;
    int i, j;

    dst = xf86ConfAllocModules();
    if (!dst)
	return NULL;

    dst->numModules = 0;
    if (src1)
	dst->numModules = src1->numModules;
    if (src2)
	dst->numModules += src2->numModules;

    if (dst->numModules > 0) {
	dst->modules = xcalloc(dst->numModules, sizeof(confLoadModulePtr));
	if (!dst->modules) {
	    xf86ConfFreeModulesData(dst);
	    xfree(dst);
	    return NULL;
	}
	j = 0;
	if (src1) {
	    for (i = 0; i < src1->numModules; i++) {
		dst->modules[j] = xf86ConfAllocLoadModule();
		if (!dst->modules) {
		    xf86ConfFreeModulesData(dst);
		    xfree(dst);
		    return NULL;
		}
		dst->modules[j]->name = xstrdup(src1->modules[i]->name);
		dst->modules[j]->options =
			xf86OptionListDup(src1->modules[i]->options);
		j++;
	    }
	}
	if (src2) {
	    for (i = 0; i < src2->numModules; i++) {
		dst->modules[j] = xf86ConfAllocLoadModule();
		if (!dst->modules) {
		    xf86ConfFreeModulesData(dst);
		    xfree(dst);
		    return NULL;
		}
		dst->modules[j]->name = xstrdup(src2->modules[i]->name);
		dst->modules[j]->options =
			xf86OptionListDup(src2->modules[i]->options);
		j++;
	    }
	}
    }
    if (src1)
	opt1 = xf86OptionListDup(src1->options);
    if (src2)
	opt2 = xf86OptionListDup(src2->options);
    dst->options = xf86OptionListMerge(opt2, opt1);
    dst->handle = dst;
    return dst;
}

/*
 * Process the Modules sections, combining them into a single list of modules
 * to load explicitly.
 */
static confModulesPtr
configModules(ConfigHandle handle, const XF86ConfModuleRec *confModules,
	      MessageType from)
{
    confModulesPtr m = NULL, m1, m2;
    const void *mh;
    
    if (confModules) {
	m = xf86ConfAllocModules();
	if (!m)
	    return NULL;
	if (!convertModules(m, confModules)) {
	    xfree(m);
	    return NULL;
	}
    } else {
	m1 = xf86ConfGetNextModules(handle, NULL);
	if (m1) {
	    mh = m1->handle;
	    do {
		m2 = xf86ConfGetNextModules(handle, mh);
		if (m2) {
		    mh = m2->handle;
		    m = xf86ConfCombineModulesData(m1, m2);
		    xf86ConfFreeModulesData(m1);
		    xf86ConfFreeModulesData(m2);
		    xfree(m1);
		    xfree(m2);
		    if (!m)
			return NULL;
		    m1 = m;
		} else {
		    m = m1;
		    mh = NULL;
		}
	    } while (mh);
	}
    }
    if (!m)
	m = xf86ConfAllocModules();
    return m;
}

void
xf86ModulelistFree(const char **moduleList, pointer *optList)
{
    int i;

    if (moduleList) {
	for (i = 0; moduleList[i]; i++)
	    xfree(moduleList[i]);
	xfree(moduleList);
    }
    if (optList) {
	for (i = 0; optList[i]; i++)
	    xf86OptionListFree(optList[i]);
	xfree(optList);
    }
}

const char **
xf86ModulelistFromConfig(pointer **optList)
{
    const char **moduleArray = NULL;
    pointer *optArray = NULL;
    int i, n;
    
    if (!optList)
	return NULL;

    if (xf86Info.confModules && xf86Info.confModules->numModules > 0) {
	n = xf86Info.confModules->numModules;
	moduleArray = xcalloc(n + 1, sizeof(*moduleArray));
	optArray = xcalloc(n + 1, sizeof(*optArray));
	for (i = 0; i < n; i++) {
	    moduleArray[i] = xstrdup(xf86Info.confModules->modules[i]->name);
	    if (!moduleArray[i]) {
		xf86ModulelistFree(moduleArray, optArray);
		return NULL;
	    }
	    optArray[i] =
		xf86OptionListDup(xf86Info.confModules->modules[i]->options);
	    if (xf86Info.confModules->modules[i]->options && !optArray[i]) {
		xf86ModulelistFree(moduleArray, optArray);
		return NULL;
	    }
	}
	moduleArray[n] = NULL;
	optArray[n] = NULL;
    }
    *optList = optArray;
    return moduleArray;
}

const char **
xf86DriverlistFromConfig()
{
    int count = 0;
    int i, j;
    const char **modulearray;
    screenLayoutPtr pScreenLayout;

    if (!xf86Info.serverLayout)
	return NULL;

    count = xf86Info.serverLayout->numScreens +
	    xf86Info.serverLayout->numInactives;
    if (count == 0)
	return NULL;

    modulearray = xnfalloc((count + 1) * sizeof(char*));
    j = 0;
    for (i = 0; i < xf86Info.serverLayout->numScreens; i++) {
	pScreenLayout = xf86Info.serverLayout->screenLayouts[i];
	modulearray[j] = pScreenLayout->screen->device->driver;
	j++;
    }

    for (i = 0; i < xf86Info.serverLayout->numInactives; i++) {
	modulearray[j] = xf86Info.serverLayout->inactiveDevs[i]->driver;
	j++;
    }

    modulearray[j] = NULL;

    /* Remove duplicates */
    for (i = 0; i < count; i++) {
	for (j = 0; j < i; j++) {
	    if (xf86NameCmp(modulearray[j], modulearray[i]) == 0) {
		modulearray[i] = "";
		break;
	    }
	}
    }
    return modulearray;
}


Bool
xf86BuiltinInputDriver(const char *name)
{
    if (xf86NameCmp(name, "keyboard") == 0)
	return TRUE;
    else
	return FALSE;
}


const char **
xf86InputDriverlistFromConfig()
{
    int i;
    const char **modulearray;

    if (!xf86Info.serverLayout)
	return NULL;

    if (xf86Info.serverLayout->numInputs == 0)
	return NULL;

    modulearray = xnfalloc((xf86Info.serverLayout->numInputs + 1) *
			   sizeof(char*));
    for (i = 0; i < xf86Info.serverLayout->numInputs; i++) {
	if (!xf86BuiltinInputDriver(
		xf86Info.serverLayout->inputDevs[i]->driver))
	    modulearray[i] = xf86Info.serverLayout->inputDevs[i]->driver;
	else
	    modulearray[i] = "";
    }
    modulearray[i] = NULL;

    /* Remove duplicates. */
    for (i = 0; i < xf86Info.serverLayout->numInputs; i++) {
	int j;

	for (j = 0; j < i; j++) {
	    if (xf86NameCmp(modulearray[j], modulearray[i]) == 0) {
		modulearray[i] = "";
		break;
	    }
	}
    }
    return modulearray;
}


/*
 * Generate a compiled-in list of driver names.  This is used to produce a
 * consistent probe order.  For the loader server, we also look for vendor-
 * provided modules, pre-pending them to our own list.
 */
static const char **
GenerateDriverlist(char * dirname, char * drivernames)
{
    char *cp, **driverlist;
    int count;

    /* Count the number needed */
    count = 0;
    cp = drivernames;
    while (*cp) {
	while (*cp && isspace(*cp)) cp++;
	if (!*cp) break;
	count++;
	while (*cp && !isspace(*cp)) cp++;
    }

    if (!count)
	return NULL;

    /* Now allocate the array of pointers to 0-terminated driver names */
    driverlist = (char **)xnfalloc((count + 1) * sizeof(char *));
    count = 0;
    cp = drivernames;
    while (*cp) {
	while (*cp && isspace(*cp)) cp++;
	if (!*cp) break;
	driverlist[count++] = cp;
	while (*cp && !isspace(*cp)) cp++;
	if (!*cp) break;
	*cp++ = 0;
    }
    driverlist[count] = NULL;

#ifdef XFree86LOADER
    {
        const char *subdirs[] = {NULL, NULL};
        static const char *patlist[] = {"(.*)_drv\\.so", "(.*)_drv\\.o", NULL};
        char **dlist, **clist, **dcp, **ccp;
	int size;

        subdirs[0] = dirname;

        /* Get module list */
        dlist = LoaderListDirs(subdirs, patlist);
        if (!dlist) {
            xfree(driverlist);
            return NULL;        /* No modules, no list */
        }

        clist = driverlist;

        /* The resulting list cannot be longer than the module list */
        for (dcp = dlist, count = 0;  *dcp++;  count++);
        driverlist = (char **)xnfalloc((size = count + 1) * sizeof(char *));

        /* First, add modules not in compiled-in list */
        for (count = 0, dcp = dlist;  *dcp;  dcp++) {
            for (ccp = clist;  ;  ccp++) {
                if (!*ccp) {
                    driverlist[count++] = *dcp;
		    if (count >= size)
			driverlist = (char**)
			    xnfrealloc(driverlist, ++size * sizeof(char*));
                    break;
                }
                if (!strcmp(*ccp, *dcp))
                    break;
            }
        }

        /* Next, add compiled-in names that are also modules */
        for (ccp = clist;  *ccp;  ccp++) {
            for (dcp = dlist;  *dcp;  dcp++) {
                if (!strcmp(*ccp, *dcp)) {
                    driverlist[count++] = *ccp;
		    if (count >= size)
			driverlist = (char**)
			    xnfrealloc(driverlist, ++size * sizeof(char*));
                    break;
                }
            }
        }

        driverlist[count] = NULL;
        xfree(clist);
        xfree(dlist);
    }
#endif /* XFree86LOADER */
    return (const char **)driverlist;
}


const char **
xf86DriverlistFromCompile(void)
{
    static const char **driverlist = NULL;
    static Bool generated = FALSE;

    /* This string is modified in-place */
    static char drivernames[] = DRIVERS;

    if (!generated) {
        generated = TRUE;
        driverlist = GenerateDriverlist("drivers", drivernames);
    }
    return driverlist;
}


const char **
xf86InputDriverlistFromCompile(void)
{
    static const char **driverlist = NULL;
    static Bool generated = FALSE;

    /* This string is modified in-place */
    static char drivernames[] = IDRIVERS;

    if (!generated) {
        generated = TRUE;
	driverlist = GenerateDriverlist("input", drivernames);
    }
    return driverlist;
}

confFilesPtr
xf86ConfAllocFiles()
{
    confFilesPtr pFiles;

    pFiles = xcalloc(1, sizeof(*pFiles));
    pFiles->fontPathFrom = X_NONE;
    pFiles->rgbPathFrom = X_NONE;
    pFiles->modulePathFrom = X_NONE;
    pFiles->logFileFrom = X_NONE;
    pFiles->inputDeviceListFrom = X_NONE;
    pFiles->handle = pFiles;
    return pFiles;
}

void
xf86ConfFreeFilesData(confFilesPtr pFiles)
{
    if (!pFiles)
	return;

    if (pFiles->identifier) {
	xfree(pFiles->identifier);
	pFiles->identifier = NULL;
    }
    if (pFiles->logFile) {
	xfree(pFiles->logFile);
	pFiles->logFile = NULL;
    }
    if (pFiles->rgbPath) {
	xfree(pFiles->rgbPath);
	pFiles->rgbPath = NULL;
    }
    if (pFiles->modulePath) {
	xfree(pFiles->modulePath);
	pFiles->modulePath = NULL;
    }
    if (pFiles->inputDeviceList) {
	xfree(pFiles->inputDeviceList);
	pFiles->inputDeviceList = NULL;
    }
    if (pFiles->fontPath) {
	xfree(pFiles->fontPath);
	pFiles->fontPath = NULL;
    }
    if (pFiles->options) {
	xf86OptionListFree(pFiles->options);
	pFiles->options = NULL;
    }
}

confFilesPtr
xf86ConfDupFiles(const confFilesRec *pFiles)
{
    confFilesPtr f;

    if (!pFiles)
	return NULL;

    f = xf86ConfAllocFiles();
    if (!f)
	return NULL;

    f->handle = pFiles->handle;
    f->identifier = xstrdup(pFiles->identifier);
    if (pFiles->identifier && !f->identifier) {
	xf86ConfFreeFilesData(f);
	xfree(f);
	return NULL;
    }
    f->logFile = xstrdup(pFiles->logFile);
    if (pFiles->logFile && !f->logFile) {
	xf86ConfFreeFilesData(f);
	xfree(f);
	return NULL;
    }
    f->rgbPath = xstrdup(pFiles->rgbPath);
    if (pFiles->rgbPath && !f->rgbPath) {
	xf86ConfFreeFilesData(f);
	xfree(f);
	return NULL;
    }
    f->modulePath = xstrdup(pFiles->modulePath);
    if (pFiles->modulePath && !f->modulePath) {
	xf86ConfFreeFilesData(f);
	xfree(f);
	return NULL;
    }
    f->inputDeviceList = xstrdup(pFiles->inputDeviceList);
    if (pFiles->inputDeviceList && !f->inputDeviceList) {
	xf86ConfFreeFilesData(f);
	xfree(f);
	return NULL;
    }
    f->fontPath = xstrdup(pFiles->fontPath);
    if (pFiles->fontPath && !f->fontPath) {
	xf86ConfFreeFilesData(f);
	xfree(f);
	return NULL;
    }
    f->options = xf86OptionListDup(pFiles->options);
    if (pFiles->options && !f->options) {
	xf86ConfFreeFilesData(f);
	xfree(f);
	return NULL;
    }
    return f;
}

/*
 * Convert a Files section from the parser's format to the server's format.
 */
static confFilesPtr
convertFiles(confFilesPtr pFiles, const XF86ConfFilesRec *filesConf)
{
    if (!pFiles || !filesConf)
	return NULL;

    pFiles->handle = filesConf;
    pFiles->identifier = xstrdup(filesConf->file_identifier);
    if (filesConf->file_identifier && !pFiles->identifier) {
	xf86ConfFreeFilesData(pFiles);
	return NULL;
    }
    pFiles->logFile = xstrdup(filesConf->file_logfile);
    if (filesConf->file_logfile && !pFiles->logFile) {
	xf86ConfFreeFilesData(pFiles);
	return NULL;
    }
    pFiles->rgbPath = xstrdup(filesConf->file_rgbpath);
    if (filesConf->file_rgbpath && !pFiles->rgbPath) {
	xf86ConfFreeFilesData(pFiles);
	return NULL;
    }
    pFiles->modulePath = xstrdup(filesConf->file_modulepath);
    if (filesConf->file_modulepath && !pFiles->modulePath) {
	xf86ConfFreeFilesData(pFiles);
	return NULL;
    }
    pFiles->inputDeviceList = xstrdup(filesConf->file_inputdevs);
    if (filesConf->file_inputdevs && !pFiles->inputDeviceList) {
	xf86ConfFreeFilesData(pFiles);
	return NULL;
    }
    pFiles->fontPath = xstrdup(filesConf->file_fontpath);
    if (filesConf->file_fontpath && !pFiles->fontPath) {
	xf86ConfFreeFilesData(pFiles);
	return NULL;
    }
    pFiles->options = xf86OptionListDup(filesConf->file_option_lst);
    if (filesConf->file_option_lst && !pFiles->options) {
	xf86ConfFreeFilesData(pFiles);
	return NULL;
    }
    return pFiles;
}

confFilesPtr
xf86ConfGetFilesByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    XF86ConfFilesPtr f;
    confFilesPtr pFiles = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (f = pConfig->conf_files_lst; f; f = f->list.next) {
	if (xf86nameCompare(f->file_identifier, name) == 0)
	    break;
    }
    if (f) {
	pFiles = xf86ConfAllocFiles();
	if (pFiles) {
	    if (convertFiles(pFiles, f))
		return pFiles;
	    else
		xfree(pFiles);
	}
    }
    return NULL;
}

confFilesPtr
xf86ConfGetNextFiles(ConfigHandle handle, ConfigDataHandle prevFilesHandle)
{
    const XF86ConfigRec *pConfig;
    XF86ConfFilesPtr f;
    confFilesPtr pFiles = NULL;

    /* Check for these special cases first. */
    if (handle == CMDLINE_CONFIG || handle == BUILDTIME_CONFIG ||
	handle == ACTIVE_CONFIG) {
	if (prevFilesHandle)
	    return NULL;
	else {
	    const confFilesRec *fd;

	    if (handle == ACTIVE_CONFIG)
		return xf86ConfDupFiles(xf86FilePaths);
	    else if (handle == CMDLINE_CONFIG)
		fd = &xf86FileCmdline;
	    else
		fd = &xf86FileDefaults;
	    pFiles = xf86ConfCombineFilesData(fd, X_NONE, NULL, X_NONE);
	    if (!pFiles)
		return NULL;
	    return pFiles;
	}
    }

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevFilesHandle) {
	f = pConfig->conf_files_lst;
    } else {
	for (f = pConfig->conf_files_lst; f; f = f->list.next) {
	    if (f == prevFilesHandle) {
		f = f->list.next;
		break;
	    }
	}
    }
    if (f) {
	pFiles = xf86ConfAllocFiles();
	if (pFiles) {
	    if (convertFiles(pFiles, f))
		return pFiles;
	    else
		xfree(pFiles);
	}
    }
    return NULL;
}

confFilesPtr
xf86ConfCombineFilesData(const confFilesRec *src1, MessageType m1,
			 const confFilesRec *src2, MessageType m2)
{
    pointer opt1 = NULL, opt2 = NULL;
    confFilesPtr dst = NULL;

    dst = xf86ConfAllocFiles();
    if (!dst)
	return NULL;

    if (src1 && src1->logFile) {
	dst->logFile = xstrdup(src1->logFile);
	dst->logFileFrom = (m1 != X_NONE ? m1 : src1->logFileFrom);
    } else if (src2 && src2->logFile) {
	dst->logFile = xstrdup(src2->logFile);
	dst->logFileFrom = (m2 != X_NONE ? m2 : src2->logFileFrom);
    }
    if (!dst->logFile && ((src1 && src1->logFile) ||
			  (src2 && src2->logFile))) {
	xf86ConfFreeFilesData(dst);
	xfree(dst);
	return NULL;
    }

    if (src1 && src1->rgbPath) {
	dst->rgbPath = xstrdup(src1->rgbPath);
	dst->rgbPathFrom = (m1 != X_NONE ? m1 : src1->rgbPathFrom);
    } else if (src2 && src2->rgbPath) {
	dst->rgbPath = xstrdup(src2->rgbPath);
	dst->rgbPathFrom = (m2 != X_NONE ? m2 : src2->rgbPathFrom);
    }
    if (!dst->rgbPath && ((src1 && src1->rgbPath) ||
			  (src2 && src2->rgbPath))) {
	xf86ConfFreeFilesData(dst);
	xfree(dst);
	return NULL;
    }

    if (src1 && src1->modulePath) {
	dst->modulePath = xstrdup(src1->modulePath);
	dst->modulePathFrom = (m1 != X_NONE ? m1 : src1->modulePathFrom);
    } else if (src2 && src2->modulePath) {
	dst->modulePath = xstrdup(src2->modulePath);
	dst->modulePathFrom = (m2 != X_NONE ? m2 : src2->modulePathFrom);
    }
    if (!dst->modulePath && ((src1 && src1->modulePath) ||
			     (src2 && src2->modulePath))) {
	xf86ConfFreeFilesData(dst);
	xfree(dst);
	return NULL;
    }

    if (src1 && src1->fontPath) {
	dst->fontPath = xstrdup(src1->fontPath);
	dst->fontPathFrom = (m1 != X_NONE ? m1 : src1->fontPathFrom);
    } else if (src2 && src2->fontPath) {
	dst->fontPath = xstrdup(src2->fontPath);
	dst->fontPathFrom = (m2 != X_NONE ? m2 : src2->fontPathFrom);
    }
    if (!dst->fontPath && ((src1 && src1->fontPath) ||
			   (src2 && src2->fontPath))) {
	xf86ConfFreeFilesData(dst);
	xfree(dst);
	return NULL;
    }

    if (src1 && src1->inputDeviceList) {
	dst->inputDeviceList = xstrdup(src1->inputDeviceList);
	dst->inputDeviceListFrom =
		(m1 != X_NONE ? m1 : src1->inputDeviceListFrom);
    } else if (src2 && src2->inputDeviceList) {
	dst->inputDeviceList = xstrdup(src2->inputDeviceList);
	dst->inputDeviceListFrom =
		(m2 != X_NONE ? m2 : src2->inputDeviceListFrom);
    }
    if (!dst->inputDeviceList && ((src1 && src1->inputDeviceList) ||
				  (src2 && src2->inputDeviceList))) {
	xf86ConfFreeFilesData(dst);
	xfree(dst);
	return NULL;
    }

    if (src1)
	opt1 = xf86OptionListDup(src1->options);
    if (src2)
	opt2 = xf86OptionListDup(src2->options);
    dst->options = xf86OptionListMerge(opt2, opt1);

    dst->handle = dst;
    return dst;
}

/*
 * xf86GetPathElem --
 *	Extract a single element from the font path string starting at
 *	pnt.  The font path element will be returned, and pnt will be
 *	updated to point to the start of the next element, or set to
 *	NULL if there are no more.
 */
static char *
xf86GetPathElem(char **pnt)
{
    char *p1;

    p1 = *pnt;
    *pnt = strchr(*pnt, ',');
    if (*pnt) {
	**pnt = '\0';
	(*pnt)++;
    }
    return p1;
}

/*
 * xf86ValidateFontPath --
 *	Validates the user-specified font path.  Each element that
 *	begins with a '/' is checked to make sure the directory exists.
 *	If the directory exists, the existence of a file named 'fonts.dir'
 *	is checked.  If either check fails, an error is printed and the
 *	element is removed from the font path.
 */

#define DIR_FILE "/fonts.dir"
static char *
xf86ValidateFontPath(const char *p)
{
    char *tmp_path, *out_pnt, *path_elem, *next, *p1, *dir_elem, *path;
    struct stat stat_buf;
    Bool ok;
    int dirlen;

    path = xstrdup(p);
    if (!path)
	return NULL;
    tmp_path = xcalloc(1, strlen(path) + 1);
    if (!tmp_path)
	return NULL;
    out_pnt = tmp_path;
    path_elem = NULL;
    next = path;
    while (next) {
	path_elem = xf86GetPathElem(&next);
	if (*path_elem == '/') {
#ifndef __UNIXOS2__
	    dir_elem = xcalloc(1, strlen(path_elem) + 1);
	    if (!dir_elem) {
		xfree(tmp_path);
		return NULL;
	    }
	    if ((p1 = strchr(path_elem, ':')))
#else
	/* OS/2 must prepend X11ROOT */
	    path_elem = (char*)__XOS2RedirRoot(path_elem);
	    dir_elem = xcalloc(1, strlen(path_elem) + 1);
	    if (!dir_elem) {
		xfree(tmp_path);
		return NULL;
	    }
	    if ((p1 = strchr(path_elem + 2, ':')))
#endif
		dirlen = p1 - path_elem;
	    else
		dirlen = strlen(path_elem);
	    strncpy(dir_elem, path_elem, dirlen);
	    dir_elem[dirlen] = '\0';
	    ok = (stat(dir_elem, &stat_buf) == 0 && S_ISDIR(stat_buf.st_mode));
	    if (!ok) {
		xf86Msg(X_WARNING,
			"The directory \"%s\" does not exist.\n", dir_elem);
		xf86ErrorF("\tEntry deleted from font path.\n");
		xfree(dir_elem);
		continue;
	    } else {
		p1 = xalloc(strlen(dir_elem) + strlen(DIR_FILE) + 1);
		if (!p1) {
		    xfree(dir_elem);
		    xfree(tmp_path);
		    return NULL;
		}
		strcpy(p1, dir_elem);
		strcat(p1, DIR_FILE);
		ok = (stat(p1, &stat_buf) == 0 && S_ISREG(stat_buf.st_mode));
		xfree(p1);
		if (!ok) {
		    xf86Msg(X_WARNING,
			    "\"fonts.dir\" not found (or not valid) in "
			    "\"%s\".\n", dir_elem);
		    xf86ErrorF("\tEntry deleted from font path.\n");
		    xf86ErrorF("\t(Run 'mkfontdir' on \"%s\").\n", dir_elem);
		    xfree(dir_elem);
		    continue;
		}
	    }
	    xfree(dir_elem);
	}

	/*
	 * Either an OK directory, or a font server name.  So add it to
	 * the path.
	 */
	if (out_pnt != tmp_path)
	    *out_pnt++ = ',';
	strcat(out_pnt, path_elem);
	out_pnt += strlen(path_elem);
    }
    return tmp_path;
}

static confFilesPtr
configFiles(ConfigHandle handle, const XF86ConfFilesRec *filesConf,
	    MessageType from)
{
    confFilesPtr f = NULL, f1, f2;
    const void *fh;

    /*
     * If filesConf is supplied explicitly, use it.  Otherwise combine all
     * of the Files data.
     */
    if (filesConf) {
	f = xf86ConfAllocFiles();
	if (!f)
	    return NULL;
	if (!(f = convertFiles(f, filesConf))) {
	    xfree(f);
	    return NULL;
	}
    } else {
	f1 = xf86ConfGetNextFiles(handle, NULL);
	if (f1) {
	    fh = f1->handle;
	    do {
		f2 = xf86ConfGetNextFiles(handle, fh);
		if (f2) {
		    fh = f2->handle;
		    f = xf86ConfCombineFilesData(f1, from, f2, from);
		    xf86ConfFreeFilesData(f1);
		    xf86ConfFreeFilesData(f2);
		    xfree(f1);
		    xfree(f2);
		    if (!f)
			return NULL;
		    f1 = f;
		} else {
		    f = f1;
		    fh = NULL;
		}
	    } while (fh);
	}
    }
    /*
     * Combine the config file data with the defaults and command line
     * options.
     */
    f1 = f;
    f = xf86ConfCombineFilesData(&xf86FileCmdline, X_CMDLINE, f1, from);
    xf86ConfFreeFilesData(f1);
    xfree(f1);
    if (!f)
	return NULL;
    f1 = f;
    f = xf86ConfCombineFilesData(f1, X_NONE, &xf86FileDefaults, X_DEFAULT);
    xf86ConfFreeFilesData(f1);
    xfree(f1);
    if (!f)
	return NULL;
    f->identifier = xstrdup("<combined>");
    xf86ConfFreeFilesData(xf86FilePaths);
    xf86FilePaths = f;

    /* Validate/print path info. */
    {
	char *fp;

	fp = xf86ValidateFontPath(xf86FilePaths->fontPath);
	xfree(xf86FilePaths->fontPath);
	if (fp && *fp) {
	    xf86FilePaths->fontPath = defaultFontPath = fp;
	} else {
	    xf86Msg(X_WARNING, "FontPath is completely invalid.  "
			       "Using compiled-in default.\n");
	    xf86FilePaths->fontPath = xstrdup(defaultFontPath);
	    if (!xf86FilePaths->fontPath)
		return NULL;
	    xf86FilePaths->fontPathFrom = X_DEFAULT;
	}
    }

    /* Fatal error if no valid font path is found. */
    if (!defaultFontPath || !*defaultFontPath)
	FatalError("No valid FontPath could be found.");

    xf86Msg(xf86FilePaths->fontPathFrom, "FontPath set to \"%s\"\n",
	    xf86FilePaths->fontPath);

    rgbPath = xnfstrdup(xf86FilePaths->rgbPath);
    xf86Msg(xf86FilePaths->rgbPathFrom, "RgbPath set to \"%s\"\n",
	    xf86FilePaths->rgbPath);

    if (xf86FilePaths->inputDeviceList) {
	xf86Msg(xf86FilePaths->inputDeviceListFrom,
		"Input device list set to \"%s\"\n",
		xf86FilePaths->inputDeviceList);
    }
  
    xf86Msg(xf86FilePaths->modulePathFrom, "ModulePath set to \"%s\"\n",
	    xf86FilePaths->modulePath);

#if 0
    /* LogFile */
    /*
     * XXX The problem with this is that the log file is already open.
     * One option might be to copy the exiting contents to the new location.
     * and re-open it.  The down side is that the default location would
     * already have been overwritten.  Another option would be to start with
     * unique temporary location, then copy it once the correct name is known.
     * A problem with this is what happens if the server exits before that
     * happens.
     */
    xf86Msg(xf86FilePaths->logFileFrom, "Log file set to \"%s\"\n",
	    xf86FilePaths->logFile);
#endif
    return xf86FilePaths;
}

confFlagsPtr
xf86ConfAllocServerFlags()
{
    confFlagsPtr pFlags;

    pFlags = xcalloc(1, sizeof(*pFlags));
    pFlags->handle = pFlags;
    return pFlags;
}

void
xf86ConfFreeServerFlagsData(confFlagsPtr pFlags)
{
    if (!pFlags)
	return;

    if (pFlags->identifier) {
	xfree(pFlags->identifier);
	pFlags->identifier = NULL;
    }
    if (pFlags->options) {
	xf86OptionListFree(pFlags->options);
	pFlags->options = NULL;
    }
}

confFlagsPtr
xf86ConfDupServerFlags(const confFlagsRec *pFlags)
{
    confFlagsPtr f;

    if (!pFlags)
	return NULL;

    f = xf86ConfAllocServerFlags();
    if (!f)
	return NULL;

    f->handle = pFlags->handle;
    f->identifier = xstrdup(pFlags->identifier);
    if (pFlags->identifier && !f->identifier) {
	xf86ConfFreeServerFlagsData(f);
	xfree(f);
	return NULL;
    }
    f->options = xf86OptionListDup(pFlags->options);
    if (pFlags->options && !f->options) {
	xf86ConfFreeServerFlagsData(f);
	xfree(f);
	return NULL;
    }
    return f;
}

/*
 * Convert a Files section from the parser's format to the server's format.
 */
static confFlagsPtr
convertServerFlags(confFlagsPtr pFlags, const XF86ConfFlagsRec *flagsConf)
{
    if (!pFlags || !flagsConf)
	return NULL;

    pFlags->handle = flagsConf;
    pFlags->identifier = xstrdup(flagsConf->flg_identifier);
    if (flagsConf->flg_identifier && !pFlags->identifier) {
	xf86ConfFreeServerFlagsData(pFlags);
	return NULL;
    }
    pFlags->options = xf86OptionListDup(flagsConf->flg_option_lst);
    if (flagsConf->flg_option_lst && !pFlags->options) {
	xf86ConfFreeServerFlagsData(pFlags);
	return NULL;
    }
    return pFlags;
}

confFlagsPtr
xf86ConfGetServerFlagsByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    const XF86ConfFlagsRec *f;
    confFlagsPtr pFlags = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (f = pConfig->conf_flags_lst; f; f = f->list.next) {
	if (xf86nameCompare(f->flg_identifier, name) == 0)
	    break;
    }
    if (f) {
	pFlags = xf86ConfAllocServerFlags();
	if (pFlags) {
	    if (convertServerFlags(pFlags, f))
		return pFlags;
	    else
		xfree(pFlags);
	}
    }
    return NULL;
}

confFlagsPtr
xf86ConfGetNextServerFlags(ConfigHandle handle,
			   ConfigDataHandle prevFlagsHandle)
{
    const XF86ConfigRec *pConfig;
    const XF86ConfFlagsRec *f;
    confFlagsPtr pFlags = NULL;

    /* The active special case. */
    if (handle == ACTIVE_CONFIG) {
	if (!prevFlagsHandle)
	    return xf86ConfDupServerFlags(xf86Info.confFlags);
	else
	    return NULL;
    }

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevFlagsHandle) {
	f = pConfig->conf_flags_lst;
    } else {
	for (f = pConfig->conf_flags_lst; f; f = f->list.next) {
	    if (f == prevFlagsHandle) {
		f = f->list.next;
		break;
	    }
	}
    }
    if (f) {
	pFlags = xf86ConfAllocServerFlags();
	if (pFlags) {
	    if (convertServerFlags(pFlags, f))
		return pFlags;
	    else
		xfree(pFlags);
	}
    }
    return NULL;
}

confFlagsPtr
xf86ConfCombineServerFlagsData(const confFlagsRec *src1,
			       const confFlagsRec *src2)
{
    confFlagsPtr dst = NULL;
    pointer opt1 = NULL, opt2 = NULL;

    dst = xf86ConfAllocServerFlags();
    if (!dst)
	return NULL;

    if (src1)
	opt1 = xf86OptionListDup(src1->options);
    if (src2)
	opt2 = xf86OptionListDup(src2->options);
    dst->options = xf86OptionListMerge(opt2, opt1);
    return dst;
}

typedef enum {
    FLAG_NOTRAPSIGNALS,
    FLAG_DONTVTSWITCH,
    FLAG_DONTZAP,
    FLAG_DONTZOOM,
    FLAG_DISABLEVIDMODE,
    FLAG_ALLOWNONLOCAL,
    FLAG_DISABLEMODINDEV,
    FLAG_MODINDEVALLOWNONLOCAL,
    FLAG_ALLOWMOUSEOPENFAIL,
    FLAG_VTINIT,
    FLAG_VTSYSREQ,
    FLAG_XKBDISABLE,
    FLAG_PCIPROBE1,
    FLAG_PCIPROBE2,
    FLAG_PCIFORCECONFIG1,
    FLAG_PCIFORCECONFIG2,
    FLAG_PCIFORCENONE,
    FLAG_PCIOSCONFIG,
    FLAG_SAVER_BLANKTIME,
    FLAG_DPMS_STANDBYTIME,
    FLAG_DPMS_SUSPENDTIME,
    FLAG_DPMS_OFFTIME,
    FLAG_PIXMAP,
    FLAG_PC98,
    FLAG_ESTIMATE_SIZES_AGGRESSIVELY,
    FLAG_NOPM,
    FLAG_XINERAMA,
    FLAG_ALLOW_DEACTIVATE_GRABS,
    FLAG_ALLOW_CLOSEDOWN_GRABS,
    FLAG_LOG,
    FLAG_RENDER_COLORMAP_MODE,
    FLAG_HANDLE_SPECIAL_KEYS,
    FLAG_RANDR
} FlagValues;

static OptionInfoRec FlagOptions[] = {
  { FLAG_NOTRAPSIGNALS,		"NoTrapSignals",		OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_DONTVTSWITCH,		"DontVTSwitch",			OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_DONTZAP,		"DontZap",			OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_DONTZOOM,		"DontZoom",			OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_DISABLEVIDMODE,	"DisableVidModeExtension",	OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_ALLOWNONLOCAL,		"AllowNonLocalXvidtune",	OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_DISABLEMODINDEV,	"DisableModInDev",		OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_MODINDEVALLOWNONLOCAL,	"AllowNonLocalModInDev",	OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_ALLOWMOUSEOPENFAIL,	"AllowMouseOpenFail",		OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_VTINIT,		"VTInit",			OPTV_STRING,
	{0}, FALSE },
  { FLAG_VTSYSREQ,		"VTSysReq",			OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_XKBDISABLE,		"XkbDisable",			OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_PCIPROBE1,		"PciProbe1"		,	OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_PCIPROBE2,		"PciProbe2",			OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_PCIFORCECONFIG1,	"PciForceConfig1",		OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_PCIFORCECONFIG2,	"PciForceConfig2",		OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_PCIFORCENONE,		"PciForceNone",			OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_PCIOSCONFIG,	        "PciOsConfig",   		OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_SAVER_BLANKTIME,	"BlankTime"		,	OPTV_INTEGER,
	{0}, FALSE },
  { FLAG_DPMS_STANDBYTIME,	"StandbyTime",			OPTV_INTEGER,
	{0}, FALSE },
  { FLAG_DPMS_SUSPENDTIME,	"SuspendTime",			OPTV_INTEGER,
	{0}, FALSE },
  { FLAG_DPMS_OFFTIME,		"OffTime",			OPTV_INTEGER,
	{0}, FALSE },
  { FLAG_PIXMAP,		"Pixmap",			OPTV_INTEGER,
	{0}, FALSE },
  { FLAG_PC98,			"PC98",				OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_ESTIMATE_SIZES_AGGRESSIVELY,"EstimateSizesAggressively",OPTV_INTEGER,
	{0}, FALSE },
  { FLAG_NOPM,			"NoPM",				OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_XINERAMA,		"Xinerama",			OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_ALLOW_DEACTIVATE_GRABS,"AllowDeactivateGrabs",		OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_ALLOW_CLOSEDOWN_GRABS, "AllowClosedownGrabs",		OPTV_BOOLEAN,
	{0}, FALSE },
  { FLAG_LOG,			"Log",				OPTV_STRING,
	{0}, FALSE },
  { FLAG_RENDER_COLORMAP_MODE,	"RenderColormapMode",		OPTV_STRING,
	{0}, FALSE },
  { FLAG_HANDLE_SPECIAL_KEYS,	"HandleSpecialKeys",		OPTV_STRING,
	{0}, FALSE },
  { FLAG_RANDR,			"RandR",			OPTV_BOOLEAN,
	{0}, FALSE },
  { -1,				NULL,				OPTV_NONE,
	{0}, FALSE },
};

#if defined(i386) || defined(__i386__)
static Bool
detectPC98(void)
{
#ifdef SUPPORT_PC98
    unsigned char buf[2];

    if (xf86ReadBIOS(0xf8000, 0xe80, buf, 2) != 2)
	return FALSE;
    if ((buf[0] == 0x98) && (buf[1] == 0x21))
	return TRUE;
    else
	return FALSE;
#else
    return FALSE;
#endif
}
#endif /* __i386__ */

static confFlagsPtr
configServerFlags(ConfigHandle handle, const XF86ConfFlagsPtr flagsConf,
		  XF86OptionPtr layoutOpts, MessageType from)
{
    confFlagsPtr f = NULL, f1, f2;
    const void *fh;
    pointer opt1, opt2;
    int i;
    Pix24Flags pix24 = Pix24DontCare;
    Bool value;

    /*
     * If flagsConf is supplied explicitly, use it.  Otherwise combine all of
     * the ServerFlags data.
     */
    if (flagsConf) {
	f = xf86ConfAllocServerFlags();
	if (!f)
	    return NULL;
	if (!(f = convertServerFlags(f, flagsConf))) {
	    xfree(f);
	    return NULL;
	}
    } else {
	f1 = xf86ConfGetNextServerFlags(handle, NULL);
	if (f1) {
	    fh = f1->handle;
	    do {
		f2 = xf86ConfGetNextServerFlags(handle, fh);
		if (f2) {
		    fh = f2->handle;
		    f = xf86ConfCombineServerFlagsData(f1, f2);
		    xf86ConfFreeServerFlagsData(f1);
		    xf86ConfFreeServerFlagsData(f2);
		    xfree(f1);
		    xfree(f2);
		    if (!f)
			return NULL;
		    f1 = f;
		} else {
		    f = f1;
		    fh = NULL;
		}
	    } while (fh);
	}
    }

    if (!f)
	f = xf86ConfAllocServerFlags();
    if (!f)
	return NULL;

    /*
     * Merge the ServerLayout and ServerFlags options.  The former have
     * precedence over the latter.
     */
    if (layoutOpts) {
	opt1 = xf86OptionListDup(layoutOpts);
	opt2 = xf86OptionListDup(f->options);
	f->options = xf86OptionListMerge(opt2, opt1);
    }

    xf86ProcessOptions(-1, f->options, FlagOptions);

    xf86GetOptValBool(FlagOptions, FLAG_NOTRAPSIGNALS, &xf86Info.notrapSignals);
    xf86GetOptValBool(FlagOptions, FLAG_DONTVTSWITCH, &xf86Info.dontVTSwitch);
    xf86GetOptValBool(FlagOptions, FLAG_DONTZAP, &xf86Info.dontZap);
    xf86GetOptValBool(FlagOptions, FLAG_DONTZOOM, &xf86Info.dontZoom);

    xf86GetOptValBool(FlagOptions, FLAG_ALLOW_DEACTIVATE_GRABS,
		      &(xf86Info.grabInfo.allowDeactivate));
    xf86GetOptValBool(FlagOptions, FLAG_ALLOW_CLOSEDOWN_GRABS,
		      &(xf86Info.grabInfo.allowClosedown));

    /*
     * Set things up based on the config file information.  Some of these
     * settings may be overridden later when the command line options are
     * checked.
     */
#ifdef XF86VIDMODE
    if (xf86GetOptValBool(FlagOptions, FLAG_DISABLEVIDMODE, &value))
	xf86Info.vidModeEnabled = !value;
    if (xf86GetOptValBool(FlagOptions, FLAG_ALLOWNONLOCAL, &value))
	xf86Info.vidModeAllowNonLocal = value;
#endif

#ifdef XF86MISC
    if (xf86GetOptValBool(FlagOptions, FLAG_DISABLEMODINDEV, &value))
	xf86Info.miscModInDevEnabled = !value;
    if (xf86GetOptValBool(FlagOptions, FLAG_MODINDEVALLOWNONLOCAL, &value))
	xf86Info.miscModInDevAllowNonLocal = value;
#endif

    if (xf86GetOptValBool(FlagOptions, FLAG_ALLOWMOUSEOPENFAIL, &value))
	xf86Info.allowMouseOpenFail = value;

    if (xf86GetOptValBool(FlagOptions, FLAG_VTSYSREQ, &value)) {
#ifdef USE_VT_SYSREQ
	xf86Info.vtSysreq = value;
	xf86Msg(X_CONFIG, "VTSysReq %s\n", value ? "enabled" : "disabled");
#else
	if (value)
	    xf86Msg(X_WARNING, "VTSysReq is not supported on this OS.\n");
#endif
    }

    if (xf86GetOptValBool(FlagOptions, FLAG_XKBDISABLE, &value)) {
#ifdef XKB
	noXkbExtension = value;
	xf86Msg(X_CONFIG, "Xkb %s\n", value ? "disabled" : "enabled");
#else
	if (!value)
	    xf86Msg(X_WARNING, "Xserver doesn't support XKB.\n");
#endif
    }

    xf86Info.vtinit = xf86GetOptValString(FlagOptions, FLAG_VTINIT);

    if (xf86IsOptionSet(FlagOptions, FLAG_PCIPROBE1))
	xf86Info.pciFlags = PCIProbe1;
    if (xf86IsOptionSet(FlagOptions, FLAG_PCIPROBE2))
	xf86Info.pciFlags = PCIProbe2;
    if (xf86IsOptionSet(FlagOptions, FLAG_PCIFORCECONFIG1))
	xf86Info.pciFlags = PCIForceConfig1;
    if (xf86IsOptionSet(FlagOptions, FLAG_PCIFORCECONFIG2))
	xf86Info.pciFlags = PCIForceConfig2;
    if (xf86IsOptionSet(FlagOptions, FLAG_PCIOSCONFIG))
	xf86Info.pciFlags = PCIOsConfig;
    if (xf86IsOptionSet(FlagOptions, FLAG_PCIFORCENONE))
	xf86Info.pciFlags = PCIForceNone;

    xf86Info.pmFlag = TRUE;
    if (xf86GetOptValBool(FlagOptions, FLAG_NOPM, &value)) 
	xf86Info.pmFlag = !value;
    {
	const char *s;
	if ((s = xf86GetOptValString(FlagOptions, FLAG_LOG))) {
	    if (!xf86NameCmp(s,"flush")) {
		xf86Msg(X_CONFIG, "Flushing logfile enabled\n");
		LogSetParameter(XLOG_FLUSH, TRUE);
	    } else if (!xf86NameCmp(s,"sync")) {
		xf86Msg(X_CONFIG, "Syncing logfile enabled\n");
		LogSetParameter(XLOG_SYNC, TRUE);
	    } else {
		xf86Msg(X_WARNING,"Unknown Log option.\n");
	    }
        }
    }
    
#ifdef RENDER
    {
	const char *s;

	if ((s = xf86GetOptValString(FlagOptions, FLAG_RENDER_COLORMAP_MODE))){
	    int policy = PictureParseCmapPolicy (s);
	    if (policy == PictureCmapPolicyInvalid)
		xf86Msg(X_WARNING, "Unknown colormap policy \"%s\".\n", s);
	    else
	    {
		xf86Msg(X_CONFIG, "Render colormap policy set to %s\n", s);
		PictureCmapPolicy = policy;
	    }
	}
    }
#endif
    {
	const char *s;
	if ((s = xf86GetOptValString(FlagOptions, FLAG_HANDLE_SPECIAL_KEYS))) {
	    if (!xf86NameCmp(s,"always")) {
		xf86Msg(X_CONFIG, "Always handling special keys in DDX\n");
		xf86Info.ddxSpecialKeys = SKAlways;
	    } else if (!xf86NameCmp(s,"whenneeded")) {
		xf86Msg(X_CONFIG, "Special keys handled in DDX only if needed\n");
		xf86Info.ddxSpecialKeys = SKWhenNeeded;
	    } else if (!xf86NameCmp(s,"never")) {
		xf86Msg(X_CONFIG, "Never handling special keys in DDX\n");
		xf86Info.ddxSpecialKeys = SKNever;
	    } else {
		xf86Msg(X_WARNING,"Unknown HandleSpecialKeys option.\n");
	    }
        }
    }
#ifdef RANDR
    xf86Info.disableRandR = FALSE;
    xf86Info.randRFrom = X_DEFAULT;
    if (xf86GetOptValBool(FlagOptions, FLAG_RANDR, &value)) {
	xf86Info.disableRandR = !value;
	xf86Info.randRFrom = X_CONFIG;
    }
#endif
    i = -1;
    xf86GetOptValInteger(FlagOptions, FLAG_ESTIMATE_SIZES_AGGRESSIVELY, &i);
    if (i >= 0)
	xf86Info.estimateSizesAggressively = i;
    else
	xf86Info.estimateSizesAggressively = 0;
	
    i = -1;
    xf86GetOptValInteger(FlagOptions, FLAG_SAVER_BLANKTIME, &i);
    if (i >= 0)
	ScreenSaverTime = defaultScreenSaverTime = i * MILLI_PER_MIN;

#ifdef DPMSExtension
    i = -1;
    xf86GetOptValInteger(FlagOptions, FLAG_DPMS_STANDBYTIME, &i);
    if (i >= 0)
	DPMSStandbyTime = defaultDPMSStandbyTime = i * MILLI_PER_MIN;
    i = -1;
    xf86GetOptValInteger(FlagOptions, FLAG_DPMS_SUSPENDTIME, &i);
    if (i >= 0)
	DPMSSuspendTime = defaultDPMSSuspendTime = i * MILLI_PER_MIN;
    i = -1;
    xf86GetOptValInteger(FlagOptions, FLAG_DPMS_OFFTIME, &i);
    if (i >= 0)
	DPMSOffTime = defaultDPMSOffTime = i * MILLI_PER_MIN;
#endif

    i = -1;
    xf86GetOptValInteger(FlagOptions, FLAG_PIXMAP, &i);
    switch (i) {
    case 24:
	pix24 = Pix24Use24;
	break;
    case 32:
	pix24 = Pix24Use32;
	break;
    case -1:
	break;
    default:
	xf86ConfigError("Pixmap option's value (%d) must be 24 or 32\n", i);
	return FALSE;
    }
    if (xf86Pix24 != Pix24DontCare) {
	xf86Info.pixmap24 = xf86Pix24;
	xf86Info.pix24From = X_CMDLINE;
    } else if (pix24 != Pix24DontCare) {
	xf86Info.pixmap24 = pix24;
	xf86Info.pix24From = X_CONFIG;
    } else {
	xf86Info.pixmap24 = Pix24DontCare;
	xf86Info.pix24From = X_DEFAULT;
    }
#if defined(i386) || defined(__i386__)
    if (xf86GetOptValBool(FlagOptions, FLAG_PC98, &value)) {
	xf86Info.pc98 = value;
	if (value) {
	    xf86Msg(X_CONFIG, "Japanese PC98 architecture\n");
	}
    } else
	if (detectPC98()) {
	    xf86Info.pc98 = TRUE;
	    xf86Msg(X_PROBED, "Japanese PC98 architecture\n");
	}
#endif

#ifdef PANORAMIX
    from = X_DEFAULT;
    if (!noPanoramiXExtension)
      from = X_CMDLINE;
    else if (xf86GetOptValBool(FlagOptions, FLAG_XINERAMA, &value)) {
      noPanoramiXExtension = !value;
      from = X_CONFIG;
    }
    if (!noPanoramiXExtension)
      xf86Msg(from, "Xinerama: enabled.\n");
#endif

    xf86MsgVerb(X_INFO, 3, "Checking for unused ServerFlags options:\n");
    xf86ShowUnusedOptionsVerb(-1, f->options, 3);
    return f;
}

/*
 * XXX This function is temporary, and will be removed when the keyboard
 * driver is converted into a regular input driver.
 */
static Bool
configInputKbd(IDevPtr pIDev)
{
    char *s;
    MessageType from = X_DEFAULT;
    Bool customKeycodesDefault = FALSE;
    int verb = 0;

    /* Initialize defaults */
    xf86Info.xleds = 0L;
    xf86Info.kbdDelay = 500;
    xf86Info.kbdRate = 30;
    xf86Info.kbdProc = NULL;
    xf86Info.vtinit = NULL;
    xf86Info.vtSysreq = VT_SYSREQ_DEFAULT;
#if defined(SVR4) && defined(i386)
    xf86Info.panix106 = FALSE;
#endif
    xf86Info.kbdCustomKeycodes = FALSE;
#ifdef WSCONS_SUPPORT
    xf86Info.kbdFd = -1;
#endif
#ifdef XKB
    if (!xf86IsPc98()) {
	xf86Info.xkbrules = "xfree86";
	xf86Info.xkbmodel = "pc105";
	xf86Info.xkblayout = "us";
	xf86Info.xkbvariant = NULL;
	xf86Info.xkboptions = NULL;
    } else {
	xf86Info.xkbrules = "xfree98";
	xf86Info.xkbmodel = "pc98";
	xf86Info.xkblayout = "nec/jp";
	xf86Info.xkbvariant = NULL;
	xf86Info.xkboptions = NULL;
    }
    xf86Info.xkbcomponents_specified = FALSE;
    /* Should discourage the use of these. */
    xf86Info.xkbkeymap = NULL;
    xf86Info.xkbtypes = NULL;
    xf86Info.xkbcompat = NULL;
    xf86Info.xkbkeycodes = NULL;
    xf86Info.xkbsymbols = NULL;
    xf86Info.xkbgeometry = NULL;
#endif

    s = xf86SetStrOption(pIDev->commonOptions, "Protocol", "standard");
    if (xf86NameCmp(s, "standard") == 0) {
	xf86Info.kbdProc = xf86KbdProc;
	xf86Info.kbdEvents = xf86KbdEvents;
	xfree(s);
    } else if (xf86NameCmp(s, "xqueue") == 0) {
#ifdef XQUEUE
	xf86Info.kbdProc = xf86XqueKbdProc;
	xf86Info.kbdEvents = xf86XqueEvents;
	xf86Msg(X_CONFIG, "Xqueue selected for keyboard input.\n");
#endif
	xfree(s);
#ifdef WSCONS_SUPPORT
    } else if (xf86NameCmp(s, "wskbd") == 0) {
	xf86Info.kbdProc = xf86KbdProc;
	xf86Info.kbdEvents = xf86WSKbdEvents;
	xfree(s);
	s = xf86SetStrOption(pIDev->commonOptions, "Device", NULL);
	xf86Msg(X_CONFIG, "Keyboard: Protocol: wskbd\n");
	if (!s) {
	   xf86ConfigError("A \"device\" option is required with"
			   " the \"wskbd\" keyboard protocol.");
	   return FALSE;
	}
	xf86Info.kbdFd = open(s, O_RDWR | O_NONBLOCK | O_EXCL);
	if (xf86Info.kbdFd == -1) {
	    xf86ConfigError("cannot open \"%s\"", s);
	    xfree(s);
	    return FALSE;
	}
	xfree(s);
	/* Find out keyboard type. */
	if (ioctl(xf86Info.kbdFd, WSKBDIO_GTYPE, &xf86Info.wsKbdType) == -1) {
	    xf86ConfigError("Cannot get keyboard type.");
	    close(xf86Info.kbdFd);
	    return FALSE;
	}
	switch (xf86Info.wsKbdType) {
	case WSKBD_TYPE_PC_XT:
	    xf86Msg(X_PROBED, "Keyboard type: XT\n");
	    break;
	case WSKBD_TYPE_PC_AT:
	    xf86Msg(X_PROBED, "Keyboard type: AT\n");
	    break;
	case WSKBD_TYPE_USB:
	    xf86Msg(X_PROBED, "Keyboard type: USB\n");
	    break;
#ifdef WSKBD_TYPE_ADB
	case WSKBD_TYPE_ADB:
	    xf86Msg(X_PROBED, "Keyboard type: ADB\n");
	    break;
#endif
#ifdef WSKBD_TYPE_SUN
	case WSKBD_TYPE_SUN:
	    xf86Msg(X_PROBED, "Keyboard type: Sun\n");
	    break;
#endif
#ifdef WSKBD_TYPE_SUN5
	case WSKBD_TYPE_SUN5:
	    xf86Msg(X_PROBED, "Keyboard type: Sun5\n");
	    break;
#endif
	default:
	    xf86Msg(X_PROBED, "Keyboard type: unknown (%d)\n",
		xf86Info.wsKbdType);
	}
#endif
    } else {
	xf86ConfigError("\"%s\" is not a valid keyboard protocol name.", s);
	xfree(s);
	return FALSE;
    }

    s = xf86SetStrOption(pIDev->commonOptions, "AutoRepeat", NULL);
    if (s) {
	if (sscanf(s, "%d %d", &xf86Info.kbdDelay, &xf86Info.kbdRate) != 2) {
	    xf86ConfigError("\"%s\" is not a valid AutoRepeat value", s);
	    xfree(s);
	    return FALSE;
	}
	xfree(s);
    }

    s = xf86SetStrOption(pIDev->commonOptions, "XLeds", NULL);
    if (s) {
	char *l, *end;
	unsigned int i;
	l = strtok(s, " \t\n");
	while (l) {
	    i = strtoul(l, &end, 0);
	    if (*end == '\0')
		xf86Info.xleds |= 1L << (i - 1);
	    else {
		xf86ConfigError("\"%s\" is not a valid XLeds value", l);
		xfree(s);
		return FALSE;
	    }
	    l = strtok(NULL, " \t\n");
	}
	xfree(s);
    }

#ifdef XKB
    from = X_DEFAULT;
    if (noXkbExtension)
	from = X_CMDLINE;
    else if (xf86FindOption(pIDev->commonOptions, "XkbDisable")) {
	xf86Msg(X_WARNING, "KEYBOARD: XKB should be disabled in the "
	    "ServerFlags section instead\n"
	    "\tof in the \"keyboard\" InputDevice section.\n");
	noXkbExtension =
	xf86SetBoolOption(pIDev->commonOptions, "XkbDisable", FALSE);
	from = X_CONFIG;
    }
    if (noXkbExtension)
	xf86Msg(from, "XKB: disabled.\n");

#define NULL_IF_EMPTY(s) (s[0] ? s : (xfree(s), (char *)NULL))

    if (!noXkbExtension && !XkbInitialMap) {
	if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbKeymap", NULL))) {
	    xf86Info.xkbkeymap = NULL_IF_EMPTY(s);
	    xf86Msg(X_CONFIG, "XKB: keymap: \"%s\" "
		    "(overrides other XKB settings)\n", xf86Info.xkbkeymap);
	} else {
	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbCompat",
				      NULL))) {
		xf86Info.xkbcompat = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: compat: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbTypes",
				      NULL))) {
		xf86Info.xkbtypes = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: types: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbKeycodes",
				      NULL))) {
		xf86Info.xkbkeycodes = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: keycodes: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbGeometry",
				      NULL))) {
		xf86Info.xkbgeometry = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: geometry: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbSymbols",
				      NULL))) {
		xf86Info.xkbsymbols = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: symbols: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbRules",
				      NULL))) {
		xf86Info.xkbrules = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: rules: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbModel",
				      NULL))) {
		xf86Info.xkbmodel = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: model: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbLayout",
				      NULL))) {
		xf86Info.xkblayout = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: layout: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbVariant",
				      NULL))) {
		xf86Info.xkbvariant = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: variant: \"%s\"\n", s);
	    }

	    if ((s = xf86SetStrOption(pIDev->commonOptions, "XkbOptions",
				      NULL))) {
		xf86Info.xkboptions = NULL_IF_EMPTY(s);
		xf86Info.xkbcomponents_specified = TRUE;
		xf86Msg(X_CONFIG, "XKB: options: \"%s\"\n", s);
	    }
	}
    }
#undef NULL_IF_EMPTY
#endif
#if defined(SVR4) && defined(i386)
    if ((xf86Info.panix106 =
		xf86SetBoolOption(pIDev->commonOptions, "Panix106", FALSE))) {
	xf86Msg(X_CONFIG, "PANIX106: enabled\n");
    }
#endif

    /*
     * This was once a compile time option (ASSUME_CUSTOM_KEYCODES)
     * defaulting to 1 on Linux/PPC. It is no longer necessary, but for
     * backwards compatibility we provide 'Option "CustomKeycodes"'
     * and try to autoprobe on Linux/PPC.
     */
    from = X_DEFAULT;
    verb = 2;
#if defined(__linux__) && defined(__powerpc__)
    {
	FILE *f;

	f = fopen("/proc/sys/dev/mac_hid/keyboard_sends_linux_keycodes","r");
	if (f) {
	    if (fgetc(f) == '0') {
		customKeycodesDefault = TRUE;
		from = X_PROBED;
		verb = 1;
	    }
	    fclose(f);
	}
    }
#endif
    if (xf86FindOption(pIDev->commonOptions, "CustomKeycodes")) {
	from = X_CONFIG;
	verb = 1;
    }
    xf86Info.kbdCustomKeycodes =
	xf86SetBoolOption(pIDev->commonOptions, "CustomKeycodes",
			  customKeycodesDefault);
    xf86MsgVerb(from, verb, "Keyboard: CustomKeycode %s.\n",
		xf86Info.kbdCustomKeycodes ? "enabled" : "disabled");
    return TRUE;
}

/*
 * Locate the core input devices.  These can be specified/located in
 * the following ways, in order of priority:
 *
 *  1. The InputDevices named by the -pointer and -keyboard command line
 *     options.
 *  2. The "CorePointer" and "CoreKeyboard" InputDevices referred to by
 *     the active ServerLayout.
 *  3. The first InputDevices marked as "CorePointer" and "CoreKeyboard".
 *  4. The first InputDevices that use the 'mouse' and 'keyboard' or 'kbd'
 *     drivers.
 *  5. Default devices with an empty (default) configuration.  These defaults
 *     will reference the 'mouse' and 'keyboard' drivers.
 */

static Bool
checkCoreInputDevices(ConfigHandle handle, serverLayoutPtr pServerLayout,
		      Bool implicitLayout)
{
    int i, j, n;
    IDevPtr corePointer = NULL, coreKeyboard = NULL;
    Bool foundPointer = FALSE, foundKeyboard = FALSE;
    const char *pointerMsg = NULL, *keyboardMsg = NULL;
    IDevPtr indp, pIDev = NULL;
    XF86ConfInputRec defPtr, defKbd;
    pointer opt1 = NULL, opt2 = NULL;

    /*
     * First check if a core pointer or core keyboard have been specified
     * in the active ServerLayout.  If more than one is specified for either,
     * remove the core attribute from the later ones.
     */
    for (i = 0; i < pServerLayout->numInputs; i++) {
	indp = pServerLayout->inputDevs[i];
	if (!indp)
	    continue;
	opt1 = opt2 = NULL;
	if (indp->commonOptions &&
	    xf86CheckBoolOption(indp->commonOptions, "CorePointer", FALSE)) {
	    opt1 = indp->commonOptions;
	}
	if (indp->extraOptions &&
	    xf86CheckBoolOption(indp->extraOptions, "CorePointer", FALSE)) {
	    opt2 = indp->extraOptions;
	}
	if (opt1 || opt2) {
	    if (!corePointer) {
		corePointer = indp;
		corePointer->from = X_CONFIG;
	    } else {
		if (opt1)
		    xf86ReplaceBoolOption(opt1, "CorePointer", FALSE);
		if (opt2)
		    xf86ReplaceBoolOption(opt2, "CorePointer", FALSE);
		xf86Msg(X_WARNING, "Duplicate core pointer devices.  "
			"Removing core pointer attribute from \"%s\".\n",
			indp->identifier);
	    }
	}
	opt1 = opt2 = NULL;
	if (indp->commonOptions &&
	    xf86CheckBoolOption(indp->commonOptions, "CoreKeyboard", FALSE)) {
	    opt1 = indp->commonOptions;
	}
	if (indp->extraOptions &&
	    xf86CheckBoolOption(indp->extraOptions, "CoreKeyboard", FALSE)) {
	    opt2 = indp->extraOptions;
	}
	if (opt1 || opt2) {
	    if (!coreKeyboard) {
		coreKeyboard = indp;
		coreKeyboard->from = X_CONFIG;
	    } else {
		if (opt1)
		    xf86ReplaceBoolOption(opt1, "CoreKeyboard", FALSE);
		if (opt2)
		    xf86ReplaceBoolOption(opt2, "CoreKeyboard", FALSE);
		xf86Msg(X_WARNING, "Duplicate core keyboard devices.  "
			"Removing core keyboard attribute from \"%s\".\n",
			indp->identifier);
	    }
	}
    }

    /* 1. Check for the -pointer command line option. */
    if (xf86PointerName) {
	pIDev = xf86ConfGetInputDeviceByName(handle, xf86PointerName);
	if (!pIDev) {
	    xf86Msg(X_ERROR, "No InputDevice section called \"%s\".\n",
		    xf86PointerName);
	    return FALSE;
	}
	pIDev->from = X_CMDLINE;
	/*
	 * If one was already specified in the ServerLayout, it needs to be
	 * removed.
	 */
	if (corePointer) {
	    for (i = 0; i < pServerLayout->numInputs; i++) {
		indp = pServerLayout->inputDevs[i];
		if (indp == corePointer) {
		    xf86ConfFreeInputDeviceData(indp);
		    pServerLayout->numInputs--;
		    for (j = i; j < pServerLayout->numInputs; j++) {
			pServerLayout->inputDevs[j] =
				pServerLayout->inputDevs[j + 1];
		    }
		    pServerLayout->inputDevs[j] = NULL;
		    break;
		}
	    }
	}
	corePointer = NULL;
	foundPointer = TRUE;
    }

    /* 2. ServerLayout-specified core pointer. */
    if (corePointer) {
	foundPointer = TRUE;
	corePointer->from = X_CONFIG;
    }

    /* 3. First core pointer device. */
    if (!foundPointer) {
	pIDev = xf86ConfGetInputDeviceByOption(handle, "CorePointer");
	if (pIDev) {
	    foundPointer = TRUE;
	    pIDev->from = X_DEFAULT;
	    pointerMsg = "first core pointer device";
	}
    }

    /*
     * 4. First pointer named CONF_IMPLICIT_POINTER or with 'mouse' as
     *    the driver.
     */
    if (!foundPointer) {
	pIDev = xf86ConfGetInputDeviceByName(handle, CONF_IMPLICIT_POINTER);
	if (!pIDev)
	    pIDev = xf86ConfGetInputDeviceByDriver(handle, "mouse");
	if (pIDev) {
	    foundPointer = TRUE;
	    pIDev->from = X_DEFAULT;
	    pointerMsg = "first mouse device";
	}
    }

    /* 5. Built-in default. */
    if (!foundPointer) {
	bzero(&defPtr, sizeof(defPtr));
	defPtr.inp_identifier = xstrdup("<default pointer>");
	defPtr.inp_driver = xstrdup("mouse");
	pIDev = xf86ConfAllocInputDevice();
	if (pIDev) {
	    if (!convertInputDevice(pIDev, &defPtr)) {
		xfree(pIDev);
		pIDev = NULL;
	    } else {
		foundPointer = TRUE;
		pIDev->from = X_DEFAULT;
		pointerMsg = "default mouse configuration";
	    }
	}
    }

    /* Add the core pointer device to the layout, and set it to Core. */
    if (foundPointer && pIDev) {
	IDevPtr *newDevs;
	char **newNames;
	pointer *newOpts;

	n = pServerLayout->numInputs++;
	newDevs = xrealloc(pServerLayout->inputDevs,
			   pServerLayout->numInputs * sizeof(*newDevs));
	if (!newDevs) {
	    xf86Msg(X_ERROR, "Allocation of new input device list failed.\n");
	    return FALSE;
	}
	pServerLayout->inputDevs = newDevs;
	newNames = xrealloc(pServerLayout->inputNames,
			    pServerLayout->numInputs * sizeof(newDevs));
	if (!newNames) {
	    xf86Msg(X_ERROR, "Allocation of new input device list failed.\n");
	    return FALSE;
	}
	pServerLayout->inputNames = newNames;
	newOpts = xrealloc(pServerLayout->inputExtraOptions,
			   pServerLayout->numInputs * sizeof(*newOpts));
	if (!newOpts) {
	    xf86Msg(X_ERROR, "Allocation of new input device list failed.\n");
	    return FALSE;
	}
	pServerLayout->inputExtraOptions = newOpts;
	pIDev->extraOptions = xf86addNewOption(NULL, "CorePointer", NULL);
	pServerLayout->inputDevs[n] = pIDev;
	pServerLayout->inputNames[n] = xstrdup(pIDev->identifier);
	pServerLayout->inputExtraOptions[n] =
		xf86OptionListDup(pIDev->extraOptions);
	pIDev = NULL;
    }

    if (!foundPointer) {
	/* This shouldn't happen. */
	xf86Msg(X_ERROR, "Cannot locate a core pointer device.\n");
	return FALSE;
    }

    /* 1. Check for the -keyboard command line option. */
    if (xf86KeyboardName) {
	pIDev = xf86ConfGetInputDeviceByName(handle, xf86KeyboardName);
	if (!pIDev) {
	    xf86Msg(X_ERROR, "No InputDevice section called \"%s\".\n",
		    xf86KeyboardName);
	    return FALSE;
	}
	pIDev->from = X_CMDLINE;
	/*
	 * If one was already specified in the ServerLayout, it needs to be
	 * removed.
	 */
	if (coreKeyboard) {
	    for (i = 0; i < pServerLayout->numInputs; i++) {
		indp = pServerLayout->inputDevs[i];
		if (indp == coreKeyboard) {
		    xf86ConfFreeInputDeviceData(indp);
		    pServerLayout->numInputs--;
		    for (j = i; j < pServerLayout->numInputs; j++) {
			pServerLayout->inputDevs[j] =
				pServerLayout->inputDevs[j + 1];
		    }
		    pServerLayout->inputDevs[j] = NULL;
		    break;
		}
	    }
	}
	coreKeyboard = NULL;
	foundKeyboard = TRUE;
    }

    /* 2. ServerLayout-specified core keyboard. */
    if (coreKeyboard) {
	foundKeyboard = TRUE;
	coreKeyboard->from = X_CONFIG;
    }

    /* 3. First core keyboard device. */
    if (!foundKeyboard) {
	pIDev = xf86ConfGetInputDeviceByOption(handle, "CoreKeyboard");
	if (pIDev) {
	    foundKeyboard = TRUE;
	    pIDev->from = X_DEFAULT;
	    keyboardMsg = "first core keyboard device";
	}
    }

    /*
     * 4. First keyboard named CONF_IMPLICIT_KEYBOARD, or with 'keyboard'
     *    or 'kbd' as the driver.
     */
    if (!foundKeyboard) {
	pIDev = xf86ConfGetInputDeviceByName(handle, CONF_IMPLICIT_KEYBOARD);
	if (!pIDev)
	    pIDev = xf86ConfGetInputDeviceByDriver(handle, "keyboard");
	if (!pIDev)
	    pIDev = xf86ConfGetInputDeviceByDriver(handle, "kbd");
	if (pIDev) {
	    foundKeyboard = TRUE;
	    pIDev->from = X_DEFAULT;
	    keyboardMsg = "first keyboard device";
	}
    }

    /* 5. Built-in default. */
    if (!foundKeyboard) {
	bzero(&defKbd, sizeof(defKbd));
	defKbd.inp_identifier = xstrdup("<default keyboard>");
	defKbd.inp_driver = xstrdup("keyboard");
	pIDev = xf86ConfAllocInputDevice();
	if (pIDev) {
	    if (!convertInputDevice(pIDev, &defKbd)) {
		xfree(pIDev);
		pIDev = NULL;
	    } else {
		foundKeyboard = TRUE;
		pIDev->from = X_DEFAULT;
		keyboardMsg = "default keyboard configuration";
	    }
	}
    }

    /* Add the core keyboard device to the layout, and set it to Core. */
    if (foundKeyboard && pIDev) {
	if (!xf86NameCmp(pIDev->driver, "keyboard"))
	    foundKeyboard = configInputKbd(pIDev);
	if (foundKeyboard) {
	    IDevPtr *newDevs;
	    char **newNames;
	    pointer *newOpts;

	    n = pServerLayout->numInputs++;
	    newDevs = xrealloc(pServerLayout->inputDevs,
			       pServerLayout->numInputs * sizeof(*newDevs));
	    if (!newDevs) {
		xf86Msg(X_ERROR,
			"Allocation of new input device list failed.\n");
		return FALSE;
	    }
	    pServerLayout->inputDevs = newDevs;
	    newNames = xrealloc(pServerLayout->inputNames,
				pServerLayout->numInputs * sizeof(newDevs));
	    if (!newNames) {
		xf86Msg(X_ERROR,
			"Allocation of new input device list failed.\n");
		return FALSE;
	    }
	    pServerLayout->inputNames = newNames;
	    newOpts = xrealloc(pServerLayout->inputExtraOptions,
			       pServerLayout->numInputs * sizeof(*newOpts));
	    if (!newOpts) {
		xf86Msg(X_ERROR,
			"Allocation of new input device list failed.\n");
		return FALSE;
	    }
	    pServerLayout->inputExtraOptions = newOpts;
	    pIDev->extraOptions = xf86addNewOption(NULL, "CoreKeyboard", NULL);
	    pServerLayout->inputDevs[n] = pIDev;
	    pServerLayout->inputNames[n] = xstrdup(pIDev->identifier);
	    pServerLayout->inputExtraOptions[n] =
		    xf86OptionListDup(pIDev->extraOptions);
	    pIDev = NULL;
	}
    }

    if (!foundKeyboard) {
	/* This shouldn't happen. */
	xf86Msg(X_ERROR, "Cannot locate a core keyboard device.\n");
	return FALSE;
    }

    if (pointerMsg) {
	xf86Msg(X_WARNING, "The core pointer device wasn't specified "
		"explicitly in the layout.\n"
		"\tUsing the %s.\n", pointerMsg);
    }

    if (keyboardMsg) {
	xf86Msg(X_WARNING, "The core keyboard device wasn't specified "
		"explicitly in the layout.\n"
		"\tUsing the %s.\n", keyboardMsg);
    }
    return TRUE;
}

screenLayoutPtr
xf86ConfAllocScreenLayout()
{
    screenLayoutPtr pScreenLayout;

    pScreenLayout = xcalloc(1, sizeof(*pScreenLayout));
    pScreenLayout->where = PosAbsolute;
    pScreenLayout->x = -1;
    pScreenLayout->y = -1;
    return pScreenLayout;
}

void
xf86ConfFreeScreenLayoutData(screenLayoutPtr pScreenLayout)
{
    if (!pScreenLayout)
	return;

    if (pScreenLayout->screen) {
	xf86ConfFreeScreenData(pScreenLayout->screen);
	pScreenLayout->screen = NULL;
    }
    if (pScreenLayout->refname) {
	xfree(pScreenLayout->refname);
	pScreenLayout->refname = NULL;
    }
    if (pScreenLayout->screenName) {
	xfree(pScreenLayout->screenName);
	pScreenLayout->screenName = NULL;
    }
    pScreenLayout->refscreen = NULL;
}

screenLayoutPtr
xf86ConfDupScreenLayout(const screenLayoutRec *pScreenLayout, int depth)
{
    screenLayoutPtr s;

    if (!pScreenLayout)
	return NULL;

    s = xf86ConfAllocScreenLayout();
    if (!s)
	return NULL;

    s->screenName = xstrdup(pScreenLayout->screenName);
    if (pScreenLayout->screenName && !s->screenName) {
	xf86ConfFreeScreenLayoutData(s);
	xfree(s);
	return NULL;
    }
    s->screenNum = pScreenLayout->screenNum;
    s->x = pScreenLayout->x;
    s->y = pScreenLayout->y;
    s->refname = xstrdup(pScreenLayout->refname);
    if (pScreenLayout->refname && !s->refname) {
	xf86ConfFreeScreenLayoutData(s);
	xfree(s);
	return NULL;
    }
    s->where = pScreenLayout->where;
    if (depth > 0) {
	s->screen = xf86ConfDupScreen(pScreenLayout->screen, depth - 1);
	s->refscreen = pScreenLayout->refscreen;
    }
    return s;
}

static screenLayoutPtr
convertScreenLayout(screenLayoutPtr pScreenLayout,
		    const XF86ConfAdjacencyRec *adjacency)
{
    if (!pScreenLayout)
	return NULL;

    pScreenLayout->screenName = xstrdup(adjacency->adj_screen_str);
    if (adjacency->adj_screen_str && !pScreenLayout->screenName) {
	xf86ConfFreeScreenLayoutData(pScreenLayout);
	return NULL;
    }
    pScreenLayout->screenNum = adjacency->adj_scrnum;
    pScreenLayout->x = adjacency->adj_x;
    pScreenLayout->y = adjacency->adj_y;
    pScreenLayout->refname = xstrdup(adjacency->adj_refscreen);
    if (adjacency->adj_refscreen && !pScreenLayout->refname) {
	xf86ConfFreeScreenLayoutData(pScreenLayout);
	return NULL;
    }
    switch (adjacency->adj_where) {
    case CONF_ADJ_ABSOLUTE:
	pScreenLayout->where = PosAbsolute;
	break;
    case CONF_ADJ_RIGHTOF:
	pScreenLayout->where = PosRightOf;
	break;
    case CONF_ADJ_LEFTOF:
	pScreenLayout->where = PosLeftOf;
	break;
    case CONF_ADJ_ABOVE:
	pScreenLayout->where = PosAbove;
	break;
    case CONF_ADJ_BELOW:
	pScreenLayout->where = PosBelow;
	break;
    case CONF_ADJ_RELATIVE:
	pScreenLayout->where = PosRelative;
	break;
    case CONF_ADJ_OBSOLETE:
	xf86Msg(X_ERROR,
		"Obsolete screen layout encountered for screen \"%s\".\n"
		"\tThis is no longer supported.  "
		"Default positioning will be used.\n",
		pScreenLayout->screenName);
	pScreenLayout->where = PosAbsolute;
	pScreenLayout->x = -1;
	pScreenLayout->y = -1;
	break;
    }
    return pScreenLayout;
}

serverLayoutPtr
xf86ConfAllocServerLayout()
{
    serverLayoutPtr pServerLayout;

    pServerLayout = xcalloc(1, sizeof(*pServerLayout));
    pServerLayout->handle = pServerLayout;
    pServerLayout->from = X_NONE;
    return pServerLayout;
}

void
xf86ConfFreeServerLayoutData(serverLayoutPtr pServerLayout)
{
    int i;

    if (!pServerLayout)
	return;

    if (pServerLayout->screenLayouts) {
	for (i = 0; i < pServerLayout->numScreens; i++)  {
	    xf86ConfFreeScreenLayoutData(pServerLayout->screenLayouts[i]);
	    xfree(pServerLayout->screenLayouts[i]);
	    pServerLayout->screenLayouts[i] = NULL;
	}
	xfree(pServerLayout->screenLayouts);
	pServerLayout->screenLayouts = NULL;
    }
    pServerLayout->numScreens = 0;
    if (pServerLayout->inactiveNames) {
	for (i = 0; i < pServerLayout->numInactives; i++) {
	    xfree(pServerLayout->inactiveNames[i]);
	    pServerLayout->inactiveNames[i] = NULL;
	}
	xfree(pServerLayout->inactiveNames);
	pServerLayout->inactiveNames = NULL;
    }
    if (pServerLayout->inactiveDevs) {
	for (i = 0; i < pServerLayout->numInactives; i++) {
	    xf86ConfFreeGraphicsDeviceData(pServerLayout->inactiveDevs[i]);
	    xfree(pServerLayout->inactiveDevs[i]);
	    pServerLayout->inactiveDevs[i] = NULL;
	}
	xfree(pServerLayout->inactiveDevs);
	pServerLayout->inactiveDevs = NULL;
    }
    pServerLayout->numInactives = 0;
    if (pServerLayout->inputNames) {
	for (i = 0; i < pServerLayout->numInputs; i++) {
	    xfree(pServerLayout->inputNames[i]);
	    pServerLayout->inputNames[i] = NULL;
	}
	xfree(pServerLayout->inputNames);
	pServerLayout->inputNames = NULL;
    }
    if (pServerLayout->inputExtraOptions) {
	for (i = 0; i < pServerLayout->numInputs; i++) {
	    xf86OptionListFree(pServerLayout->inputExtraOptions[i]);
	    pServerLayout->inputExtraOptions[i] = NULL;
	}
	xfree(pServerLayout->inputExtraOptions);
	pServerLayout->inputExtraOptions = NULL;
    }
    if (pServerLayout->inputDevs) {
	for (i = 0; i < pServerLayout->numInputs; i++) {
	    xf86ConfFreeInputDeviceData(pServerLayout->inputDevs[i]);
	    xfree(pServerLayout->inputDevs[i]);
	    pServerLayout->inputDevs[i] = NULL;
	}
	xfree(pServerLayout->inputDevs);
	pServerLayout->inputDevs = NULL;
    }
    pServerLayout->numInputs = 0;
    if (pServerLayout->id) {
	xfree(pServerLayout->id);
	pServerLayout->id = NULL;
    }
    if (pServerLayout->options) {
	xf86OptionListFree(pServerLayout->options);
	pServerLayout->options = NULL;
    }
}

serverLayoutPtr
xf86ConfDupServerLayout(const serverLayoutRec *pServerLayout, int depth)
{
    serverLayoutPtr s;
    int i;

    if (!pServerLayout)
	return NULL;

    s = xf86ConfAllocServerLayout();
    if (!s)
	return NULL;

    s->handle = pServerLayout->handle;
    s->from = pServerLayout->from;
    s->numScreens = pServerLayout->numScreens;
    if (s->numScreens > 0) {
	s->screenLayouts = xcalloc(s->numScreens, sizeof(screenLayoutPtr));
	if (!s->screenLayouts) {
	    xfree(s);
	    return NULL;
	}
	for (i = 0; i < s->numScreens; i++) {
	    s->screenLayouts[i] =
		xf86ConfDupScreenLayout(pServerLayout->screenLayouts[i], depth);
	    if (!s->screenLayouts[i]) {
		xf86ConfFreeServerLayoutData(s);
		xfree(s);
		return NULL;
	    }
	}
    }
    s->numInactives = pServerLayout->numInactives;
    if (s->numInactives > 0) {
	s->inactiveNames = xf86DupStringArray(pServerLayout->inactiveNames,
					      pServerLayout->numInactives);
	if (pServerLayout->inactiveNames && !s->inactiveNames) {
	    xf86ConfFreeServerLayoutData(s);
	    xfree(s);
	    return NULL;
	}
	if (depth > 0) {
	    s->inactiveDevs = xcalloc(s->numInactives, sizeof(GDevPtr));
	    if (!s->inactiveDevs) {
		xf86ConfFreeServerLayoutData(s);
		xfree(s);
		return NULL;
	    }
	    for (i = 0; i < s->numInactives; i++) {
		s->inactiveDevs[i] =
		    xf86ConfDupGraphicsDevice(pServerLayout->inactiveDevs[i]);
		if (pServerLayout->inactiveDevs[i] && !s->inactiveDevs[i]) {
		    xf86ConfFreeServerLayoutData(s);
		    xfree(s);
		    return NULL;
		}
	    }
	}
    }

    s->numInputs = pServerLayout->numInputs;
    if (s->numInputs > 0) {
	s->inputNames = xf86DupStringArray(pServerLayout->inputNames,
					   pServerLayout->numInputs);
	if (pServerLayout->inputNames && !s->inputNames) {
	    xf86ConfFreeServerLayoutData(s);
	    xfree(s);
	    return NULL;
	}
	if (pServerLayout->inputExtraOptions) {
	    s->inputExtraOptions = xcalloc(s->numInputs, sizeof(pointer));
	    if (!s->inputExtraOptions) {
		xf86ConfFreeServerLayoutData(s);
		xfree(s);
		return NULL;
	    }
	    for (i = 0; i < s->numInputs; i++) {
		s->inputExtraOptions[i] =
		    xf86OptionListDup(pServerLayout->inputExtraOptions[i]);
		if (pServerLayout->inputExtraOptions[i] &&
		    !s->inputExtraOptions[i]) {
		    xf86ConfFreeServerLayoutData(s);
		    xfree(s);
		    return NULL;
		}
	    }
	}
	if (depth > 0) {
	    s->inputDevs = xcalloc(s->numInputs, sizeof(IDevPtr));
	    if (!s->inputDevs) {
		xf86ConfFreeServerLayoutData(s);
		xfree(s);
		return NULL;
	    }
	    for (i = 0; i < s->numInputs; i++) {
		s->inputDevs[i] =
		    xf86ConfDupInputDevice(pServerLayout->inputDevs[i], depth);
		if (pServerLayout->inputDevs[i] && !s->inputDevs[i]) {
		    xf86ConfFreeServerLayoutData(s);
		    xfree(s);
		    return NULL;
		}
	    }
	}
    }
    s->id = xstrdup(pServerLayout->id);
    if (pServerLayout->id && !s->id) {
	xf86ConfFreeServerLayoutData(s);
	xfree(s);
	return NULL;
    }
    s->options = xf86OptionListDup(pServerLayout->options);
    if (pServerLayout->options && !s->options) {
	xf86ConfFreeServerLayoutData(s);
	xfree(s);
	return NULL;
    }
    return s;
}

/*
 * Convert a ServerLayout section from the parser's format to the server's
 * format.
 */
static serverLayoutPtr
convertServerLayout(serverLayoutPtr pServerLayout,
		    const XF86ConfLayoutRec *confLayout)
{
    XF86ConfAdjacencyPtr adjp;
    XF86ConfInactivePtr idp;
    XF86ConfInputrefPtr irp;
    int count = 0;

    if (!pServerLayout || !confLayout)
	return NULL;

    pServerLayout->handle = confLayout;

    adjp = confLayout->lay_adjacency_lst;

    /*
     * We know that each screen is referenced exactly once on the left side
     * of a Screen statement in the ServerLayout section.  So to allocate
     * the right size for the array we do a quick walk of the list to figure
     * out how many screens we have.
     */
    while (adjp) {
	count++;
	adjp = adjp->list.next;
    }
    pServerLayout->numScreens = count;
    xf86MsgVerb(X_INFO, 5, "Found %d screen%s in the layout section \"%s\".\n",
		count, PLURAL(count), confLayout->lay_identifier);
    if (count) {
	pServerLayout->screenLayouts = xcalloc(count, sizeof(screenLayoutPtr));
	if (!pServerLayout->screenLayouts)
	    return NULL;
	adjp = confLayout->lay_adjacency_lst;
	count = 0;
	while (adjp) {
	    pServerLayout->screenLayouts[count] = xf86ConfAllocScreenLayout();
	    if (!pServerLayout->screenLayouts[count]) {
		xf86ConfFreeServerLayoutData(pServerLayout);
		return NULL;
	    }
	    if (!convertScreenLayout(pServerLayout->screenLayouts[count],
				     adjp)) {
		xf86ConfFreeServerLayoutData(pServerLayout);
		return NULL;
	    }
	    if (pServerLayout->screenLayouts[count]->screenNum < 0)
		pServerLayout->screenLayouts[count]->screenNum = count;
	    count++;
	    adjp = adjp->list.next;
	}
    }

    /*
     * Count the number of inactive devices.
     * XXX Do we still need these?
     */
    count = 0;
    idp = confLayout->lay_inactive_lst;
    while (idp) {
	count++;
	idp = idp->list.next;
    }
    pServerLayout->numInactives = count;
    xf86MsgVerb(X_INFO, 5,
		"Found %d inactive device%s in the layout section \"%s\".\n",
		count, PLURAL(count), confLayout->lay_identifier);
    if (count > 0) {
	pServerLayout->inactiveNames = xcalloc(count, sizeof(char *));
	if (!pServerLayout->inactiveNames) {
	    xf86ConfFreeServerLayoutData(pServerLayout);
	    return NULL;
	}
	count = 0;
	while (idp) {
	    pServerLayout->inactiveNames[count] =
		xstrdup(idp->inactive_device_str);
	    if (idp->inactive_device_str &&
		!pServerLayout->inactiveNames[count]) {
		xf86ConfFreeServerLayoutData(pServerLayout);
		return NULL;
	    }
	    count++;
	    idp = idp->list.next;
	}
    }

    /*
     * Count the number of input devices.
     */
    count = 0;
    irp = confLayout->lay_input_lst;
    while (irp) {
	count++;
	irp = irp->list.next;
    }
    pServerLayout->numInputs = count;
    xf86MsgVerb(X_INFO, 5,
		"Found %d input device%s in the layout section \"%s\".\n",
		count, PLURAL(count), confLayout->lay_identifier);
    if (count > 0) {
	pServerLayout->inputNames = xcalloc(count, sizeof(char *));
	if (!pServerLayout->inputNames) {
	    xf86ConfFreeServerLayoutData(pServerLayout);
	    return NULL;
	}
	pServerLayout->inputExtraOptions = xcalloc(count, sizeof(pointer));
	if (!pServerLayout->inputExtraOptions) {
	    xf86ConfFreeServerLayoutData(pServerLayout);
	    return NULL;
	}
	count = 0;
	irp = confLayout->lay_input_lst;
	while (irp) {
	    pServerLayout->inputNames[count] = xstrdup(irp->iref_inputdev_str);
	    if (irp->iref_inputdev_str && !pServerLayout->inputNames[count]) {
		xf86ConfFreeServerLayoutData(pServerLayout);
		return NULL;
	    }
	    pServerLayout->inputExtraOptions[count] =
		xf86OptionListDup(irp->iref_option_lst);
	    if (irp->iref_option_lst &&
		!pServerLayout->inputExtraOptions[count]) {
		xf86ConfFreeServerLayoutData(pServerLayout);
		return NULL;
	    }
	    count++;
	    irp = irp->list.next;
	}
    }
    pServerLayout->id = xstrdup(confLayout->lay_identifier);
    if (confLayout->lay_identifier && !pServerLayout->id) {
	xf86ConfFreeServerLayoutData(pServerLayout);
	return NULL;
    }
    pServerLayout->options = xf86OptionListDup(confLayout->lay_option_lst);
    if (confLayout->lay_option_lst && !pServerLayout->options) {
	xf86ConfFreeServerLayoutData(pServerLayout);
	return NULL;
    }
    return pServerLayout;
}

serverLayoutPtr
xf86ConfGetServerLayoutByName(ConfigHandle handle, const char *name, int depth)
{
    const XF86ConfigRec *pConfig;
    XF86ConfLayoutPtr l;
    serverLayoutPtr pServerLayout = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (l = pConfig->conf_layout_lst; l; l = l->list.next) {
	if (xf86nameCompare(l->lay_identifier, name) == 0) {
	    break;
	}
    }
    if (l) {
	pServerLayout = xf86ConfAllocServerLayout();
	if (pServerLayout) {
	    if (!convertServerLayout(pServerLayout, l)) {
		xfree(pServerLayout);
		return NULL;
	    }
	    if (depth > 0) {
		if (!xf86ConfResolveServerLayout(handle,
						 pServerLayout, depth)) {
		    xf86ConfFreeServerLayoutData(pServerLayout);
		    xfree(pServerLayout);
		    return NULL;
		}
	    }
	    return pServerLayout;
	}
    }
    return NULL;
}

serverLayoutPtr
xf86ConfGetNextServerLayout(ConfigHandle handle,
			    ConfigDataHandle prevLayoutHandle, int depth)
{
    const XF86ConfigRec *pConfig;
    XF86ConfLayoutPtr l;
    serverLayoutPtr pServerLayout = NULL;

    /* The special active case. */
    if (handle == ACTIVE_CONFIG) {
	if (!prevLayoutHandle)
	    return xf86ConfDupServerLayout(xf86Info.serverLayout, depth);
	else
	    return NULL;
    }

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevLayoutHandle) {
	l = pConfig->conf_layout_lst;
    } else {
	for (l = pConfig->conf_layout_lst; l; l = l->list.next) {
	    if (l == prevLayoutHandle) {
		l = l->list.next;
		break;
	    }
	}
    }
    if (l) {
	pServerLayout = xf86ConfAllocServerLayout();
	if (pServerLayout) {
	    if (!convertServerLayout(pServerLayout, l)) {
		xfree(pServerLayout);
		return NULL;
	    }
	    if (depth > 0) {
		if (!xf86ConfResolveServerLayout(handle,
						 pServerLayout, depth)) {
		    xf86ConfFreeServerLayoutData(pServerLayout);
		    xfree(pServerLayout);
		    return NULL;
		}
	    }
	    return pServerLayout;
	}
    }
    return NULL;
}

static void
resolveServerLayoutCleanup(serverLayoutPtr pServerLayout)
{
    int i;

    if (pServerLayout->screenLayouts) {
	for (i = 0; i < pServerLayout->numScreens; i++) {
	    if (pServerLayout->screenLayouts[i]) {
		xf86ConfFreeScreenData(pServerLayout->screenLayouts[i]->screen);
		pServerLayout->screenLayouts[i]->screen = NULL;
	    }
	}
    }
    if (pServerLayout->inactiveDevs) {
	for (i = 0; i < pServerLayout->numInactives; i++) {
	    xf86ConfFreeGraphicsDeviceData(pServerLayout->inactiveDevs[i]);
	    pServerLayout->inactiveDevs[i] = NULL;
	}
	xfree(pServerLayout->inactiveDevs);
    }
    if (pServerLayout->inputDevs) {
	for (i = 0; i < pServerLayout->numInputs; i++) {
	    xf86ConfFreeInputDeviceData(pServerLayout->inputDevs[i]);
	    pServerLayout->inputDevs[i] = NULL;
	}
	xfree(pServerLayout->inputDevs);
    }
}

serverLayoutPtr
xf86ConfResolveServerLayout(ConfigHandle handle, serverLayoutPtr pServerLayout,
			    int depth)
{
    screenLayoutPtr *ppScreenLayout;
    int i = 0, j;

    if (!pServerLayout || depth < 1)
	return pServerLayout;

    resolveServerLayoutCleanup(pServerLayout);

    if (!pServerLayout->screenLayouts)
	return pServerLayout;

    ppScreenLayout = pServerLayout->screenLayouts;
    for (i = 0; i < pServerLayout->numScreens; i++) {
	if (!ppScreenLayout[i]->screenName)
	    continue;
	ppScreenLayout[i]->screen =
	    xf86ConfGetScreenByName(handle, ppScreenLayout[i]->screenName,
				    depth - 1);
	if (!ppScreenLayout[i]->screen)
	    continue;
	ppScreenLayout[i]->screen->screennum = ppScreenLayout[i]->screenNum;
    }

    /* Fill in the refscreen value. */
    for (i = 0; i < pServerLayout->numScreens; i++) {
	for (j = 0; j < pServerLayout->numScreens; j++) {
	    if (ppScreenLayout[i]->refname &&
		strcmp(ppScreenLayout[i]->refname,
		       ppScreenLayout[j]->screenName) == 0) {
		ppScreenLayout[i]->refscreen = ppScreenLayout[j]->screen;
	    }
	}
	if (ppScreenLayout[i]->where != PosAbsolute &&
	    !ppScreenLayout[i]->refscreen) {
	    xf86Msg(X_WARNING,
		    "Screen \"%s\" doesn't exist: deleting placement.\n",
		    ppScreenLayout[i]->refname);
	    ppScreenLayout[i]->where = PosAbsolute;
	    ppScreenLayout[i]->x = -1;
	    ppScreenLayout[i]->y = -1;
	}
    }

    /* Inactive devices. */
    if (pServerLayout->numInactives > 0 && pServerLayout->inactiveNames) {
	pServerLayout->inactiveDevs =
	    xcalloc(pServerLayout->numInactives, sizeof(GDevPtr));
	if (!pServerLayout->inactiveDevs) {
	    resolveServerLayoutCleanup(pServerLayout);
	    return NULL;
	}
	for (i = 0; i < pServerLayout->numInactives; i++) {
	    if (!pServerLayout->inactiveNames[i])
		continue;
	    pServerLayout->inactiveDevs[i] =
		xf86ConfGetGraphicsDeviceByName(handle,
					    pServerLayout->inactiveNames[i]);
	}
    }

    /* Input devices. */
    if (pServerLayout->numInputs > 0 && pServerLayout->inputNames) {
	pServerLayout->inputDevs =
	    xcalloc(pServerLayout->numInputs, sizeof(IDevPtr));
	if (!pServerLayout->inputDevs) {
	    resolveServerLayoutCleanup(pServerLayout);
	    return NULL;
	}
	for (i = 0; i < pServerLayout->numInputs; i++) {
	    if (!pServerLayout->inputNames[i])
		continue;
	    pServerLayout->inputDevs[i] =
		xf86ConfGetInputDeviceByName(handle,
					     pServerLayout->inputNames[i]);
	    if (!pServerLayout->inputDevs[i])
		continue;
	    if (pServerLayout->inputExtraOptions) {
		pServerLayout->inputDevs[i]->extraOptions =
			xf86OptionListDup(pServerLayout->inputExtraOptions[i]);
		if (pServerLayout->inputExtraOptions[i] &&
		    !pServerLayout->inputDevs[i]->extraOptions) {
		    resolveServerLayoutCleanup(pServerLayout);
		    return NULL;
		}
	    }
	}
    }
    return pServerLayout;
}

int
xf86ConfCheckResolvedServerLayout(const serverLayoutRec *pServerLayout,
				  int depth, Bool strict)
{
    int i;
    Bool failures = 0;
    screenLayoutPtr *ppScreenLayout;

    if (!pServerLayout || depth < 1)
	return 0;

    ppScreenLayout = pServerLayout->screenLayouts;
    for (i = 0; i < pServerLayout->numScreens; i++) {
	if (!ppScreenLayout[i]->screenName)
	    continue;
	if (ppScreenLayout[i]->screen) {
	    failures +=
		xf86ConfCheckResolvedScreen(ppScreenLayout[i]->screen,
					    depth - 1, strict);
	} else {
	    xf86Msg(X_ERROR, "No Screen \"%s\" "
			     "(referenced by ServerLayout \"%s\").\n",
		    ppScreenLayout[i]->screenName, pServerLayout->id);
	    failures++;
	}
    }
    if (strict && pServerLayout->numScreens == 0) {
	xf86Msg(X_ERROR, "No Screen references in ServerLayout \"%s\".\n",
		pServerLayout->id);
	failures++;
    }

    if (pServerLayout->numInactives > 0 && pServerLayout->inactiveNames) {
	for (i = 0; i < pServerLayout->numInactives; i++) {
	    if (!pServerLayout->inactiveNames[i])
		continue;
	    if (!pServerLayout->inactiveDevs[i]) {
		xf86Msg(X_ERROR, "No Device \"%s\" "
				 "(referenced by ServerLayout \"%s\").\n",
			pServerLayout->inactiveNames[i], pServerLayout->id);
		failures++;
	    }
	}
    }
    if (pServerLayout->numInputs > 0 && pServerLayout->inputNames) {
	for (i = 0; i < pServerLayout->numInputs; i++) {
	    if (!pServerLayout->inputNames[i])
		continue;
	    if (!pServerLayout->inputDevs[i]) {
		xf86Msg(X_ERROR, "No Input Device \"%s\" "
				 "(referenced by ServerLayout \"%s\").\n",
			pServerLayout->inputNames[i], pServerLayout->id);
		failures++;
	    }
	}
    } else if (strict) {
	xf86Msg(X_ERROR, "No Input Device references in ServerLayout \"%s\".\n",
		pServerLayout->id);
	failures++;
    }
    return failures;
}

/*
 * Figure out which layout is active, and fill in the config data for
 * everything that it references.
 */
static serverLayoutPtr
configServerLayout(ConfigHandle handle, char *defaultLayout)
{
    int i;
    const char *name = NULL;
    serverLayoutPtr pServerLayout;

    /*
     * Find the active ServerLayout section.  The order of priority is:
     *
     *   1. Name specified by the -layout command line option.
     *   2. Name specified by the DefaultServerLayout config option.
     *   3. The first ServerLayout section.
     */
    if (xf86LayoutName)
	name = xf86LayoutName;
    else if (defaultLayout)
	name = defaultLayout;
    if (name) {
	pServerLayout = xf86ConfGetServerLayoutByName(handle, name,
						      CONFIG_RESOLVE_ALL);
	if (!pServerLayout) {
	    xf86Msg(X_ERROR, "No ServerLayout section called \"%s\".\n", name);
	    return NULL;
	}
	pServerLayout->from = xf86LayoutName ? X_CMDLINE : X_CONFIG;
    } else {
	pServerLayout = xf86ConfGetNextServerLayout(handle, NULL,
						    CONFIG_RESOLVE_ALL);
	if (!pServerLayout) {
	    xf86Msg(X_ERROR, "No ServerLayout sections found.\n");
	    return NULL;
	}
	pServerLayout->from = X_DEFAULT;
    }

    if (pServerLayout->numScreens <= 0) {
	xf86Msg(X_ERROR, "No Screens in ServerLayout \"%s\".\n",
		pServerLayout->id);
	xf86ConfFreeServerLayoutData(pServerLayout);
	xfree(pServerLayout);
	return NULL;
    }

    /* Special case handling for the built-in keyboard device. */
    if (pServerLayout->inputDevs) {
	for (i = 0; i < pServerLayout->numInputs; i++) {
	    IDevPtr pIDev = pServerLayout->inputDevs[i];
	    if (pIDev && !xf86NameCmp(pIDev->driver, "keyboard")) {
		if (!configInputKbd(pIDev)) {
		    xf86ConfFreeServerLayoutData(pServerLayout);
		    xfree(pServerLayout);
		    return NULL;
		}
	    }
	}
    }

    if (!checkCoreInputDevices(handle, pServerLayout, FALSE)) {
	xf86ConfFreeServerLayoutData(pServerLayout);
	xfree(pServerLayout);
	return NULL;
    }
    return pServerLayout;
}

/*
 * No ServerLayout section, so find the first Screen section and set that up as
 * the only active screen.
 */
static serverLayoutPtr
configImpliedServerLayout(ConfigHandle handle, const char *screenName)
{
    confScreenPtr s;
    screenLayoutPtr pScreenLayout;
    serverLayoutPtr pServerLayout;

    /*
     * Find the active Screen section in the absence of a ServerLayout
     * section, or when a Screen section is specified explicitly with
     * the '-screen' option.  The order of priority is:
     *
     *   1. Name specified by the -screen command line option.
     *   2. The first Screen section.
     */

    if (screenName) {
	s = xf86ConfGetScreenByName(handle, screenName, CONFIG_RESOLVE_ALL);
	if (!s) {
	    xf86Msg(X_ERROR, "No Screen section called \"%s\".\n", screenName);
	    return NULL;
	}
    } else {
	s = xf86ConfGetNextScreen(handle, NULL, CONFIG_RESOLVE_ALL);
	if (!s) {
	    xf86Msg(X_ERROR, "No Screen sections found.\n");
	    return NULL;
	}
    }

    pServerLayout = xf86ConfAllocServerLayout();
    if (!pServerLayout) {
	xf86ConfFreeScreenData(s);
	xfree(s);
	return NULL;
    }

    /* With an implied ServerLayout, there is exactly one screen. */
    pServerLayout->screenLayouts = xcalloc(1, sizeof(serverLayoutPtr));
    if (!pServerLayout->screenLayouts) {
	xf86ConfFreeScreenData(s);
	xfree(s);
	xfree(pServerLayout);
	return NULL;
    }
    pServerLayout->numScreens = 1;
    pScreenLayout = xf86ConfAllocScreenLayout();
    if (!pScreenLayout) {
	xf86ConfFreeScreenData(s);
	xfree(s);
	xf86ConfFreeServerLayoutData(pServerLayout);
	xfree(pServerLayout);
	return NULL;
    }
    pServerLayout->screenLayouts[0] = pScreenLayout;
    pScreenLayout->screen = s;
    pScreenLayout->screenName = xstrdup(s->id);
    if (s->id && !pScreenLayout->screenName) {
	xf86ConfFreeServerLayoutData(pServerLayout);
	xfree(pServerLayout);
	return NULL;
    }
    pServerLayout->id = xstrdup("(implicit)");
    if (!pServerLayout->id) {
	xf86ConfFreeServerLayoutData(pServerLayout);
	xfree(pServerLayout);
	return NULL;
    }
    pServerLayout->from = X_DEFAULT;
    /* Find implied core devices. */
    if (!checkCoreInputDevices(handle, pServerLayout, TRUE)) {
	xf86ConfFreeServerLayoutData(pServerLayout);
	xfree(pServerLayout);
	return NULL;
    }
    return pServerLayout;
}

confXvPortPtr
xf86ConfAllocXvPort()
{
    confXvPortPtr pPort;

    pPort = xcalloc(1, sizeof(*pPort));
    pPort->handle = pPort;
    return pPort;
}

void
xf86ConfFreeXvPortData(confXvPortPtr pPort)
{
    if (!pPort)
	return;

    if (pPort->identifier) {
	xfree(pPort->identifier);
	pPort->identifier = NULL;
    }
    if (pPort->options) {
	xf86OptionListFree(pPort->options);
	pPort->options = NULL;
    }
}

confXvPortPtr
xf86ConfDupXvPort(const confXvPortRec *pPort)
{
    confXvPortPtr p;

    if (!pPort)
	return NULL;

    p = xf86ConfAllocXvPort();
    if (!p)
	return NULL;

    p->handle = pPort->handle;
    p->identifier = xstrdup(pPort->identifier);
    if (pPort->identifier && !p->identifier) {
	xf86ConfFreeXvPortData(p);
	xfree(p);
	return NULL;
    }
    p->options = xf86OptionListDup(pPort->options);
    if (pPort->options && !p->options) {
	xf86ConfFreeXvPortData(p);
	xfree(p);
	return NULL;
    }
    return p;
}

static confXvPortPtr
convertXvPort(confXvPortPtr pPort, const XF86ConfVideoPortRec *confPort)
{
    if (!pPort || ! confPort)
	return NULL;

    pPort->handle = confPort;
    pPort->identifier = xstrdup(confPort->vp_identifier);
    if (confPort->vp_identifier && !pPort->identifier) {
	xf86ConfFreeXvPortData(pPort);
	return NULL;
    }
    pPort->options = xf86OptionListDup(confPort->vp_option_lst);
    if (confPort->vp_option_lst && !pPort->options) {
	xf86ConfFreeXvPortData(pPort);
	return NULL;
    }
    return pPort;
}

confXvAdaptorPtr
xf86ConfAllocXvAdaptor()
{
    confXvAdaptorPtr pAdaptor;

    pAdaptor = xcalloc(1, sizeof(*pAdaptor));
    pAdaptor->handle = pAdaptor;
    return pAdaptor;
}

void
xf86ConfFreeXvAdaptorData(confXvAdaptorPtr pAdaptor)
{
    int i;

    if (!pAdaptor)
	return;

    if (pAdaptor->identifier) {
	xfree(pAdaptor->identifier);
	pAdaptor->identifier = NULL;
    }
    if (pAdaptor->xvPorts) {
	for (i = 0; i < pAdaptor->numports; i++) {
	    xf86ConfFreeXvPortData(pAdaptor->xvPorts[i]);
	    xfree(pAdaptor->xvPorts[i]);
	    pAdaptor->xvPorts[i] = NULL;
	}
	xfree(pAdaptor->xvPorts);
	pAdaptor->xvPorts = NULL;
    }
    pAdaptor->numports = 0;
    if (pAdaptor->options) {
	xf86OptionListFree(pAdaptor->options);
	pAdaptor->options = NULL;
    }
}

confXvAdaptorPtr
xf86ConfDupXvAdaptor(const confXvAdaptorRec *pAdaptor)
{
    int i;
    confXvAdaptorPtr a;

    if (!pAdaptor)
	return NULL;

    a = xf86ConfAllocXvAdaptor();
    if (!a)
	return NULL;

    a->handle = pAdaptor->handle;
    a->identifier = xstrdup(pAdaptor->identifier);
    if (pAdaptor->identifier && !a->identifier) {
	xf86ConfFreeXvAdaptorData(a);
	xfree(a);
	return NULL;
    }
    a->options = xf86OptionListDup(pAdaptor->options);
    if (pAdaptor->options && !a->options) {
	xf86ConfFreeXvAdaptorData(a);
	xfree(a);
	return NULL;
    }
    a->numports = pAdaptor->numports;
    if (a->numports > 0) {
	a->xvPorts = xcalloc(a->numports, sizeof(confXvPortPtr));
	if (!a->xvPorts) {
	    xf86ConfFreeXvAdaptorData(a);
	    xfree(a);
	    return NULL;
	}
	for (i = 0; i < a->numports; i++) {
	    a->xvPorts[i] = xf86ConfDupXvPort(pAdaptor->xvPorts[i]);
	    if (pAdaptor->xvPorts[i] && !a->xvPorts[i]) {
		xf86ConfFreeXvAdaptorData(a);
		xfree(a);
		return NULL;
	    }
	}
    }
    return a;
}

static confXvAdaptorPtr
convertXvAdaptor(confXvAdaptorPtr pAdaptor,
		 const XF86ConfVideoAdaptorRec *confAdaptor)
{
    int count = 0;
    XF86ConfVideoPortPtr confPort;

    if (!pAdaptor || ! confAdaptor)
	return NULL;

    pAdaptor->handle = confAdaptor;
    pAdaptor->identifier = xstrdup(confAdaptor->va_identifier);
    if (confAdaptor->va_identifier && !pAdaptor->identifier) {
	xf86ConfFreeXvAdaptorData(pAdaptor);
	return NULL;
    }
    pAdaptor->options = xf86OptionListDup(confAdaptor->va_option_lst);
    if (confAdaptor->va_option_lst && !pAdaptor->options) {
	xf86ConfFreeXvAdaptorData(pAdaptor);
	return NULL;
    }

#if 0
    if (conf_adaptor->va_busid || conf_adaptor->va_driver) {
	xf86Msg(X_CONFIG, "|   | Unsupported device type, skipping entry\n");
	return FALSE;
    }
#endif

    /*
     * Figure out how many videoport subsections there are and fill them in.
     */
    confPort = confAdaptor->va_port_lst;
    while (confPort) {
	count++;
	confPort = confPort->list.next;
    }
    if (count > 0) {
	pAdaptor->xvPorts = xcalloc(count, sizeof(confXvAdaptorPtr));
	if (!pAdaptor->xvPorts) {
	    xf86ConfFreeXvAdaptorData(pAdaptor);
	    return NULL;
	}
	pAdaptor->numports = count;
	count = 0;
	confPort = confAdaptor->va_port_lst;
	while (confPort) {
	    pAdaptor->xvPorts[count] = xf86ConfAllocXvPort();
	    if (!pAdaptor->xvPorts[count]) {
		xf86ConfFreeXvAdaptorData(pAdaptor);
		return NULL;
	    }
	    if (!convertXvPort(pAdaptor->xvPorts[count], confPort)) {
		xf86ConfFreeXvAdaptorData(pAdaptor);
		return NULL;
	    }
	    count++;
	    confPort = confPort->list.next;
	}
    }
    return pAdaptor;
}

confXvAdaptorPtr
xf86ConfGetXvAdaptorByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    XF86ConfVideoAdaptorPtr a;
    confXvAdaptorPtr pAdaptor = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (a = pConfig->conf_videoadaptor_lst; a; a = a->list.next) {
	if (xf86nameCompare(a->va_identifier, name) == 0) {
	    break;
	}
    }
    if (a) {
	pAdaptor = xf86ConfAllocXvAdaptor();
	if (pAdaptor) {
	    if (convertXvAdaptor(pAdaptor, a))
		return pAdaptor;
	    else
		xfree(pAdaptor);
	}
    }
    return NULL;
}

static const confXvAdaptorRec *
getNextActiveXvAdaptor(ConfigDataHandle prev)
{
    const confScreenRec *pConfScreen = NULL;
    int i = 0;
    Bool found = FALSE;

    while ((pConfScreen = getNextActiveScreen(pConfScreen))) {
	for (i = 0; i < pConfScreen->numXvAdaptors; i++) {
	    if (!prev) {
		found = TRUE;
		break;
	    } else if (pConfScreen->xvAdaptorList &&
		       pConfScreen->xvAdaptorList[i]->handle == prev) {
		i++;
		found = TRUE;
		break;
	    }
	}
	if (found) {
	    if (i < pConfScreen->numXvAdaptors)
		break;
	    else {
		prev = NULL;
		found = FALSE;
	    }
	}
    }
    if (found)
	return pConfScreen->xvAdaptorList[i];
    else
	return NULL;
}

confXvAdaptorPtr
xf86ConfGetNextXvAdaptor(ConfigHandle handle,
			 ConfigDataHandle prevAdaptorHandle)
{
    const XF86ConfigRec *pConfig;
    XF86ConfVideoAdaptorPtr a;
    confXvAdaptorPtr pAdaptor = NULL;

    if (handle == ACTIVE_CONFIG)
	return xf86ConfDupXvAdaptor(getNextActiveXvAdaptor(prevAdaptorHandle));

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevAdaptorHandle) {
	a = pConfig->conf_videoadaptor_lst;
    } else {
	for (a = pConfig->conf_videoadaptor_lst; a; a = a->list.next) {
	    if (a == prevAdaptorHandle) {
		a = a->list.next;
		break;
	    }
	}
    }
    if (a) {
	pAdaptor = xf86ConfAllocXvAdaptor();
	if (pAdaptor) {
	    if (convertXvAdaptor(pAdaptor, a))
		return pAdaptor;
	    else
		xfree(pAdaptor);
	}
    }
    return NULL;
}

confScreenPtr
xf86ConfAllocScreen()
{
    confScreenPtr pConfScreen;

    pConfScreen = xcalloc(1, sizeof(*pConfScreen));
    pConfScreen->screennum = -1;
    pConfScreen->handle = pConfScreen;
    return pConfScreen;
}

void
xf86ConfFreeScreenData(confScreenPtr pConfScreen)
{
    int i;

    if (!pConfScreen)
	return;

    if (pConfScreen->id) {
	xfree(pConfScreen->id);
	pConfScreen->id = NULL;
    }
    if (pConfScreen->monitorNames) {
	for (i = 0; i < pConfScreen->numMonitors; i++) {
	    xfree(pConfScreen->monitorNames[i]);
	    pConfScreen->monitorNames[i] = NULL;
	}
	xfree(pConfScreen->monitorNames);
	pConfScreen->monitorNames = NULL;
    }
    if (pConfScreen->monitorNums) {
	xfree(pConfScreen->monitorNums);
	pConfScreen->monitorNums = NULL;
    }
    if (pConfScreen->monitors) {
	for (i = 0; i < pConfScreen->numMonitors; i++) {
	    xf86ConfFreeMonitorData(pConfScreen->monitors[i]);
	    if (pConfScreen->monitor == pConfScreen->monitors[i])
		pConfScreen->monitor = NULL;
	    xfree(pConfScreen->monitors[i]);
	    pConfScreen->monitors[i] = NULL;
	}
	xfree(pConfScreen->monitors);
	pConfScreen->monitors = NULL;
    }
    pConfScreen->numMonitors = 0;
    if (pConfScreen->monitor) {
	xf86ConfFreeMonitorData(pConfScreen->monitor);
	xfree(pConfScreen->monitor);
	pConfScreen->monitor = NULL;
    }
    if (pConfScreen->deviceName) {
	xfree(pConfScreen->deviceName);
	pConfScreen->deviceName = NULL;
    }
    if (pConfScreen->device) {
	xf86ConfFreeGraphicsDeviceData(pConfScreen->device);
	xfree(pConfScreen->device);
	pConfScreen->device = NULL;
    }
    if (pConfScreen->displayList) {
	for (i = 0; i < pConfScreen->numdisplays; i++) {
	    xf86ConfFreeDisplayData(pConfScreen->displayList[i]);
	    xfree(pConfScreen->displayList[i]);
	    pConfScreen->displayList[i] = NULL;
	}
	xfree(pConfScreen->displayList);
	pConfScreen->displayList = NULL;
    }
    pConfScreen->numdisplays = 0;
    if (pConfScreen->xvAdaptorNames) {
	for (i = 0; i < pConfScreen->numXvAdaptors; i++) {
	    xfree(pConfScreen->xvAdaptorNames[i]);
	    pConfScreen->xvAdaptorNames[i] = NULL;
	}
	xfree(pConfScreen->xvAdaptorNames);
	pConfScreen->xvAdaptorNames = NULL;
    }
    if (pConfScreen->xvAdaptorList) {
	for (i = 0; i < pConfScreen->numXvAdaptors; i++) {
	    xf86ConfFreeXvAdaptorData(pConfScreen->xvAdaptorList[i]);
	    xfree(pConfScreen->xvAdaptorList[i]);
	    pConfScreen->xvAdaptorList[i] = NULL;
	}
	xfree(pConfScreen->xvAdaptorList);
	pConfScreen->xvAdaptorList = NULL;
    }
    pConfScreen->numXvAdaptors = 0;
    if (pConfScreen->options) {
	xf86OptionListFree(pConfScreen->options);
	pConfScreen->options = NULL;
    }
}

confScreenPtr
xf86ConfDupScreen(const confScreenRec *pConfScreen, int depth)
{
    int i;
    confScreenPtr s;

    if (!pConfScreen)
	return NULL;

    s = xf86ConfAllocScreen();
    if (!s)
	return NULL;

    s->handle = pConfScreen->handle;
    s->id = xstrdup(pConfScreen->id);
    if (pConfScreen->id && !s->id) {
	xf86ConfFreeScreenData(s);
	xfree(s);
	return NULL;
    }
    s->deviceName = xstrdup(pConfScreen->deviceName);
    if (pConfScreen->deviceName && !s->deviceName) {
	xf86ConfFreeScreenData(s);
	xfree(s);
	return NULL;
    }
    s->defaultdepth = pConfScreen->defaultdepth;
    s->defaultbpp = pConfScreen->defaultbpp;
    s->defaultfbbpp = pConfScreen->defaultfbbpp;
    s->screennum = pConfScreen->screennum;
    s->numMonitors = pConfScreen->numMonitors;
    if (s->numMonitors > 0) {
	s->monitorNames = xf86DupStringArray(pConfScreen->monitorNames,
					     s->numMonitors);
	if (pConfScreen->monitorNames && !s->monitorNames) {
	    xf86ConfFreeScreenData(s);
	    xfree(s);
	    return NULL;
	}
	if (pConfScreen->monitorNums) {
	    s->monitorNums = xcalloc(s->numMonitors, sizeof(int));
	    if (!s->monitorNums) {
		xf86ConfFreeScreenData(s);
		xfree(s);
		return NULL;
	    }
	    memcpy(s->monitorNums, pConfScreen->monitorNums,
		   s->numMonitors * sizeof(int));
	}
	if (depth > 0 && pConfScreen->monitors) {
	    s->monitors = xcalloc(s->numMonitors, sizeof(MonPtr));
	    if (!s->monitors) {
		xf86ConfFreeScreenData(s);
		xfree(s);
		return NULL;
	    }
	    for (i = 0; i < s->numMonitors; i++) {
		s->monitors[i] =
		    xf86ConfDupMonitor(pConfScreen->monitors[i], depth - 1);
		if (pConfScreen->monitors[i] && !s->monitors[i]) {
		    xf86ConfFreeScreenData(s);
		    xfree(s);
		    return NULL;
		}
	    }
	    s->monitor = s->monitors[0];
	}
    }

    if (pConfScreen->device) {
	if (depth > 0) {
	    s->device = xf86ConfDupGraphicsDevice(pConfScreen->device);
	    if (!s->device) {
		xf86ConfFreeScreenData(s);
		xfree(s);
		return NULL;
	    }
	} else {
	    s->device = xf86ConfAllocGraphicsDevice();
	    if (!s->device) {
		xf86ConfFreeScreenData(s);
		xfree(s);
		return NULL;
	    }
	    s->device->identifier = xstrdup(pConfScreen->device->identifier);
	    if (pConfScreen->device->identifier && !s->device->identifier) {
		xf86ConfFreeScreenData(s);
		xfree(s);
		return NULL;
	    }
	}
    }
    s->options = xf86OptionListDup(pConfScreen->options);
    if (pConfScreen->options && !s->options) {
	xf86ConfFreeScreenData(s);
	xfree(s);
	return NULL;
    }

    s->numdisplays = pConfScreen->numdisplays;
    if (s->numdisplays > 0 && pConfScreen->displayList) {
	s->displayList = xcalloc(s->numdisplays, sizeof(DispPtr));
	if (!s->displayList) {
	    xf86ConfFreeScreenData(s);
	    xfree(s);
	    return NULL;
	}
	for (i = 0; i < s->numdisplays; i++) {
	    s->displayList[i] = xf86ConfDupDisplay(pConfScreen->displayList[i]);
	    if (pConfScreen->displayList[i] && !s->displayList[i]) {
		xf86ConfFreeScreenData(s);
		xfree(s);
		return NULL;
	    }
	}
    }

    s->numXvAdaptors = pConfScreen->numXvAdaptors;
    if (s->numXvAdaptors > 0) {
	s->xvAdaptorNames = xf86DupStringArray(pConfScreen->xvAdaptorNames,
					       s->numXvAdaptors);
	if (pConfScreen->xvAdaptorNames && !s->xvAdaptorNames) {
	    xf86ConfFreeScreenData(s);
	    xfree(s);
	    return NULL;
	}
	if (depth > 0) {
	    s->xvAdaptorList =
		xcalloc(s->numXvAdaptors, sizeof(confXvAdaptorPtr));
	    if (!s->xvAdaptorList) {
		xf86ConfFreeScreenData(s);
		xfree(s);
		return NULL;
	    }
	    for (i = 0; i < s->numXvAdaptors; i++) {
		s->xvAdaptorList[i] =
		    xf86ConfDupXvAdaptor(pConfScreen->xvAdaptorList[i]);
		if (pConfScreen->xvAdaptorList[i] && !s->xvAdaptorList[i]) {
		    xf86ConfFreeScreenData(s);
		    xfree(s);
		    return NULL;
		}
	    }
	}
    }
    return s;
}

static void
resolveScreenCleanup(confScreenPtr pConfScreen, Bool cleanMonNames)
{
    int i;

    if (pConfScreen->monitors) {
	for (i = 0; i < pConfScreen->numMonitors; i++)
	    xf86ConfFreeMonitorData(pConfScreen->monitors[i]);
	xfree(pConfScreen->monitors);
	pConfScreen->monitors = NULL;
    }
    if (cleanMonNames) {
	if (pConfScreen->monitorNames) {
	    for (i = 0; i < pConfScreen->numMonitors; i++)
		xfree(pConfScreen->monitorNames[i]);
	    xfree(pConfScreen->monitorNames);
	    pConfScreen->monitorNames = NULL;
	}
	if (pConfScreen->monitorNums) {
	    xfree(pConfScreen->monitorNums);
	    pConfScreen->monitorNums = NULL;
	}
	pConfScreen->numMonitors = 0;
    }
    if (pConfScreen->device) {
	xf86ConfFreeGraphicsDeviceData(pConfScreen->device);
	pConfScreen->device = NULL;
    }
    if (pConfScreen->xvAdaptorList) {
	for (i = 0; i < pConfScreen->numXvAdaptors; i++)
	    xf86ConfFreeXvAdaptorData(pConfScreen->xvAdaptorList[i]);
	xfree(pConfScreen->xvAdaptorList);
	pConfScreen->xvAdaptorList = NULL;
    }
}

confScreenPtr
xf86ConfResolveScreen(ConfigHandle handle, confScreenPtr pConfScreen,
		      int depth)
{
    int i;
    Bool defaultMonitor = FALSE;

    if (!pConfScreen || depth < 1)
	return pConfScreen;

    /* Clean out any fields that we will be filling in. */
    resolveScreenCleanup(pConfScreen, FALSE);
    if (pConfScreen->numMonitors > 0 && pConfScreen->monitorNames) {
	pConfScreen->monitors =
	    xcalloc(pConfScreen->numMonitors, sizeof(MonPtr));
	if (!pConfScreen->monitors)
	    return NULL;
	for (i = 0; i < pConfScreen->numMonitors; i++) {
	    if (!pConfScreen->monitorNames[i])
		continue;

	    pConfScreen->monitors[i] =
		xf86ConfGetMonitorByName(handle, pConfScreen->monitorNames[i],
					 depth - 1);
	    if (!pConfScreen->monitors[i]) {
		continue;
	    }
	    if (pConfScreen->monitorNums[i])
		pConfScreen->monitors[i]->monitorNum =
			pConfScreen->monitorNums[i];
	}
    } else {
	/* If no monitor is specified, create a default one. */
	XF86ConfMonitorRec defMon;
	MonPtr newMonitor;
	
	bzero(&defMon, sizeof(defMon));
	defMon.mon_identifier = xstrdup("<default monitor>");
	/*
	 * TARGET_REFRESH_RATE may be defined to effectively limit the
	 * default resolution to the largest that has a "good" refresh
	 * rate.
	 */
#ifdef TARGET_REFRESH_RATE
	defMon.mon_option_lst = xf86ReplaceRealOption(defMon.mon_option_lst,
						      "TargetRefresh",
						      TARGET_REFRESH_RATE);
#endif
	pConfScreen->numMonitors = 1;
	pConfScreen->monitorNames = xcalloc(1, sizeof(char *));
	pConfScreen->monitorNums = xcalloc(1, sizeof(int));
	pConfScreen->monitors = xcalloc(1, sizeof(MonPtr));
	newMonitor = xcalloc(1, sizeof(*newMonitor));
	if (!pConfScreen->monitorNames || !pConfScreen->monitorNums ||
	    !pConfScreen->monitors || !newMonitor) {
	    resolveScreenCleanup(pConfScreen, TRUE);
	    xfree(newMonitor);
	    return NULL;
	}
	if (!convertMonitor(newMonitor, &defMon)) {
	    resolveScreenCleanup(pConfScreen, TRUE);
	    xfree(newMonitor);
	    return NULL;
	}
	if (depth - 1 > 0) {
	    if (!xf86ConfResolveMonitor(handle, newMonitor)) {
		resolveScreenCleanup(pConfScreen, TRUE);
		xfree(newMonitor);
		return NULL;
	    }
	}
	newMonitor->defaultMon = TRUE;
	pConfScreen->monitors[0] = newMonitor;
	pConfScreen->monitorNums[0] = newMonitor->monitorNum;
	pConfScreen->monitorNames[0] = xstrdup(newMonitor->id);
	if (newMonitor->id && !pConfScreen->monitorNames[0]) {
	    resolveScreenCleanup(pConfScreen, TRUE);
	    return NULL;
	}
	defaultMonitor = TRUE;
    }
    if (pConfScreen->monitors)
	pConfScreen->monitor = pConfScreen->monitors[0];

    if (defaultMonitor) {
	xf86MsgVerb(X_INFO, 5,
		    "Created %d default Monitor%s for the "
		    "Screen section \"%s\".\n",
		    pConfScreen->numMonitors, PLURAL(pConfScreen->numMonitors),
		    pConfScreen->id);
    } else {
	xf86MsgVerb(X_INFO, 5,
		    "Found %d Monitor%s in the Screen section \"%s\".\n",
		    pConfScreen->numMonitors, PLURAL(pConfScreen->numMonitors),
		    pConfScreen->id);
    }

    pConfScreen->device =
	xf86ConfGetGraphicsDeviceByName(handle, pConfScreen->deviceName);

    if (pConfScreen->device) {
	pConfScreen->device->active = TRUE;
	pConfScreen->device->myScreenSection = pConfScreen;
    }

    if (pConfScreen->numXvAdaptors > 0 && pConfScreen->xvAdaptorNames) {
	pConfScreen->xvAdaptorList =
	    xcalloc(pConfScreen->numXvAdaptors, sizeof(confXvAdaptorPtr));
	if (!pConfScreen->xvAdaptorList) {
	    resolveScreenCleanup(pConfScreen, defaultMonitor);
	    return NULL;
	}
	for (i = 0; i < pConfScreen->numXvAdaptors; i++) {
	    pConfScreen->xvAdaptorList[i] =
		xf86ConfGetXvAdaptorByName(handle,
					   pConfScreen->xvAdaptorNames[i]);
	}
    }

    if (defaultMonitor) {
	xf86Msg(X_WARNING, "No monitor specified for screen \"%s\".\n"
		"\tUsing a default monitor configuration.\n", pConfScreen->id);
    }
    return pConfScreen;
}

int
xf86ConfCheckResolvedScreen(const confScreenRec *pConfScreen, int depth,
			    Bool strict)
{
    int i;
    int failures = 0;

    if (!pConfScreen || depth < 1)
	return 0;

    if (pConfScreen->numMonitors > 0 && pConfScreen->monitorNames) {
	for (i = 0; i < pConfScreen->numMonitors; i++) {
	    if (!pConfScreen->monitorNames[i])
		continue;
	    if (pConfScreen->monitors && pConfScreen->monitors[i]) {
		failures +=
		    xf86ConfCheckResolvedMonitor(pConfScreen->monitors[i],
						 depth - 1, strict);
	    } else {
		xf86Msg(X_ERROR, "No Monitor \"%s\" "
				 "(referenced by Screen \"%s\").\n",
			pConfScreen->monitorNames[i], pConfScreen->id);
		failures++;
	    }
	}
    } else if (strict) {
	xf86Msg(X_ERROR, "No Monitor references in Screen \"%s\".\n",
		pConfScreen->id);
	failures++;
    }

    if (pConfScreen->deviceName) {
	if (!pConfScreen->device) {
	    xf86Msg(X_ERROR, "No Device \"%s\" "
			       "(referenced by Screen \"%s\").\n",
		    pConfScreen->deviceName, pConfScreen->id);
	    failures++;
	}
    } else if (strict) {
	xf86Msg(X_ERROR, "No Device references in Screen \"%s\".\n",
		pConfScreen->id);
	failures++;
    }

    if (pConfScreen->numXvAdaptors > 0 && pConfScreen->xvAdaptorNames) {
	for (i = 0; i < pConfScreen->numXvAdaptors; i++) {
	    if (!pConfScreen->xvAdaptorNames[i])
		continue;
	    if (!pConfScreen->xvAdaptorList || !pConfScreen->xvAdaptorList[i]) {
		xf86Msg(X_ERROR, "No VideoAdaptor \"%s\" "
				 "(referenced by Screen \"%s\").\n",
			pConfScreen->xvAdaptorNames[i], pConfScreen->id);
		failures++;
	    }
	}
    }
    return failures;
}

/*
 * assignIndices takes an array of numbers indexed by config file ordering
 * and assigns values to the unassigned (negative) values.  The values
 * assigned begin with the largest already-assigned value + 1, and the
 * unassigned values are incremented in config file order.
 *
 * Note: duplicate already-assigned values are not resolved.  This is typically
 * a configuration error, but handling such things is up to the driver, and
 * optional.
 *
 * Examples:
 *
 *   1. {-1, 0, -1, 1} -> {2, 0, 3, 1}
 *   2. {0, 1, 2, 1} -> {0, 1, 2, 1}
 *   3. {1, 3, -1, 5} -> {1, 3, 6, 5}
 */

static void
assignIndices(int *mapping, int count)
{
    int i;
    int max = -1;

    /* First pass: find largest assigned value. */
    for (i = 0; i < count; i++) {
	if (mapping[i] > max)
	    max = mapping[i];
    }

    /* Second pass: assign values to unassigned slots. */
    for (i = 0; i < count; i++) {
	if (mapping[i] < 0)
	    mapping[i] = ++max;
    }
}

static confScreenPtr
convertScreen(confScreenPtr pConfScreen, const XF86ConfScreenRec *confScreen)
{
    int count;
    XF86ConfDisplayPtr dispptr;
    XF86ConfAdaptorLinkPtr conf_adaptor;
    XF86ConfMonitorListPtr mlistp;
    int i;
    int *mapping;

    if (!pConfScreen || !confScreen)
	return NULL;

    pConfScreen->handle = confScreen;
    pConfScreen->id = xstrdup(confScreen->scrn_identifier);
    if (confScreen->scrn_identifier && !pConfScreen->id) {
	xf86ConfFreeScreenData(pConfScreen);
	return NULL;
    }
    pConfScreen->defaultdepth = confScreen->scrn_defaultdepth;
    pConfScreen->defaultbpp = confScreen->scrn_defaultbpp;
    pConfScreen->defaultfbbpp = confScreen->scrn_defaultfbbpp;
    pConfScreen->screennum = -1;

    /* Figure out how many monitor references there are. */
    mlistp = confScreen->scrn_monitor_lst;
    count = 0;
    while (mlistp) {
	count++;
	mlistp = mlistp->list.next;
    }
    pConfScreen->numMonitors = count;
    if (count > 0) {
	pConfScreen->monitorNames = xcalloc(count, sizeof(char *));
	if (!pConfScreen->monitorNames) {
	    xf86ConfFreeScreenData(pConfScreen);
	    return NULL;
	}
	mapping = xcalloc(count, sizeof(int));
	if (!mapping) {
	    xf86ConfFreeScreenData(pConfScreen);
	    return NULL;
	}
	mlistp = confScreen->scrn_monitor_lst;
	for (i = 0; i < count; i++) {
	    mapping[i] = mlistp->monitor_num;
	    mlistp = mlistp->list.next;
	}
	assignIndices(mapping, count);
	pConfScreen->monitorNums = mapping;
	mlistp = confScreen->scrn_monitor_lst;
	for (i = 0; i < count; i++) {
	    pConfScreen->monitorNames[i] = xstrdup(mlistp->monitor_str);
	    if (mlistp->monitor_str && !pConfScreen->monitorNames[i]) {
		xf86ConfFreeScreenData(pConfScreen);
		return NULL;
	    }
	    mlistp = mlistp->list.next;
	}
    }

    pConfScreen->deviceName = xstrdup(confScreen->scrn_device_str);
    if (confScreen->scrn_device_str && !pConfScreen->deviceName) {
	xf86ConfFreeScreenData(pConfScreen);
	return NULL;
    }
    pConfScreen->options = xf86OptionListDup(confScreen->scrn_option_lst);
    if (confScreen->scrn_option_lst && !pConfScreen->options) {
	xf86ConfFreeScreenData(pConfScreen);
	return NULL;
    }
    
    /* Figure out how many display subsections there are, and allocate them. */
    dispptr = confScreen->scrn_display_lst;
    count = 0;
    while (dispptr) {
	count++;
	dispptr = dispptr->list.next;
    }
    pConfScreen->numdisplays = count;
    if (count > 0) {
	pConfScreen->displayList = xcalloc(count, sizeof(DispPtr));
	if (!pConfScreen->displayList) {
	    xf86ConfFreeScreenData(pConfScreen);
	    return NULL;
	}
	count = 0;
	dispptr = confScreen->scrn_display_lst;
	while (dispptr) {
	    pConfScreen->displayList[count] = xf86ConfAllocDisplay();
	    if (!pConfScreen->displayList[count]) {
		xf86ConfFreeScreenData(pConfScreen);
		return NULL;
	    }
	    if (!convertDisplay(pConfScreen->displayList[count], dispptr)) {
		xf86ConfFreeScreenData(pConfScreen);
		return NULL;
	    }
	    count++;
	    dispptr = dispptr->list.next;
	}
    }

    /*
     * Figure out how many videoadaptor references there.
     */
    count = 0;
    conf_adaptor = confScreen->scrn_adaptor_lst;
    while (conf_adaptor) {
	count++;
	conf_adaptor = conf_adaptor->list.next;
    }
    pConfScreen->numXvAdaptors = count;
    if (count > 0) {
	pConfScreen->xvAdaptorNames = xcalloc(count, sizeof(char *));
	if (!pConfScreen->xvAdaptorNames) {
	    xf86ConfFreeScreenData(pConfScreen);
	    return NULL;
	}
	conf_adaptor = confScreen->scrn_adaptor_lst;
	count = 0;
	while (conf_adaptor) {
	    pConfScreen->xvAdaptorNames[count] =
		xstrdup(conf_adaptor->al_adaptor_str);
	    if (conf_adaptor->al_adaptor_str &&
		!pConfScreen->xvAdaptorNames[count]) {
		xf86ConfFreeScreenData(pConfScreen);
		return NULL;
	    }
	    count++;
	    conf_adaptor = conf_adaptor->list.next;
	}
    }
    return pConfScreen;
}

confScreenPtr
xf86ConfGetScreenByName(ConfigHandle handle, const char *name, int depth)
{
    const XF86ConfigRec *pConfig;
    XF86ConfScreenPtr s;
    confScreenPtr pConfScreen = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (s = pConfig->conf_screen_lst; s; s = s->list.next) {
	if (xf86nameCompare(s->scrn_identifier, name) == 0) {
	    break;
	}
    }
    if (s) {
	pConfScreen = xf86ConfAllocScreen();
	if (pConfScreen) {
	    if (!convertScreen(pConfScreen, s)) {
		xfree(pConfScreen);
		return NULL;
	    }
	    if (depth > 0) {
		if (!xf86ConfResolveScreen(handle, pConfScreen, depth)) {
		    xf86ConfFreeScreenData(pConfScreen);
		    xfree(pConfScreen);
		    return NULL;
		}
	    }
	    return pConfScreen;
	}
    }
    return NULL;
}

static const confScreenRec *
getNextActiveScreen(ConfigDataHandle prev)
{
    int i;
    Bool found = FALSE;

    for (i = 0; i < xf86Info.serverLayout->numScreens; i++) {
	if (!prev) {
	    found = TRUE;
	    break;
	} else if (xf86Info.serverLayout->screenLayouts[i]->screen->handle ==
		   prev) {
	    i++;
	    found = TRUE;
	    break;
	}
    }
    if (found && i < xf86Info.serverLayout->numScreens)
	return xf86Info.serverLayout->screenLayouts[i]->screen;
    else
	return NULL;
}

confScreenPtr
xf86ConfGetNextScreen(ConfigHandle handle, ConfigDataHandle prevScreenHandle,
		      int depth)
{
    const XF86ConfigRec *pConfig;
    XF86ConfScreenPtr s;
    confScreenPtr pConfScreen = NULL;

    if (handle == ACTIVE_CONFIG)
	return xf86ConfDupScreen(getNextActiveScreen(prevScreenHandle), depth);

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevScreenHandle) {
	s = pConfig->conf_screen_lst;
    } else {
	for (s = pConfig->conf_screen_lst; s; s = s->list.next) {
	    if (s == prevScreenHandle) {
		s = s->list.next;
		break;
	    }
	}
    }
    if (s) {
	pConfScreen = xf86ConfAllocScreen();
	if (pConfScreen) {
	    if (!convertScreen(pConfScreen, s)) {
		xfree(pConfScreen);
		return NULL;
	    }
	    if (depth > 0) {
		if (!xf86ConfResolveScreen(handle, pConfScreen, depth)) {
		    xf86ConfFreeScreenData(pConfScreen);
		    xfree(pConfScreen);
		    return NULL;
		}
	    }
	    return pConfScreen;
	}
    }
    return NULL;
}

DisplayModePtr
xf86ConfAllocMode()
{
    DisplayModePtr pMode;

    pMode = xcalloc(1, sizeof(*pMode));
    return pMode;
}

void
xf86ConfFreeModeData(DisplayModePtr pMode)
{
    if (!pMode)
	return;

    if (pMode->name) {
	xfree(pMode->name);
	pMode->name = NULL;
    }
    return;
}

DisplayModePtr
xf86ConfDupMode(const DisplayModeRec *pMode, int depth)
{
    DisplayModePtr m;

    if (!pMode)
	return NULL;

    m = xf86ConfAllocMode();
    if (!m)
	return NULL;

    if (depth > 0) {
	*m = *pMode;
	m->prev = NULL;
	m->next = NULL;
	m->name = NULL;
    } else {
	m->Clock = pMode->Clock;
	m->HDisplay = pMode->HDisplay;
	m->HSyncStart = pMode->HSyncStart;
	m->HSyncEnd = pMode->HSyncEnd;
	m->HTotal = pMode->HTotal;
	m->VDisplay = pMode->VDisplay;
	m->VSyncStart = pMode->VSyncStart;
	m->VSyncEnd = pMode->VSyncEnd;
	m->VTotal = pMode->VTotal;
	m->Flags = pMode->Flags;
	m->HSkew = pMode->HSkew;
	m->VScan = pMode->VScan;
    }
    m->name = xstrdup(pMode->name);
    if (pMode->name && !m->name) {
	xf86ConfFreeModeData(m);
	xfree(m);
	return NULL;
    }
    return m;
}

DisplayModePtr
xf86ConfAllocModeList(int n)
{
    DisplayModePtr pModes, prev = NULL, first = NULL, last = NULL;
    int i;

    if (n <= 0)
	return NULL;

    for (i = 0; i < n; i++) {
	pModes = xf86ConfAllocMode();
	if (!pModes) {
	    xf86ConfFreeModeList(first);
	    return NULL;
	}
	if (prev) {
	    prev->next = pModes;
	    pModes->prev = prev;
	}
	if (!first)
	    first = pModes;
	prev = last = pModes;
    }
    /* Pass back the 'last' value through first->prev. */
    if (first)
	first->prev = last;
    return first;
}

void
xf86ConfFreeModeList(DisplayModePtr pModes)
{
    DisplayModePtr m, next, first;

    if (!pModes)
	return;

    m = first = pModes;
    do {
	next = m->next;
	xf86ConfFreeModeData(m);
	xfree(m);
	m = next;
    } while (m && m != first);
}

DisplayModePtr
xf86ConfDupModeList(const DisplayModeRec *pModes, int depth)
{
    DisplayModePtr m, prev = NULL, first = NULL, last = NULL;
    const DisplayModeRec *pModeFirst;

    if (!pModes)
	return NULL;

    pModeFirst = pModes;
    do {
	m = xf86ConfDupMode(pModes, depth);
	if (!m) {
	    xf86ConfFreeModeList(first);
	    return NULL;
	}
	if (prev) {
	    prev->next = m;
	    m->prev = prev;
	}
	if (!first)
	    first = m;
	prev = last = m;
	pModes = pModes->next;
    } while (pModes && pModes != pModeFirst);
    /* Pass back the 'last' value through first->prev. */
    if (first)
	first->prev = last;
    return first;
}

static DisplayModePtr
convertMode(DisplayModePtr pMode, const XF86ConfModeLineRec *confMode)
{
    if (!pMode || !confMode)
	return NULL;

    pMode->type = 0;
    pMode->Clock = confMode->ml_clock;
    pMode->HDisplay = confMode->ml_hdisplay;
    pMode->HSyncStart = confMode->ml_hsyncstart;
    pMode->HSyncEnd = confMode->ml_hsyncend;
    pMode->HTotal = confMode->ml_htotal;
    pMode->VDisplay = confMode->ml_vdisplay;
    pMode->VSyncStart = confMode->ml_vsyncstart;
    pMode->VSyncEnd = confMode->ml_vsyncend;
    pMode->VTotal = confMode->ml_vtotal;
    pMode->Flags = confMode->ml_flags;
    pMode->HSkew = confMode->ml_hskew;
    pMode->VScan = confMode->ml_vscan;
    pMode->name = xstrdup(confMode->ml_identifier);
    if (confMode->ml_identifier && !pMode->name) {
	xf86ConfFreeModeData(pMode);
	return NULL;
    }
    return pMode;
}

confModeSetPtr
xf86ConfAllocModeSet()
{
    confModeSetPtr pModeSet;

    pModeSet = xcalloc(1, sizeof(*pModeSet));
    pModeSet->handle = pModeSet;
    return pModeSet;
}

void
xf86ConfFreeModeSetData(confModeSetPtr pModeSet)
{
    if (!pModeSet)
	return;

    if (pModeSet->identifier) {
	xfree(pModeSet->identifier);
	pModeSet->identifier = NULL;
    }
    if (pModeSet->options) {
	xf86OptionListFree(pModeSet->options);
	pModeSet->options = NULL;
    }
    if (pModeSet->modes) {
	xf86ConfFreeModeList(pModeSet->modes);
	pModeSet->modes = NULL;
    }
}

confModeSetPtr
xf86ConfDupModeSet(const confModeSetRec *pModeSet, int depth)
{
    confModeSetPtr ms;

    if (!pModeSet)
	return NULL;

    ms = xf86ConfAllocModeSet();
    if (!ms)
	return NULL;

    ms->handle = pModeSet->handle;
    ms->identifier = xstrdup(pModeSet->identifier);
    if (pModeSet->identifier && !ms->identifier) {
	xf86ConfFreeModeSetData(ms);
	xfree(ms);
	return NULL;
    }
    ms->options = xf86OptionListDup(pModeSet->options);
    if (pModeSet->options && !ms->options) {
	xf86ConfFreeModeSetData(ms);
	xfree(ms);
	return NULL;
    }
    ms->modes = xf86ConfDupModeList(pModeSet->modes, depth);
    if (pModeSet->modes && !ms->modes) {
	xf86ConfFreeModeSetData(ms);
	xfree(ms);
	return NULL;
    }

    /*
     * Note: As per xf86ConfDupModeList(), the last value is passed
     * back through ms->modes->prev.
     */
    return ms;
}

static confModeSetPtr
convertModeSet(confModeSetPtr pModeSet, const XF86ConfModesRec *confModeSet)
{
    DisplayModePtr m;
    XF86ConfModeLinePtr cmlp;
    int count;

    if (!pModeSet || !confModeSet)
	return NULL;

    pModeSet->handle = confModeSet;
    pModeSet->identifier = xstrdup(confModeSet->modes_identifier);
    if (confModeSet->modes_identifier && !pModeSet->identifier) {
	xf86ConfFreeModeSetData(pModeSet);
	return NULL;
    }
    pModeSet->options = xf86OptionListDup(confModeSet->modes_option_lst);
    if (confModeSet->modes_option_lst && !pModeSet->options) {
	xf86ConfFreeModeSetData(pModeSet);
	return NULL;
    }
    pModeSet->modes = NULL;
    /* Cound the number of modes. */
    cmlp = confModeSet->mon_modeline_lst;
    count = 0;
    while (cmlp) {
	count++;
	cmlp = cmlp->list.next;
    }
    if (count > 0) {
	pModeSet->modes = xf86ConfAllocModeList(count);
	if (!pModeSet->modes) {
	    xf86ConfFreeModeSetData(pModeSet);
	    return NULL;
	}
	cmlp = confModeSet->mon_modeline_lst;
	m = pModeSet->modes;
	while (cmlp && m) {
	    if (!convertMode(m, cmlp)) {
		xf86ConfFreeModeSetData(pModeSet);
		return NULL;
	    }
	    cmlp = cmlp->list.next;
	    m = m->next;
	}
    }

    /*
     * Note: As per xf86ConfAllocModeList(), the last value is passed
     * back through pModeSet->modes->prev.
     */
    return pModeSet;
}

confModeSetPtr
xf86ConfGetModeSetByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    XF86ConfModesPtr cmp;
    confModeSetPtr pModeSet = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (cmp = pConfig->conf_modes_lst; cmp; cmp = cmp->list.next) {
	if (xf86nameCompare(cmp->modes_identifier, name) == 0) {
	    break;
	}
    }
    if (cmp) {
	pModeSet = xf86ConfAllocModeSet();
	if (pModeSet) {
	    if (convertModeSet(pModeSet, cmp))
		return pModeSet;
	    else
		xfree(pModeSet);
	}
    }
    return NULL;
}

confModeSetPtr
xf86ConfGetNextModeSet(ConfigHandle handle, ConfigDataHandle prevModeSetHandle)
{
    const XF86ConfigRec *pConfig;
    XF86ConfModesPtr cmp;
    confModeSetPtr pModeSet = NULL;

    if (handle == ACTIVE_CONFIG)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevModeSetHandle) {
	cmp = pConfig->conf_modes_lst;
    } else {
	for (cmp = pConfig->conf_modes_lst; cmp; cmp = cmp->list.next) {
	    if (cmp == prevModeSetHandle) {
		cmp = cmp->list.next;
		break;
	    }
	}
    }
    if (cmp) {
	pModeSet = xf86ConfAllocModeSet();
	if (pModeSet) {
	    if (convertModeSet(pModeSet, cmp))
		return pModeSet;
	    else
		xfree(pModeSet);
	}
    }
    return NULL;
}

MonPtr
xf86ConfAllocMonitor()
{
    MonPtr pMonitor;

    pMonitor = xcalloc(1, sizeof(*pMonitor));
    pMonitor->handle = pMonitor;
    return pMonitor;
}

void
xf86ConfFreeMonitorData(MonPtr pMonitor)
{
    int i;

    if (!pMonitor)
	return;

    if (pMonitor->id) {
	xfree(pMonitor->id);
	pMonitor->id = NULL;
    }
    if (pMonitor->vendor) {
	xfree(pMonitor->vendor);
	pMonitor->vendor = NULL;
    }
    if (pMonitor->model) {
	xfree(pMonitor->model);
	pMonitor->model = NULL;
    }
    if (pMonitor->modeSetNames) {
	for (i = 0; i < pMonitor->numModeSetNames; i++) {
	    if (pMonitor->modeSetNames[i]) {
		xfree(pMonitor->modeSetNames[i]);
		pMonitor->modeSetNames[i] = NULL;
	    }
	}
	xfree(pMonitor->modeSetNames);
	pMonitor->modeSetNames = NULL;
    }
    pMonitor->numModeSetNames = 0;
    if (pMonitor->Modes) {
	xf86ConfFreeModeList(pMonitor->Modes);
	pMonitor->Modes = NULL;
	pMonitor->Last = NULL;
    }
    if (pMonitor->options) {
	xf86OptionListFree(pMonitor->options);
	pMonitor->options = NULL;
    }
}

MonPtr
xf86ConfDupMonitor(const MonRec *pMonitor, int depth)
{
    int i;
    MonPtr m;
    
    if (!pMonitor)
	return NULL;

    m = xf86ConfAllocMonitor();
    if (!m)
	return NULL;

    m->handle = pMonitor->handle;
    m->defaultMon = pMonitor->defaultMon;
    m->flags = pMonitor->flags;
    m->id = xstrdup(pMonitor->id);
    if (pMonitor->id && !m->id) {
	xf86ConfFreeMonitorData(m);
	xfree(m);
	return NULL;
    }
    m->vendor = xstrdup(pMonitor->vendor);
    if (pMonitor->vendor && !m->vendor) {
	xf86ConfFreeMonitorData(m);
	xfree(m);
	return NULL;
    }
    m->model = xstrdup(pMonitor->model);
    if (pMonitor->model && !m->model) {
	xf86ConfFreeMonitorData(m);
	xfree(m);
	return NULL;
    }
    m->Modes = xf86ConfDupModeList(pMonitor->Modes, depth);
    if (pMonitor->Modes && !m->Modes) {
	xf86ConfFreeMonitorData(m);
	xfree(m);
	return NULL;
    }
    if (m->Modes) {
	m->Last = m->Modes->prev;
	m->Modes->prev = NULL;
    }
    m->gamma = pMonitor->gamma;
    m->widthmm = pMonitor->widthmm;
    m->heightmm = pMonitor->heightmm;
    m->options = xf86OptionListDup(pMonitor->options);
    if (pMonitor->options && !m->options) {
	xf86ConfFreeMonitorData(m);
	xfree(m);
	return NULL;
    }
    memcpy(m->hsync, pMonitor->hsync, sizeof(m->hsync));
    memcpy(m->vrefresh, pMonitor->vrefresh, sizeof(m->vrefresh));
    m->nHsync = pMonitor->nHsync;
    m->nVrefresh = pMonitor->nVrefresh;

    m->numModeSetNames = pMonitor->numModeSetNames;
    if (m->numModeSetNames > 0 && pMonitor->modeSetNames) {
	m->modeSetNames = xcalloc(m->numModeSetNames + 1, sizeof(char *));
	if (!m->modeSetNames) {
	    xf86ConfFreeMonitorData(m);
	    xfree(m);
	    return NULL;
	}
	for (i = 0; i < m->numModeSetNames; i++) {
	    m->modeSetNames[i] = xstrdup(pMonitor->modeSetNames[i]);
	    if (pMonitor->modeSetNames[i] && !m->modeSetNames[i]) {
		xf86ConfFreeMonitorData(m);
		xfree(m);
		return NULL;
	    }
	}
    }
    return m;
}

MonPtr
xf86ConfResolveMonitor(ConfigHandle handle, MonPtr pMonitor)
{
    confModeSetPtr pModeSet;
    DisplayModePtr modes = NULL, last = NULL;
    DisplayModePtr localModes, localLast;
    int i;

    if (!pMonitor)
	return NULL;

    localModes = pMonitor->Modes;
    localLast = pMonitor->Last;
    pMonitor->Modes = NULL;
    pMonitor->Last = NULL;

    /* Put the UseModes references first. */
    if (pMonitor->modeSetNames && pMonitor->numModeSetNames > 0) {
	for (i = 0; i < pMonitor->numModeSetNames; i++) {
	    if (!pMonitor->modeSetNames[i])
		continue;
	    pModeSet =
		xf86ConfGetModeSetByName(handle, pMonitor->modeSetNames[i]);
	    if (!pModeSet) {
		continue;
	    }
	    modes = pModeSet->modes;
	    pModeSet->modes = NULL;
	    xf86ConfFreeModeSetData(pModeSet);
	    xfree(pModeSet);
	    pModeSet = NULL;
	    if (modes) {
		last = modes->prev;
		modes->prev = pMonitor->Last;
		if (!pMonitor->Modes)
			pMonitor->Modes = modes;
		else
			pMonitor->Last->next = modes;
		pMonitor->Last = last;
	    }
	}
    }

    /* Add back in the local modelines. */
    if (localModes) {
	localModes->prev = pMonitor->Last;
	if (!pMonitor->Modes)
	    pMonitor->Modes = localModes;
	else
	    pMonitor->Last->next = localModes;
	pMonitor->Last = localLast;
    }

    /* Add the (VESA) default modes. */
    if (!addDefaultModes(pMonitor)) {
	xf86Msg(X_WARNING, "Could not add default built-in modes.\n");
    }
    return pMonitor;
}

int
xf86ConfCheckResolvedMonitor(const MonRec *pMonitor, int depth, Bool strict)
{
    int failures = 0;

    if (!pMonitor || depth < 1)
	return 0;

    if (strict && (!pMonitor->Modes || !pMonitor->Last)) {
	xf86Msg(X_ERROR, "No modes for Monitor \"%s\".\n", pMonitor->id);
	failures++;
    }
    return failures;
}

static MonPtr
convertMonitor(MonPtr pMonitor, const XF86ConfMonitorRec *confMonitor)
{
    int count, i;
    XF86ConfModesLinkPtr modesLink;
    Gamma zeros = {0.0, 0.0, 0.0};
    float badgamma = 0.0;
    XF86ConfModeLinePtr cmodep;
    DisplayModePtr mode;
    
    if (!pMonitor || !confMonitor)
	return NULL;

    pMonitor->handle = confMonitor;
    pMonitor->id = xstrdup(confMonitor->mon_identifier);
    if (confMonitor->mon_identifier && !pMonitor->id) {
	xf86ConfFreeMonitorData(pMonitor);
	return NULL;
    }
    pMonitor->vendor = xstrdup(confMonitor->mon_vendor);
    if (confMonitor->mon_vendor && !pMonitor->vendor) {
	xf86ConfFreeMonitorData(pMonitor);
	return NULL;
    }
    pMonitor->model = xstrdup(confMonitor->mon_modelname);
    if (confMonitor->mon_modelname && !pMonitor->model) {
	xf86ConfFreeMonitorData(pMonitor);
	return NULL;
    }
    pMonitor->Modes = NULL;
    pMonitor->Last = NULL;
    pMonitor->gamma = zeros;
    pMonitor->widthmm = confMonitor->mon_width;
    pMonitor->heightmm = confMonitor->mon_height;
    pMonitor->options = xf86OptionListDup(confMonitor->mon_option_lst);
    if (confMonitor->mon_option_lst && !pMonitor->options) {
	xf86ConfFreeMonitorData(pMonitor);
	return NULL;
    }

    for (i = 0; i < confMonitor->mon_n_hsync; i++) {
	pMonitor->hsync[i].hi = confMonitor->mon_hsync[i].hi;
	pMonitor->hsync[i].lo = confMonitor->mon_hsync[i].lo;
    }
    pMonitor->nHsync = confMonitor->mon_n_hsync;
    for (i = 0; i < confMonitor->mon_n_vrefresh; i++) {
	pMonitor->vrefresh[i].hi = confMonitor->mon_vrefresh[i].hi;
	pMonitor->vrefresh[i].lo = confMonitor->mon_vrefresh[i].lo;
    }
    pMonitor->nVrefresh = confMonitor->mon_n_vrefresh;

    if (confMonitor->mon_gamma_red > GAMMA_ZERO)
	pMonitor->gamma.red = confMonitor->mon_gamma_red;
    if (confMonitor->mon_gamma_green > GAMMA_ZERO)
	pMonitor->gamma.green = confMonitor->mon_gamma_green;
    if (confMonitor->mon_gamma_blue > GAMMA_ZERO)
	pMonitor->gamma.blue = confMonitor->mon_gamma_blue;
    
    /* Check that the gamma values are within range. */
    if (pMonitor->gamma.red > GAMMA_ZERO &&
	(pMonitor->gamma.red < GAMMA_MIN ||
	 pMonitor->gamma.red > GAMMA_MAX)) {
	badgamma = pMonitor->gamma.red;
    } else if (pMonitor->gamma.green > GAMMA_ZERO &&
	(pMonitor->gamma.green < GAMMA_MIN ||
	 pMonitor->gamma.green > GAMMA_MAX)) {
	badgamma = pMonitor->gamma.green;
    } else if (pMonitor->gamma.blue > GAMMA_ZERO &&
	(pMonitor->gamma.blue < GAMMA_MIN ||
	 pMonitor->gamma.blue > GAMMA_MAX)) {
	badgamma = pMonitor->gamma.blue;
    }
    if (badgamma > GAMMA_ZERO) {
	xf86ConfigError("Gamma value %.f is out of range (%.2f - %.1f)\n",
			badgamma, GAMMA_MIN, GAMMA_MAX);
	xf86ConfFreeMonitorData(pMonitor);
	return NULL;
    }

    /* Count the UseModes references. */
    count = 0;
    modesLink = confMonitor->mon_modes_sect_lst;
    while (modesLink) {
	count++;
	modesLink = modesLink->list.next;
    }
    pMonitor->numModeSetNames = count;
    if (count > 0) {
	pMonitor->modeSetNames = xcalloc(count + 1, sizeof(char *));
	if (!pMonitor->modeSetNames) {
	    xf86ConfFreeMonitorData(pMonitor);
	    return NULL;
	}
	modesLink = confMonitor->mon_modes_sect_lst;
	count = 0;
	while (modesLink) {
	    pMonitor->modeSetNames[count] = xstrdup(modesLink->ml_modes_str);
	    if (modesLink->ml_modes_str && !pMonitor->modeSetNames[count]) {
		xf86ConfFreeMonitorData(pMonitor);
		return NULL;
	    }
	    count++;
	    modesLink = modesLink->list.next;
	}
    }

    cmodep = confMonitor->mon_modeline_lst;
    count = 0;
    while (cmodep) {
	count++;
	cmodep = cmodep->list.next;
    }
    if (count > 0) {
	pMonitor->Modes = xf86ConfAllocModeList(count);
	if (!pMonitor->Modes) {
	    xf86ConfFreeMonitorData(pMonitor);
	    return NULL;
	}
	cmodep = confMonitor->mon_modeline_lst;
	mode = pMonitor->Modes;
	while (cmodep && mode) {
	    if (!convertMode(mode, cmodep)) {
		xf86ConfFreeMonitorData(pMonitor);
		return NULL;
	    }
	    cmodep = cmodep->list.next;
	    mode = mode->next;
	}
    }
    if (pMonitor->Modes) {
	pMonitor->Last = pMonitor->Modes->prev;
	pMonitor->Modes->prev = NULL;
    }
    return pMonitor;
}

MonPtr
xf86ConfGetMonitorByName(ConfigHandle handle, const char *name, int depth)
{
    const XF86ConfigRec *pConfig;
    XF86ConfMonitorPtr m;
    MonPtr pMonitor = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (m = pConfig->conf_monitor_lst; m; m = m->list.next) {
	if (xf86nameCompare(m->mon_identifier, name) == 0) {
	    break;
	}
    }
    if (m) {
	pMonitor = xf86ConfAllocMonitor();
	if (pMonitor) {
	    if (!convertMonitor(pMonitor, m)) {
		xfree(pMonitor);
		return NULL;
	    }
	    if (depth > 0) {
		if (!xf86ConfResolveMonitor(handle, pMonitor)) {
		    xf86ConfFreeMonitorData(pMonitor);
		    xfree(pMonitor);
		    return NULL;
		}
	    }
	    return pMonitor;
	}
    }
    return NULL;
}

static const MonRec *
getNextActiveMonitor(ConfigDataHandle prev)
{
    const confScreenRec *pConfScreen = NULL;
    int i = 0;
    Bool found = FALSE;

    while ((pConfScreen = getNextActiveScreen(pConfScreen))) {
	for (i = 0; i < pConfScreen->numMonitors; i++) {
	    if (!prev) {
		found = TRUE;
		break;
	    } else if (pConfScreen->monitors[i]->handle == prev) {
		i++;
		found = TRUE;
		break;
	    }
	}
	if (found) {
	    if (i < pConfScreen->numMonitors)
		break;
	    else {
		prev = NULL;
		found = FALSE;
	    }
	}
    }
    if (found)
	return pConfScreen->monitors[i];
    else
	return NULL;
}

MonPtr
xf86ConfGetNextMonitor(ConfigHandle handle, ConfigDataHandle prevMonitorHandle,
		       int depth)
{
    const XF86ConfigRec *pConfig;
    XF86ConfMonitorPtr m;
    MonPtr pMonitor = NULL;

    if (handle == ACTIVE_CONFIG)
	return xf86ConfDupMonitor(getNextActiveMonitor(prevMonitorHandle),
				  depth);

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevMonitorHandle) {
	m = pConfig->conf_monitor_lst;
    } else {
	for (m = pConfig->conf_monitor_lst; m; m = m->list.next) {
	    if (m == prevMonitorHandle) {
		m = m->list.next;
		break;
	    }
	}
    }
    if (m) {
	pMonitor = xf86ConfAllocMonitor();
	if (pMonitor) {
	    if (!convertMonitor(pMonitor, m)) {
		xfree(pMonitor);
		return NULL;
	    }
	    if (depth > 0) {
		if (!xf86ConfResolveMonitor(handle, pMonitor)) {
		    xf86ConfFreeMonitorData(pMonitor);
		    xfree(pMonitor);
		    return NULL;
		}
	    }
	    return pMonitor;
	}
    }
    return NULL;
}

static int
lookupVisual(const char *visname)
{
    int i;

    if (!visname || !*visname)
	return -1;

    for (i = 0; i <= DirectColor; i++) {
	if (!xf86nameCompare(visname, xf86VisualNames[i]))
	    break;
    }

    if (i <= DirectColor)
	return i;
    return -1;
}

DispPtr
xf86ConfAllocDisplay()
{
    DispPtr pDisplay;
    rgb noColour = {(unsigned int)-1, (unsigned int)-1, (unsigned int)-1};

    pDisplay = xcalloc(1, sizeof(*pDisplay));
    pDisplay->monitorNum = -1;
    pDisplay->frameX0 = -1;
    pDisplay->frameY0 = -1;
    pDisplay->blackColour = noColour;
    pDisplay->whiteColour = noColour;
    pDisplay->defaultVisual = -1;
    return pDisplay;
}

void
xf86ConfFreeDisplayData(DispPtr pDisplay)
{
    int i;

    if (!pDisplay)
	return;

    if (pDisplay->modes) {
	for (i = 0; pDisplay->modes[i]; i++) {
	    xfree(pDisplay->modes[i]);
	    pDisplay->modes[i] = NULL;
	}
	xfree(pDisplay->modes);
	pDisplay->modes = NULL;
    }
    if (pDisplay->options) {
	xf86OptionListFree(pDisplay->options);
	pDisplay->options = NULL;
    }
}

DispPtr
xf86ConfDupDisplay(const DispRec *pDisplay)
{
    DispPtr d;
    char **p;
    int i, count;

    if (!pDisplay)
	return NULL;

    d = xf86ConfAllocDisplay();
    if (!d)
	return NULL;

    *d = *pDisplay;
    d->options = NULL;
    d->modes = NULL; 
    d->options = xf86OptionListDup(pDisplay->options);
    if (pDisplay->options && !d->options) {
	xf86ConfFreeDisplayData(d);
	xfree(d);
	return NULL;
    }
    p = pDisplay->modes;
    count = 0;
    while (*p) {
	count++;
	p++;
    }
    d->modes = xcalloc(count + 1, sizeof(char*));
    if (!d->modes) {
	xf86ConfFreeDisplayData(d);
	xfree(d);
	return NULL;
    }
    for (i = 0; i < count; i++) {
	d->modes[i] = xstrdup(pDisplay->modes[i]);
	if (pDisplay->modes[i] && !d->modes[i]) {
	    xf86ConfFreeDisplayData(d);
	    xfree(d);
	    return NULL;
	}
    }
    return d;
}

static DispPtr
convertDisplay(DispPtr pDisplay, const XF86ConfDisplayRec *confDisplay)
{
    int count = 0;
    XF86ModePtr mode;
    
    if (!pDisplay || !confDisplay)
	return NULL;

    pDisplay->handle = confDisplay;
    pDisplay->frameX0 = confDisplay->disp_frameX0;
    pDisplay->frameY0 = confDisplay->disp_frameY0;
    pDisplay->virtualX = confDisplay->disp_virtualX;
    pDisplay->virtualY = confDisplay->disp_virtualY;
    pDisplay->depth = confDisplay->disp_depth;
    pDisplay->fbbpp = confDisplay->disp_bpp;
    pDisplay->weight.red = confDisplay->disp_weight.red;
    pDisplay->weight.green = confDisplay->disp_weight.green;
    pDisplay->weight.blue = confDisplay->disp_weight.blue;
    pDisplay->blackColour.red = confDisplay->disp_black.red;
    pDisplay->blackColour.green = confDisplay->disp_black.green;
    pDisplay->blackColour.blue  = confDisplay->disp_black.blue;
    pDisplay->whiteColour.red = confDisplay->disp_white.red;
    pDisplay->whiteColour.green = confDisplay->disp_white.green;
    pDisplay->whiteColour.blue = confDisplay->disp_white.blue;
    pDisplay->options = xf86OptionListDup(confDisplay->disp_option_lst);
    if (confDisplay->disp_option_lst && !pDisplay->options) {
	xf86ConfFreeDisplayData(pDisplay);
	return NULL;
    }
    pDisplay->monitorNum = confDisplay->monitor_num;
    if (confDisplay->disp_visual) {
	pDisplay->defaultVisual = lookupVisual(confDisplay->disp_visual);
	if (pDisplay->defaultVisual == -1) {
	    xf86ConfigError("Invalid visual name: \"%s\"",
			    confDisplay->disp_visual);
	    return FALSE;
	}
    } else {
	pDisplay->defaultVisual = -1;
    }

    /* Hook in the modes. */
    mode = confDisplay->disp_mode_lst;
    while (mode) {
	count++;
	mode = mode->list.next;
    }
    pDisplay->modes = xcalloc(count + 1, sizeof(char*));
    if (!pDisplay->modes) {
	xf86ConfFreeDisplayData(pDisplay);
	return NULL;
    }

    mode = confDisplay->disp_mode_lst;
    count = 0;
    while (mode) {
	pDisplay->modes[count] = xstrdup(mode->mode_name);
	if (mode->mode_name && !pDisplay->modes[count]) {
	    xf86ConfFreeDisplayData(pDisplay);
	    return NULL;
	}
	count++;
	mode = mode->list.next;
    }
    return pDisplay;
}

GDevPtr
xf86ConfAllocGraphicsDevice()
{
    GDevPtr pGDev;

    pGDev = xcalloc(1, sizeof(*pGDev));
    pGDev->chipID = -1;
    pGDev->chipRev = -1;
    pGDev->screen = -1;
    pGDev->handle = pGDev;
    return pGDev;
}

void
xf86ConfFreeGraphicsDeviceData(GDevPtr pGDev)
{
    if (!pGDev)
	return;

    if (pGDev->identifier) {
	xfree(pGDev->identifier);
	pGDev->identifier = NULL;
    }
    if (pGDev->vendor) {
	xfree(pGDev->vendor);
	pGDev->vendor = NULL;
    }
    if (pGDev->board) {
	xfree(pGDev->board);
	pGDev->board = NULL;
    }
    if (pGDev->chipset) {
	xfree(pGDev->chipset);
	pGDev->chipset = NULL;
    }
    if (pGDev->ramdac) {
	xfree(pGDev->ramdac);
	pGDev->ramdac = NULL;
    }
    if (pGDev->driver) {
	xfree(pGDev->driver);
	pGDev->driver = NULL;
    }
    if (pGDev->clockchip) {
	xfree(pGDev->clockchip);
	pGDev->clockchip = NULL;
    }
    if (pGDev->busID) {
	xfree(pGDev->busID);
	pGDev->busID = NULL;
    }
    if (pGDev->options) {
	xf86OptionListFree(pGDev->options);
	pGDev->options = NULL;
    }
}

GDevPtr
xf86ConfDupGraphicsDevice(const GDevRec *pGDev)
{
    GDevPtr g;

    if (!pGDev)
	return NULL;

    g = xf86ConfAllocGraphicsDevice();
    if (!g)
	return NULL;

    g->handle = pGDev->handle;
    g->identifier = xstrdup(pGDev->identifier);
    if (pGDev->identifier && !g->identifier) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->vendor = xstrdup(pGDev->vendor);
    if (pGDev->vendor && !g->vendor) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->board = xstrdup(pGDev->board);
    if (pGDev->board && !g->board) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->chipset = xstrdup(pGDev->chipset);
    if (pGDev->chipset && !g->chipset) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->ramdac = xstrdup(pGDev->ramdac);
    if (pGDev->ramdac && !g->ramdac) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->driver = xstrdup(pGDev->driver);
    if (pGDev->driver && !g->driver) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->videoRam = pGDev->videoRam;
    g->BiosBase = pGDev->BiosBase;
    g->MemBase = pGDev->MemBase;
    g->IOBase = pGDev->IOBase;
    g->clockchip = xstrdup(pGDev->clockchip);
    if (pGDev->clockchip && !g->clockchip) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->busID = xstrdup(pGDev->busID);
    if (pGDev->busID && !g->busID) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->textClockFreq = pGDev->textClockFreq;
    g->chipID = pGDev->chipID;
    g->chipRev = pGDev->chipRev;
    g->options = xf86OptionListDup(pGDev->options);
    if (pGDev->options && !g->options) {
	xf86ConfFreeGraphicsDeviceData(g);
	xfree(g);
	return NULL;
    }
    g->irq = pGDev->irq;
    g->screen = pGDev->screen;

    memcpy(g->dacSpeeds, pGDev->dacSpeeds, sizeof(g->dacSpeeds));
    g->numclocks = pGDev->numclocks;
    memcpy(g->clock, pGDev->clock, sizeof(g->clock));
    g->claimed = pGDev->claimed;
    return g;
}

static GDevPtr
convertGraphicsDevice(GDevPtr pGDev, const XF86ConfDeviceRec *confDevice)
{
    int i;

    if (!pGDev || !confDevice)
	return NULL;

    pGDev->handle = confDevice;
    pGDev->identifier = xstrdup(confDevice->dev_identifier);
    if (confDevice->dev_identifier && !pGDev->identifier) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->vendor = xstrdup(confDevice->dev_vendor);
    if (confDevice->dev_vendor && !pGDev->vendor) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->board = xstrdup(confDevice->dev_board);
    if (confDevice->dev_board && !pGDev->board) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->chipset = xstrdup(confDevice->dev_chipset);
    if (confDevice->dev_chipset && !pGDev->chipset) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->ramdac = xstrdup(confDevice->dev_ramdac);
    if (confDevice->dev_ramdac && !pGDev->ramdac) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->driver = xstrdup(confDevice->dev_driver);
    if (confDevice->dev_driver && !pGDev->driver) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->videoRam = confDevice->dev_videoram;
    pGDev->BiosBase = confDevice->dev_bios_base;
    pGDev->MemBase = confDevice->dev_mem_base;
    pGDev->IOBase = confDevice->dev_io_base;
    pGDev->clockchip = xstrdup(confDevice->dev_clockchip);
    if (confDevice->dev_clockchip && !pGDev->clockchip) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->busID = xstrdup(confDevice->dev_busid);
    if (confDevice->dev_busid && !pGDev->busID) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->textClockFreq = confDevice->dev_textclockfreq;
    pGDev->chipID = confDevice->dev_chipid;
    pGDev->chipRev = confDevice->dev_chiprev;
    pGDev->options = xf86OptionListDup(confDevice->dev_option_lst);
    if (confDevice->dev_option_lst && !pGDev->options) {
	xf86ConfFreeGraphicsDeviceData(pGDev);
	return NULL;
    }
    pGDev->irq = confDevice->dev_irq;
    pGDev->screen = confDevice->dev_screen;

    for (i = 0; i < MAXDACSPEEDS; i++) {
	if (i < CONF_MAXDACSPEEDS)
	    pGDev->dacSpeeds[i] = confDevice->dev_dacSpeeds[i];
	else
	    pGDev->dacSpeeds[i] = 0;
    }
    pGDev->numclocks = confDevice->dev_clocks;
    if (pGDev->numclocks > MAXCLOCKS)
	pGDev->numclocks = MAXCLOCKS;
    for (i = 0; i < pGDev->numclocks; i++) {
	pGDev->clock[i] = confDevice->dev_clock[i];
    }
    pGDev->claimed = FALSE;
    return pGDev;
}

GDevPtr
xf86ConfGetGraphicsDeviceByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    XF86ConfDevicePtr d;
    GDevPtr pGDev = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (d = pConfig->conf_device_lst; d; d = d->list.next) {
	if (xf86nameCompare(d->dev_identifier, name) == 0) {
	    break;
	}
    }
    if (d) {
	pGDev = xf86ConfAllocGraphicsDevice();
	if (pGDev) {
	    if (convertGraphicsDevice(pGDev, d))
		return pGDev;
	    else
		xfree(pGDev);
	}
    }
    return NULL;
}

static const GDevRec *
getNextActiveGraphicsDevice(ConfigDataHandle prev)
{
    const confScreenRec *pConfScreen = NULL;
    Bool found = FALSE;

    while ((pConfScreen = getNextActiveScreen(pConfScreen))) {
	if (!prev) {
	    found = TRUE;
	    break;
	} else if (pConfScreen->device->handle == prev) {
	    pConfScreen = getNextActiveScreen(pConfScreen);
	    found = TRUE;;
	    break;
	}
    }
    if (found && pConfScreen)
	return pConfScreen->device;
    else
	return NULL;
}

GDevPtr
xf86ConfGetNextGraphicsDevice(ConfigHandle handle,
			      ConfigDataHandle prevDeviceHandle)
{
    const XF86ConfigRec *pConfig;
    XF86ConfDevicePtr d;
    GDevPtr pGDev = NULL;

    if (handle == ACTIVE_CONFIG)
	return xf86ConfDupGraphicsDevice(
			getNextActiveGraphicsDevice(prevDeviceHandle));

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevDeviceHandle) {
	d = pConfig->conf_device_lst;
    } else {
	for (d = pConfig->conf_device_lst; d; d = d->list.next) {
	    if (d == prevDeviceHandle) {
		d = d->list.next;
		break;
	    }
	}
    }
    if (d) {
	pGDev = xf86ConfAllocGraphicsDevice();
	if (pGDev) {
	    if (convertGraphicsDevice(pGDev, d))
		return pGDev;
	    else
		xfree(pGDev);
	}
    }
    return NULL;
}

confDRIBufferPtr
xf86ConfAllocDRIBuffer()
{
    confDRIBufferPtr pDriBuf;

    pDriBuf = xcalloc(1, sizeof(*pDriBuf));
    return pDriBuf;
}

void
xf86ConfFreeDRIBufferData(confDRIBufferPtr pDriBuf)
{
    if (!pDriBuf)
	return;
}

confDRIBufferPtr
xf86ConfDupDRIBuffer(const confDRIBufferRec *pDriBuf)
{
    confDRIBufferPtr b;

    if (!pDriBuf)
	return NULL;

    b = xf86ConfAllocDRIBuffer();
    b->count = pDriBuf->count;
    b->size = pDriBuf->size;
    b->flags = pDriBuf->flags;
    return b;
}

confDRIPtr
xf86ConfAllocDRI()
{
    confDRIPtr pDri;

    pDri = xcalloc(1, sizeof(*pDri));
    pDri->group = -1;
    pDri->mode = 0;
    pDri->handle = pDri;
    return pDri;
}

void
xf86ConfFreeDRIData(confDRIPtr pDri)
{
    int i;

    if (!pDri)
	return;

    if (pDri->id) {
	xfree(pDri->id);
	pDri->id = NULL;
    }
    if (pDri->groupName) {
	xfree(pDri->groupName);
	pDri->groupName = NULL;
    }
    if (pDri->options) {
	xf86OptionListFree(pDri->options);
	pDri->options = NULL;
    }
    if (pDri->bufsList) {
	for (i = 0; i < pDri->bufs_count; i++) {
	    xf86ConfFreeDRIBufferData(pDri->bufsList[i]);
	    xfree(pDri->bufsList[i]);
	    pDri->bufsList[i] = NULL;
	}
	xfree(pDri->bufsList);
	pDri->bufsList = NULL;
    }
    pDri->bufs_count = 0;
}

confDRIPtr
xf86ConfDupDRI(const confDRIRec *pDri)
{
    confDRIPtr d;
    int i;

    if (!pDri)
	return NULL;

    d = xf86ConfAllocDRI();
    if (!d)
	return NULL;

    d->handle = pDri->handle;
    d->id = xstrdup(pDri->id);
    if (pDri->id && !d->id) {
	xf86ConfFreeDRIData(d);
	xfree(d);
	return NULL;
    }
    d->options = xf86OptionListDup(pDri->options);
    if (pDri->options && !d->options) {
	xf86ConfFreeDRIData(d);
	xfree(d);
	return NULL;
    }
    d->groupName = xstrdup(pDri->groupName);
    if (pDri->groupName && !d->groupName) {
	xf86ConfFreeDRIData(d);
	xfree(d);
	return NULL;
    }
    d->group = pDri->group;
    d->mode = pDri->mode;
    d->bufs_count = pDri->bufs_count;
    if (d->bufs_count > 0 && pDri->bufsList) {
	d->bufsList = xcalloc(d->bufs_count, sizeof(confDRIBufferPtr));
	if (!d->bufsList) {
	    xf86ConfFreeDRIData(d);
	    xfree(d);
	    return NULL;
	}
	for (i = 0; i < d->bufs_count; i++) {
	    d->bufsList[i] = xf86ConfDupDRIBuffer(pDri->bufsList[i]);
	    if (!d->bufsList[i]) {
		xf86ConfFreeDRIData(d);
		xfree(d);
		return NULL;
	    }
	}
    }
    return d;
}

static confDRIBufferPtr
convertDRIBuffer(confDRIBufferPtr pDriBuf, const XF86ConfBuffersRec *bufConf)
{
    if (!pDriBuf || !bufConf)
	return NULL;

    pDriBuf->count = bufConf->buf_count;
    pDriBuf->size = bufConf->buf_size;
    /*
     * FIXME: Flags not implemented.  These could be used, for example,
     * to specify a contiguous block and/or write-combining cache policy.
     */
    pDriBuf->flags = XF86DRI_NO_FLAGS;
    return pDriBuf;
}

static confDRIPtr
convertDRI(confDRIPtr pDri, const XF86ConfDRIRec *driConf)
{
    struct group *groupId;
    const XF86ConfBuffersRec *db;
    int count, i;

    if (!pDri || !driConf)
	return NULL;

    pDri->handle = driConf;
    pDri->id = xstrdup(driConf->dri_identifier);
    if (driConf->dri_identifier && !pDri->id) {
	xf86ConfFreeDRIData(pDri);
	return NULL;
    }
    pDri->options = xf86OptionListDup(driConf->dri_option_lst);
    if (driConf->dri_option_lst && !pDri->options) {
	xf86ConfFreeDRIData(pDri);
	return NULL;
    }
    if (driConf->dri_group_name) {
	pDri->groupName = xstrdup(driConf->dri_group_name);
	if (!pDri->groupName) {
	    xf86ConfFreeDRIData(pDri);
	    return NULL;
	}
	if ((groupId = getgrnam(driConf->dri_group_name)))
	    pDri->group = groupId->gr_gid;
    } else {
	if (driConf->dri_group >= 0)
	    pDri->group = driConf->dri_group;
    }
    pDri->mode = driConf->dri_mode;

    /* Count how many buffers references there are. */
    count = 0;
    db = driConf->dri_buffers_lst;
    while (db) {
	count++;
	db = db->list.next;
    }

    pDri->bufs_count = count;
    if (count > 0) {
	pDri->bufsList = xcalloc(1, sizeof(confDRIBufferPtr));
	if (!pDri->bufsList) {
	    xf86ConfFreeDRIData(pDri);
	    return NULL;
	}
	db = driConf->dri_buffers_lst;
	for (i = 0; i < count; i++) {
	    pDri->bufsList[i] = xf86ConfAllocDRIBuffer();
	    if (!convertDRIBuffer(pDri->bufsList[i], db)) {
		xf86ConfFreeDRIData(pDri);
		return NULL;
	    }
	}
    }
    return pDri;
}

confDRIPtr
xf86ConfGetDRIByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    XF86ConfDRIPtr d;
    confDRIPtr pDri = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (d = pConfig->conf_dri_lst; d; d = d->list.next) {
	if (xf86nameCompare(d->dri_identifier, name) == 0)
	    break;
    }
    if (d) {
	pDri = xf86ConfAllocDRI();
	if (pDri) {
	    if (convertDRI(pDri, d))
		return pDri;
	    else
		xfree(pDri);
	}
    }
    return NULL;
}

confDRIPtr
xf86ConfGetNextDRI(ConfigHandle handle, ConfigDataHandle prevDRIHandle)
{
    const XF86ConfigRec *pConfig;
    XF86ConfDRIPtr d;
    confDRIPtr pDri = NULL;

    if (handle == ACTIVE_CONFIG) {
	if (!prevDRIHandle)
	    return xf86ConfDupDRI(&xf86ConfigDRI);
	else
	    return NULL;
    }

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevDRIHandle) {
	d = pConfig->conf_dri_lst;
    } else {
	for (d = pConfig->conf_dri_lst; d; d = d->list.next) {
	    if (d == prevDRIHandle) {
		d = d->list.next;
		break;
	    }
	}
    }
    if (d) {
	pDri = xf86ConfAllocDRI();
	if (pDri) {
	    if (convertDRI(pDri, d))
		return pDri;
	    else
		xfree(pDri);
	}
    }
    return NULL;
}

static confDRIPtr
configDRI(ConfigHandle handle, const XF86ConfDRIRec *driConf, MessageType from)
{
    confDRIPtr d = NULL;

    /* If driConf is supplied, us it.  If not, use the first DRI section. */
    if (driConf) {
	d = xf86ConfAllocDRI();
	if (!d)
	    return NULL;
	if (!(d = convertDRI(d, driConf))) {
	    xfree(d);
	    return NULL;
	}
    } else {
	d = xf86ConfGetNextDRI(handle, NULL);
    }
    /* If no DRI section found, create the default. */
    if (!d) {
	d = xf86ConfAllocDRI();
    }
    return d;
}

IDevPtr
xf86ConfAllocInputDevice()
{
    IDevPtr pIDev;

    pIDev = xcalloc(1, sizeof(*pIDev));
    pIDev->handle = pIDev;
    pIDev->from = X_NONE;
    return pIDev;
}

void
xf86ConfFreeInputDeviceData(IDevPtr pIDev)
{
    if (!pIDev)
	return;

    if (pIDev->identifier) {
	xfree(pIDev->identifier);
	pIDev->identifier = NULL;
    }
    if (pIDev->driver) {
	xfree(pIDev->driver);
	pIDev->driver = NULL;
    }
    if (pIDev->commonOptions) {
	xf86OptionListFree(pIDev->commonOptions);
	pIDev->commonOptions = NULL;
    }
    if (pIDev->extraOptions) {
	xf86OptionListFree(pIDev->extraOptions);
	pIDev->extraOptions = NULL;
    }
    return;
}

IDevPtr
xf86ConfDupInputDevice(const IDevRec *pIDev, int depth)
{
    IDevPtr i;

    if (!pIDev)
	return NULL;

    i = xf86ConfAllocInputDevice();
    if (!i)
	return NULL;

    i->handle = pIDev->handle;
    i->from = pIDev->from;
    i->identifier = xstrdup(pIDev->identifier);
    if (pIDev->identifier && !i->identifier) {
	xf86ConfFreeInputDeviceData(i);
	xfree(i);
	return NULL;
    }
    i->driver = xstrdup(pIDev->driver);
    if (pIDev->driver && !i->driver) {
	xf86ConfFreeInputDeviceData(i);
	xfree(i);
	return NULL;
    }
    i->commonOptions = xf86OptionListDup(pIDev->commonOptions);
    if (pIDev->commonOptions && !i->commonOptions) {
	xf86ConfFreeInputDeviceData(i);
	xfree(i);
	return NULL;
    }
    if (depth > 0) {
	i->extraOptions = xf86OptionListDup(pIDev->extraOptions);
	if (pIDev->extraOptions && !i->extraOptions) {
	    xf86ConfFreeInputDeviceData(i);
	    xfree(i);
	    return NULL;
	}
    }
    return i;
}

static IDevPtr
convertInputDevice(IDevPtr pIDev, const XF86ConfInputRec *confInput)
{
    if (!pIDev || !confInput)
	return NULL;

    pIDev->handle = confInput;
    pIDev->from = X_CONFIG;
    pIDev->identifier = xstrdup(confInput->inp_identifier);
    if (confInput->inp_identifier && !pIDev->identifier) {
	xf86ConfFreeInputDeviceData(pIDev);
	return NULL;
    }
    pIDev->driver = xstrdup(confInput->inp_driver);
    if (confInput->inp_driver && !pIDev->driver) {
	xf86ConfFreeInputDeviceData(pIDev);
	return NULL;
    }
    pIDev->commonOptions = xf86OptionListDup(confInput->inp_option_lst);
    if (confInput->inp_option_lst && !pIDev->commonOptions) {
	xf86ConfFreeInputDeviceData(pIDev);
	return NULL;
    }
    pIDev->extraOptions = NULL;
    return pIDev;
}

IDevPtr
xf86ConfGetInputDeviceByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    XF86ConfInputPtr i;
    IDevPtr pIDev = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (i = pConfig->conf_input_lst; i; i = i->list.next) {
	if (xf86nameCompare(i->inp_identifier, name) == 0) {
	    break;
	}
    }
    if (i) {
	pIDev = xf86ConfAllocInputDevice();
	if (pIDev) {
	    if (convertInputDevice(pIDev, i))
		return pIDev;
	    else
		xfree(pIDev);
	}
    }
    return NULL;
}

static const IDevRec *
getNextActiveInputDevice(ConfigDataHandle prev)
{
    int i;
    Bool found = FALSE;

    for (i = 0; i < xf86Info.serverLayout->numInputs; i++) {
	if (!prev) {
	    found = TRUE;
	    break;
	} else if (xf86Info.serverLayout->inputDevs[i]->handle == prev) {
	    i++;
	    found = TRUE;
	    break;
	}
    }
    if (found && i < xf86Info.serverLayout->numInputs)
	return xf86Info.serverLayout->inputDevs[i];
    else
	return NULL;
}

IDevPtr
xf86ConfGetNextInputDevice(ConfigHandle handle, ConfigDataHandle prevIDevHandle,
			   int depth)
{
    const XF86ConfigRec *pConfig;
    XF86ConfInputPtr i;
    IDevPtr pIDev = NULL;

    if (handle == ACTIVE_CONFIG)
	return xf86ConfDupInputDevice(getNextActiveInputDevice(prevIDevHandle),
				      depth);

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevIDevHandle) {
	i = pConfig->conf_input_lst;
    } else {
	for (i = pConfig->conf_input_lst; i; i = i->list.next) {
	    if (i == prevIDevHandle) {
		i = i->list.next;
		break;
	    }
	}
    }
    if (i) {
	pIDev = xf86ConfAllocInputDevice();
	if (pIDev) {
	    if (convertInputDevice(pIDev, i))
		return pIDev;
	    else
		xfree(pIDev);
	}
    }
    return NULL;
}

IDevPtr
xf86ConfGetInputDeviceByDriver(ConfigHandle handle, const char *driver)
{
    const XF86ConfigRec *pConfig;
    XF86ConfInputPtr i;
    IDevPtr pIDev = NULL;

    if (!driver)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (i = pConfig->conf_input_lst; i; i = i->list.next) {
	if (xf86nameCompare(i->inp_driver, driver) == 0) {
	    break;
	}
    }
    if (i) {
	pIDev = xf86ConfAllocInputDevice();
	if (pIDev) {
	    if (convertInputDevice(pIDev, i))
		return pIDev;
	    else
		xfree(pIDev);
	}
    }
    return NULL;
}

IDevPtr
xf86ConfGetInputDeviceByOption(ConfigHandle handle, const char *option)
{
    const XF86ConfigRec *pConfig;
    XF86ConfInputPtr i;
    IDevPtr pIDev = NULL;

    if (!option)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (i = pConfig->conf_input_lst; i; i = i->list.next) {
	if (xf86CheckBoolOption(i->inp_option_lst, option, FALSE)) {
	    break;
	}
    }
    if (i) {
	pIDev = xf86ConfAllocInputDevice();
	if (pIDev) {
	    if (convertInputDevice(pIDev, i))
		return pIDev;
	    else
		xfree(pIDev);
	}
    }
    return NULL;
}

confVendorSubPtr
xf86ConfAllocVendorSub()
{
    confVendorSubPtr pVendorSub;

    pVendorSub = xcalloc(1, sizeof(*pVendorSub));
    pVendorSub->handle = pVendorSub;
    return pVendorSub;
}

void
xf86ConfFreeVendorSubData(confVendorSubPtr pVendorSub)
{
    if (!pVendorSub)
	return;

    if (pVendorSub->identifier) {
	xfree(pVendorSub->identifier);
	pVendorSub->identifier = NULL;
    }
    if (pVendorSub->name) {
	xfree(pVendorSub->name);
	pVendorSub->name = NULL;
    }
    if (pVendorSub->options) {
	xf86OptionListFree(pVendorSub->options);
	pVendorSub->options = NULL;
    }
}

confVendorSubPtr
xf86ConfDupVendorSub(const confVendorSubRec *pVendorSub)
{
    confVendorSubPtr sv;

    if (!pVendorSub)
	return NULL;

    sv = xf86ConfAllocVendorSub();
    if (!sv)
	return NULL;

    sv->handle = pVendorSub->handle;
    sv->identifier = xstrdup(pVendorSub->identifier);
    if (pVendorSub->identifier && !sv->identifier) {
	xf86ConfFreeVendorSubData(sv);
	xfree(sv);
	return NULL;
    }
    sv->name = xstrdup(pVendorSub->name);
    if (pVendorSub->name && !sv->name) {
	xf86ConfFreeVendorSubData(sv);
	xfree(sv);
	return NULL;
    }
    sv->options = xf86OptionListDup(pVendorSub->options);
    if (pVendorSub->options && !sv->options) {
	xf86ConfFreeVendorSubData(sv);
	xfree(sv);
	return NULL;
    }
    return sv;
}

/*
 * Convert a Vendor subsection from the parser's format to the server's format.
 */
static confVendorSubPtr
convertVendorSub(confVendorSubPtr pVendorSub,
		 const XF86ConfVendSubRec *vendorSubConf)
{
    if (!pVendorSub || !vendorSubConf)
	return NULL;

    pVendorSub->handle = vendorSubConf;
    pVendorSub->identifier = xstrdup(vendorSubConf->vs_identifier);
    if (vendorSubConf->vs_identifier && !pVendorSub->identifier) {
	xf86ConfFreeVendorSubData(pVendorSub);
	return NULL;
    }
    pVendorSub->name = xstrdup(vendorSubConf->vs_name);
    if (vendorSubConf->vs_name && !pVendorSub->name) {
	xf86ConfFreeVendorSubData(pVendorSub);
	return NULL;
    }
    pVendorSub->options = xf86OptionListDup(vendorSubConf->vs_option_lst);
    if (vendorSubConf->vs_option_lst && !pVendorSub->options) {
	xf86ConfFreeVendorSubData(pVendorSub);
	return NULL;
    }
    return pVendorSub;
}

confVendorPtr
xf86ConfAllocVendor()
{
    confVendorPtr pVendor;

    pVendor = xcalloc(1, sizeof(*pVendor));
    pVendor->handle = pVendor;
    return pVendor;
}

void
xf86ConfFreeVendorData(confVendorPtr pVendor)
{
    int i;

    if (!pVendor)
	return;

    if (pVendor->identifier) {
	xfree(pVendor->identifier);
	pVendor->identifier = NULL;
    }
    if (pVendor->vendorName) {
	xfree(pVendor->vendorName);
	pVendor->vendorName = NULL;
    }
    if (pVendor->options) {
	xf86OptionListFree(pVendor->options);
	pVendor->options = NULL;
    }
    if (pVendor->subs) {
	for (i = 0; i < pVendor->numSubs; i++) {
	    xf86ConfFreeVendorSubData(pVendor->subs[i]);
	    xfree(pVendor->subs[i]);
	    pVendor->subs[i] = NULL;
	}
	xfree(pVendor->subs);
	pVendor->subs = NULL;
    }
    pVendor->numSubs = 0;
}

confVendorPtr
xf86ConfDupVendor(const confVendorRec *pVendor)
{
    confVendorPtr v;
    int i;

    if (!pVendor)
	return NULL;

    v = xf86ConfAllocVendor();
    if (!v)
	return NULL;

    v->handle = pVendor->handle;
    v->identifier = xstrdup(pVendor->identifier);
    if (pVendor->identifier && !v->identifier) {
	xf86ConfFreeVendorData(v);
	xfree(v);
	return NULL;
    }
    v->vendorName = xstrdup(pVendor->vendorName);
    if (pVendor->vendorName && !v->vendorName) {
	xf86ConfFreeVendorData(v);
	xfree(v);
	return NULL;
    }
    v->options = xf86OptionListDup(pVendor->options);
    if (pVendor->options && !v->options) {
	xf86ConfFreeVendorData(v);
	xfree(v);
	return NULL;
    }

    v->numSubs = pVendor->numSubs;
    if (v->numSubs && pVendor->subs) {
	v->subs = xcalloc(v->numSubs, sizeof(confVendorSubPtr));
	if (!v->subs) {
	    xf86ConfFreeVendorData(v);
	    xfree(v);
	    return NULL;
	}
	for (i = 0; i < v->numSubs; i++) {
	    v->subs[i] = xf86ConfDupVendorSub(pVendor->subs[i]);
	    if (!v->subs[i]) {
		xf86ConfFreeVendorData(v);
		xfree(v);
		return NULL;
	    }
	}
    }
    return v;
}

/*
 * Convert a Vendor section from the parser's format to the server's format.
 */
static confVendorPtr
convertVendor(confVendorPtr pVendor, const XF86ConfVendorRec *vendorConf)
{
    XF86ConfVendSubPtr vsp;
    int count, i;

    if (!pVendor || !vendorConf)
	return NULL;

    pVendor->handle = vendorConf;
    pVendor->identifier = xstrdup(vendorConf->vnd_identifier);
    if (vendorConf->vnd_identifier && !pVendor->identifier) {
	xf86ConfFreeVendorData(pVendor);
	return NULL;
    }
    pVendor->vendorName = xstrdup(vendorConf->vnd_name);
    if (vendorConf->vnd_name && !pVendor->vendorName) {
	xf86ConfFreeVendorData(pVendor);
	return NULL;
    }
    pVendor->options = xf86OptionListDup(vendorConf->vnd_option_lst);
    if (vendorConf->vnd_option_lst && !pVendor->options) {
	xf86ConfFreeVendorData(pVendor);
	return NULL;
    }

    /* Count how many vendor subsections there are. */
    vsp = vendorConf->vnd_sub_lst;
    count = 0;
    while (vsp) {
	count++;
	vsp = vsp->list.next;
    }
    pVendor->numSubs = count;
    if (count > 0) {
	pVendor->subs = xcalloc(count, sizeof(confVendorSubPtr));
	if (!pVendor->subs) {
	    xf86ConfFreeVendorData(pVendor);
	    return NULL;
	}
	vsp = vendorConf->vnd_sub_lst;
	for (i = 0; i < count; i++) {
	    pVendor->subs[i] = xf86ConfAllocVendorSub();
	    if (!pVendor->subs[i]) {
		xf86ConfFreeVendorData(pVendor);
		return NULL;
	    }
	    if (!convertVendorSub(pVendor->subs[i], vsp)) {
		xf86ConfFreeVendorData(pVendor);
		return NULL;
	    }
	    vsp = vsp->list.next;
	}
    }
    return pVendor;
}

confVendorPtr
xf86ConfGetVendorByName(ConfigHandle handle, const char *name)
{
    const XF86ConfigRec *pConfig;
    XF86ConfVendorPtr v;
    confVendorPtr pVendor = NULL;

    if (!name)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (v = pConfig->conf_vendor_lst; v; v = v->list.next) {
	if (xf86nameCompare(v->vnd_identifier, name) == 0)
	    break;
    }
    if (v) {
	pVendor = xf86ConfAllocVendor();
	if (pVendor) {
	    if (convertVendor(pVendor, v))
		return pVendor;
	    else
		xfree(pVendor);
	}
    }
    return NULL;
}

confVendorPtr
xf86ConfGetVendorByVendorName(ConfigHandle handle, const char *vname)
{
    const XF86ConfigRec *pConfig;
    XF86ConfVendorPtr v;
    confVendorPtr pVendor = NULL;

    if (!vname)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    for (v = pConfig->conf_vendor_lst; v; v = v->list.next) {
	if (xf86nameCompare(v->vnd_name, vname) == 0)
	    break;
    }
    if (v) {
	pVendor = xf86ConfAllocVendor();
	if (pVendor) {
	    if (convertVendor(pVendor, v))
		return pVendor;
	    else
		xfree(pVendor);
	}
    }
    return NULL;
}

confVendorPtr
xf86ConfGetNextVendor(ConfigHandle handle, ConfigDataHandle prevVendorHandle)
{
    const XF86ConfigRec *pConfig;
    XF86ConfVendorPtr v;
    confVendorPtr pVendor = NULL;

    if (handle == ACTIVE_CONFIG)
	return NULL;

    pConfig = ConfigHandleToConfigRead(handle);
    if (!prevVendorHandle) {
	v = pConfig->conf_vendor_lst;
    } else {
	for (v = pConfig->conf_vendor_lst; v; v = v->list.next) {
	    if (v == prevVendorHandle) {
		v = v->list.next;
		break;
	    }
	}
    }
    if (v) {
	pVendor = xf86ConfAllocVendor();
	if (pVendor) {
	    if (convertVendor(pVendor, v))
		return pVendor;
	    else
		xfree(pVendor);
	}
    }
    return NULL;
}

static Bool
addDefaultModes(MonPtr pMonitor)
{
    DisplayModePtr mode;
    int i = 0;

    while (xf86DefaultModes[i].name != NULL) {
	if (!xf86ModeIsPresent(xf86DefaultModes[i].name, pMonitor->Modes,
			       0, M_T_DEFAULT)) {
	    do {
		mode = xnfalloc(sizeof(DisplayModeRec));
		memcpy(mode, &xf86DefaultModes[i], sizeof(DisplayModeRec));
		if (xf86DefaultModes[i].name)
		    mode->name = xnfstrdup(xf86DefaultModes[i].name);
		xf86AddModeToMonitor(pMonitor, mode);
		i++;
	    } while((xf86DefaultModes[i].name != NULL) &&
		    (!strcmp(xf86DefaultModes[i].name,
			     xf86DefaultModes[i-1].name)));
	} else
	    i++;
    }
    return TRUE;
}

/* Load a config file. */

ConfigStatus
xf86LoadConfigFile(const char *filename, Bool append)
{
    char *searchpath;
    MessageType from = X_DEFAULT;
    XF86ConfigPtr pConfig;

    pConfig = (XF86ConfigPtr)xf86Info.config;
    /* Free any previously allocated config. */
    if (!append && pConfig) {
	xf86freeConfig(pConfig);
	xf86Info.config = pConfig = NULL;
    }

    if (getuid() == 0)
	searchpath = ROOT_CONFIGPATH;
    else
	searchpath = USER_CONFIGPATH;

    if (!filename) {
	if (xf86ConfigFile)
	    from = X_CMDLINE;

	filename = xf86openConfigFile(searchpath, xf86ConfigFile, PROJECTROOT);
	if (filename) {
	    xf86MsgVerb(from, 0, "Using config file: \"%s\".\n", filename);
	    xf86ConfigFile = xnfstrdup(filename);
	} else {
	    xf86Msg(X_ERROR, "Unable to locate/open config file.");
	    if (xf86ConfigFile)
		xf86ErrorFVerb(0, ": \"%s\"", xf86ConfigFile);
	    xf86ErrorFVerb(0, "\n");
	    return CONFIG_NOFILE;
	}
    }

    if (!(xf86Info.config = xf86parseConfigFile(pConfig))) {
	xf86Msg(X_ERROR, "Problem parsing the config file \"%s\".\n",
		filename);
	return CONFIG_PARSE_ERROR;
    }

    xf86closeConfigFile ();
    return CONFIG_OK;
}

Bool
xf86CheckForLayoutOrScreen(ConfigHandle handle)
{
    const XF86ConfigRec *pConfig;

    pConfig = ConfigHandleToConfigRead(handle);
    if (pConfig && (pConfig->conf_layout_lst || pConfig->conf_screen_lst)) {
	return TRUE;
    }
    return FALSE;
}

static void
printLayoutSummary(const serverLayoutRec *pServerLayout)
{
    int i, j;
    screenLayoutPtr pScreenLayout;
    IDevPtr pIDev;
    GDevPtr pGDev;

    if (!pServerLayout)
	return;

    xf86Msg(pServerLayout->from, "ServerLayout \"%s\"\n", pServerLayout->id);
    for (i = 0; i < pServerLayout->numScreens; i++) {
	if (!pServerLayout->screenLayouts[i])
	    continue;
	pScreenLayout = pServerLayout->screenLayouts[i];
	if (pScreenLayout->screen) {
	    char *position = NULL;
	    switch (pScreenLayout->where) {
	    case PosAbsolute:
		xasprintf(&position, "(%d, %d)", pScreenLayout->x,
			  pScreenLayout->y);
		break;
	    case PosRightOf:
		xasprintf(&position, "Right of \"%s\"", pScreenLayout->refname);
		break;
	    case PosLeftOf:
		xasprintf(&position, "Left of \"%s\"", pScreenLayout->refname);
		break;
	    case PosAbove:
		xasprintf(&position,"Above \"%s\"", pScreenLayout->refname);
		break;
	    case PosBelow:
		xasprintf(&position, "Below \"%s\"", pScreenLayout->refname);
		break;
	    case PosRelative:
		xasprintf(&position, "(%d, %d) relative to \"%s\"",
			  pScreenLayout->x, pScreenLayout->y,
			  pScreenLayout->refname);
		break;
	    default:
		position = NULL;
		break;
	    }
	    xf86Msg(xf86ScreenName ? X_CMDLINE : X_DEFAULT,
		    "|-->Screen \"%s\" (%d) %s\n", pScreenLayout->screen->id,
		    pScreenLayout->screen->screennum, EMPTYIFNULL(position));
	    if (position)
		xfree(position);
	    if (pScreenLayout->screen->device) {
		xf86Msg(X_CONFIG, "|  |-->Device \"%s\"\n",
			pScreenLayout->screen->device->identifier);
	    }
	    if (pScreenLayout->screen->monitors) {
		for (j = 0; j < pScreenLayout->screen->numMonitors; j++) {
		    if (pScreenLayout->screen->monitors[j]) {
			xf86Msg(pScreenLayout->screen->monitors[j]->defaultMon ?
				    X_DEFAULT : X_CONFIG,
				"|  |-->Monitor \"%s\" (%d)\n",
				pScreenLayout->screen->monitors[j]->id,
				pScreenLayout->screen->monitors[j]->monitorNum);
		    }
		}
	    }
	    if (pScreenLayout->screen->xvAdaptorList) {
		for (j = 0; j < pScreenLayout->screen->numXvAdaptors; j++) {
		    xf86Msg(X_CONFIG, "|  |-->VideoAdaptor \"%s\"\n",
			pScreenLayout->screen->xvAdaptorList[j]->identifier);
		}
	    }
	}
    }
    if (pServerLayout->inputDevs) {
	for (i = 0; i < pServerLayout->numInputs; i++) {
	    pIDev = pServerLayout->inputDevs[i];
	    if (pIDev) {
		xf86Msg(pIDev->from, "|-->Input Device \"%s\"\n",
			pIDev->identifier);
	    }
	}
    }
    if (pServerLayout->inactiveDevs) {
	for (i = 0; i < pServerLayout->numInactives; i++) {
	    pGDev = pServerLayout->inactiveDevs[i];
	    if (pGDev) {
	    xf86Msg(X_CONFIG, "|-->Inactive Device \"%s\"\n",
		    pGDev->identifier);
	    }
	}
    }
}

/*
 * Process the configuration data from the parser.
 */
ConfigStatus
xf86ProcessConfiguration()
{
    const XF86ConfigRec *pConfig;
    confDRIPtr dri;
    int i;

    pConfig = ConfigHandleToConfigRead(DEFAULT_CONFIG);
    if (!pConfig) {
	xf86Msg(X_ERROR, "No configuration to process.\n");
	return CONFIG_PARSE_ERROR;
    }

    /*
     * Convert the information contained in the parser structures into our
     * own structures.
     */

    /* First check if a layout section is present, and if it is valid. */
    if (!pConfig->conf_layout_lst || xf86ScreenName) {
	if (xf86ScreenName == NULL) {
	    xf86Msg(X_WARNING,
		    "No Layout section.  Using the first Screen section.\n");
	}
	xf86Info.serverLayout =
	    configImpliedServerLayout(DEFAULT_CONFIG, xf86ScreenName);
    } else {
	char *defaultLayout = NULL;

	if (pConfig->conf_flags_lst) {
	    pointer optlist =
		pConfig->conf_flags_lst->flg_option_lst;
	    if (optlist && xf86FindOption(optlist, "defaultserverlayout")) {
		defaultLayout =
		    xf86SetStrOption(optlist, "defaultserverlayout", NULL);
	    }
	}
	xf86Info.serverLayout = configServerLayout(DEFAULT_CONFIG,
						   defaultLayout);
    }
    if (!xf86Info.serverLayout) {
	xf86Msg(X_ERROR, "Unable to determine the screen layout.\n");
	return CONFIG_PARSE_ERROR;
    }

    printLayoutSummary(xf86Info.serverLayout);

    i = xf86ConfCheckResolvedServerLayout(xf86Info.serverLayout,
					  CONFIG_RESOLVE_ALL, TRUE);
    if (i > 0) {
	xf86Msg(X_ERROR, "%d error%s in resolving the ServerLayout \"%s\".\n",
		i, PLURAL(i), xf86Info.serverLayout->id);
	return CONFIG_PARSE_ERROR;
    }
					
    /* Now process everything else */

    xf86Info.confFiles = configFiles(DEFAULT_CONFIG, NULL, X_CONFIG);

    if (!xf86Info.confFiles) {
	xf86Msg(X_ERROR, "Unable to process the Files data.\n");
	return CONFIG_PARSE_ERROR;
    }

    xf86Info.confFlags = configServerFlags(DEFAULT_CONFIG, NULL,
					   xf86Info.serverLayout->options,
					   X_CONFIG);
    if (!xf86Info.confFlags) {
	xf86Msg(X_ERROR, "Unable to process the ServerFlags data.\n");
	return CONFIG_PARSE_ERROR;
    }

    xf86Info.confModules = configModules(DEFAULT_CONFIG, NULL, X_CONFIG);

    if (!xf86Info.confModules) {
	xf86Msg(X_ERROR, "Unable to process the Modules data.\n");
	return CONFIG_PARSE_ERROR;
    }

    dri = configDRI(DEFAULT_CONFIG, NULL, X_CONFIG);
    if (!dri) {
	xf86Msg(X_ERROR, "Unable to process the DRI data.\n");
	return CONFIG_PARSE_ERROR;
    } else {
	xf86ConfigDRI = *dri;
	xfree(dri);
    }

    /*
     * Handle some command line options that can override some of the
     * ServerFlags settings.
     */
#ifdef XF86VIDMODE
    if (xf86VidModeDisabled)
	xf86Info.vidModeEnabled = FALSE;
    if (xf86VidModeAllowNonLocal)
	xf86Info.vidModeAllowNonLocal = TRUE;
#endif

#ifdef XF86MISC
    if (xf86MiscModInDevDisabled)
	xf86Info.miscModInDevEnabled = FALSE;
    if (xf86MiscModInDevAllowNonLocal)
	xf86Info.miscModInDevAllowNonLocal = TRUE;
#endif

    if (xf86AllowMouseOpenFail)
	xf86Info.allowMouseOpenFail = TRUE;
    return CONFIG_OK;
}


/* These make the equivalent parser functions visible to the common layer. */
Bool
xf86PathIsAbsolute(const char *path)
{
    return (xf86pathIsAbsolute(path) != 0);
}

Bool
xf86PathIsSafe(const char *path)
{
    return (xf86pathIsSafe(path) != 0);
}

