/*
 * SBUS and OpenPROM access functions.
 *
 * Copyright (C) 2000 Jakub Jelinek (jakub@redhat.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bus/Sbus.c,v 1.1 2001/04/20 17:02:43 tsi Exp $ */

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#include "xf86sbusBus.h"
#include "xf86Sbus.h"

static int promFd = -1;
static int promRootNode, promCurrentNode;
static int promOpenCount = 0;
static int promP1275 = -1;
#define MAX_PROP	128
#define MAX_VAL		(4096-128-4)
static struct openpromio *promOpio;

static int
promGetSibling(int node)
{
    promOpio->oprom_size = sizeof(int);

    if (node == -1) return 0;
    *(int *)promOpio->oprom_array = node;
    if (ioctl(promFd, OPROMNEXT, promOpio) < 0)
	return 0;
    promCurrentNode = *(int *)promOpio->oprom_array;
    return *(int *)promOpio->oprom_array;
}

static int
promGetChild(int node)
{
    promOpio->oprom_size = sizeof(int);

    if (!node || node == -1) return 0;
    *(int *)promOpio->oprom_array = node;
    if (ioctl(promFd, OPROMCHILD, promOpio) < 0)
	return 0;
    promCurrentNode = *(int *)promOpio->oprom_array;
    return *(int *)promOpio->oprom_array;
}

static char *
promGetProperty(const char *prop, int *lenp)
{
    promOpio->oprom_size = MAX_VAL;

    strcpy(promOpio->oprom_array, prop);
    if (ioctl(promFd, OPROMGETPROP, promOpio) < 0)
	return 0;
    if (lenp) *lenp = promOpio->oprom_size;
    return promOpio->oprom_array;
}

static int
promGetBool(const char *prop)
{
    promOpio->oprom_size = 0;

    *(int *)promOpio->oprom_array = 0;
    for (;;) {
	promOpio->oprom_size = MAX_PROP;
	if (ioctl(promFd, OPROMNXTPROP, promOpio) < 0)
	    return 0;
	if (!promOpio->oprom_size)
	    return 0;
	if (!strcmp(promOpio->oprom_array, prop))
	    return 1;
    }
}

enum {
    PROM_NODE_PCI = 16,
    PROM_NODE_EBUS = 8,
    PROM_NODE_SBUS = 4,
    PROM_NODE_PREF = 2,
    PROM_NODE_SIBLING = 1
};

static int
promSetNode(sbusPromNodePtr pnode)
{
    int node;

    if (!pnode->node || pnode->node == -1)
	return -1;
    if (pnode->cookie[0] & PROM_NODE_SIBLING)
	node = promGetSibling(pnode->cookie[1]);
    else
	node = promGetChild(pnode->cookie[1]);
    if (pnode->node != node)
	return -1;
    return 0;
}

static void
promIsP1275(void)
{
    FILE *f;
    char buffer[1024];

    if (promP1275 != -1)
	return;
    promP1275 = 0;
    f = fopen("/proc/cpuinfo","r");
    if (!f) return;
    while (fgets(buffer, 1024, f) != NULL)
	if (!strncmp (buffer, "type", 4) && strstr (buffer, "sun4u")) {
	    promP1275 = 1;
	    break;
	}
    fclose(f);
}

void
sparcPromClose(void)
{
    if (promOpenCount > 1) {
	promOpenCount--;
	return;
    }
    if (promFd != -1) {
	close(promFd);
	promFd = -1;
    }
    if (promOpio) {
	xfree(promOpio);
	promOpio = NULL;
    }
    promOpenCount = 0;
}

int
sparcPromInit(void)
{
    if (promOpenCount) {
	promOpenCount++;
	return 0;
    }
    promFd = open("/dev/openprom", O_RDONLY, 0);
    if (promFd == -1)
	return -1;
    promOpio = (struct openpromio *)xalloc(4096);
    if (!promOpio) {
	sparcPromClose();
	return -1;
    }
    promRootNode = promGetSibling(0);
    if (!promRootNode) {
	sparcPromClose();
	return -1;
    }
    promIsP1275();
    promOpenCount++;

    return 0;
}

char *
sparcPromGetProperty(sbusPromNodePtr pnode, const char *prop, int *lenp)
{
    if (promSetNode(pnode))
	return NULL;
    return promGetProperty(prop, lenp);
}

int
sparcPromGetBool(sbusPromNodePtr pnode, const char *prop)
{
    if (promSetNode(pnode))
	return 0;
    return promGetBool(prop);
}

static sbusDevicePtr *devicePtrs;

static void promWalk(int node, int oldnode, int flags)
{
    int nextnode;
    int len, sbus = (flags & PROM_NODE_SBUS);
    char *prop = promGetProperty("device_type", &len);
    int devId, i, j;
    sbusPromNode pNode, pNode2;

    while (prop) {
	if (len <= 0 || strcmp(prop, "display"))
	    break;
	prop = promGetProperty("name", &len);
	if (!prop || len <= 0)
	    break;
	while ((*prop >= 'A' && *prop <= 'Z') || *prop == ',') prop++;
	if (!sbus && strcmp(prop, "ffb") && strcmp(prop, "afb") &&
	    strcmp(prop, "cgfourteen"))
	    break;
	/*
	 * All /SUNW,ffb outside of SBUS tree come before all
	 * /SUNW,afb outside of SBUS tree in Linux.
	 */
	if (!sbus && !strcmp(prop, "afb"))
	    flags |= PROM_NODE_PREF;
	for (i = 0; sbusDeviceTable[i].devId; i++)
	    if (!strcmp(prop, sbusDeviceTable[i].promName))
		break;
	devId = sbusDeviceTable[i].devId;
	if (! devId)
	    break;
	for (i = 0; i < 32; i++) {
	    if (! devicePtrs[i] || devicePtrs[i]->devId != devId)
		continue;
	    if (devicePtrs[i]->node.node) {
		if ((devicePtrs[i]->node.cookie[0] & ~PROM_NODE_SIBLING) <=
		    (flags & ~PROM_NODE_SIBLING))
		    continue;
		for (j = i + 1, pNode = devicePtrs[i]->node; j < 32; j++) {
		    if (! devicePtrs[j] || devicePtrs[j]->devId != devId)
			continue;
		    pNode2 = devicePtrs[j]->node;
		    devicePtrs[j]->node = pNode;
		    pNode = pNode2;
		}
	    }
	    devicePtrs[i]->node.node = node;
	    devicePtrs[i]->node.cookie[0] = flags;
	    devicePtrs[i]->node.cookie[1] = oldnode;
	    break;
	}
	break;
    }
    prop = promGetProperty("name", &len);
    if (prop && len > 0) {
	if (!strcmp(prop, "sbus") || !strcmp(prop, "sbi"))
	    sbus = PROM_NODE_SBUS;
    }
    nextnode = promGetChild(node);
    if (nextnode)
	promWalk(nextnode, node, sbus);
    nextnode = promGetSibling(node);
    if (nextnode)
	promWalk(nextnode, node, PROM_NODE_SIBLING|sbus);
}

void
sparcPromAssignNodes(void)
{
    sbusDevicePtr psdp, *psdpp;
    int n, holes = 0, i, j;
    FILE *f;

    devicePtrs = xnfcalloc(sizeof(sbusDevicePtr), 32);
    for (psdpp = xf86SbusInfo, psdp = *psdpp, n = 0; psdp; psdp = *++psdpp, n++) {
	if (psdp->fbNum != n)
	    holes = 1;
	devicePtrs[psdp->fbNum] = psdp;
    }
    if (holes && (f = fopen("/proc/fb", "r")) != NULL) {
	/* We could not open one of fb devices, check /proc/fb to see what
	 * were the types of the cards missed. */
	char buffer[64];
	int fbNum, devId;
	static struct {
	    int devId;
	    char *prefix;
	} procFbPrefixes[] = {
	    { SBUS_DEVICE_BW2, "BWtwo" },
	    { SBUS_DEVICE_CG14, "CGfourteen" },
	    { SBUS_DEVICE_CG6, "CGsix" },
	    { SBUS_DEVICE_CG3, "CGthree" },
	    { SBUS_DEVICE_FFB, "Creator" },
	    { SBUS_DEVICE_FFB, "Elite 3D" },
	    { SBUS_DEVICE_LEO, "Leo" },
	    { SBUS_DEVICE_TCX, "TCX" },
	    { 0, NULL },
	};

	while (fscanf(f, "%d %63s\n", &fbNum, buffer) == 2) {
	    for (i = 0; procFbPrefixes[i].devId; i++)
		if (! strncmp(procFbPrefixes[i].prefix, buffer,
		      strlen(procFbPrefixes[i].prefix)))
		    break;
	    devId = procFbPrefixes[i].devId;
	    if (! devId) continue;
	    if (devicePtrs[fbNum]) {
		if (devicePtrs[fbNum]->devId != devId)
		    xf86ErrorF("Inconsistent /proc/fb with FBIOGATTR\n");
	    } else if (!devicePtrs[fbNum]) {
		devicePtrs[fbNum] = psdp = xnfcalloc(sizeof (sbusDevice), 1);
		psdp->devId = devId;
		psdp->fbNum = fbNum;
		psdp->fd = -2;
	    }
	}
	fclose(f);
    }
    promGetSibling(0);
    promWalk(promRootNode, 0, 2);
    for (i = 0, j = 0; i < 32; i++)
	if (devicePtrs[i] && devicePtrs[i]->fbNum == -1)
	    j++;
    xf86SbusInfo = xnfrealloc(xf86SbusInfo, sizeof(psdp) * (n + j + 1));
    for (i = 0, psdpp = xf86SbusInfo; i < 32; i++)
	if (devicePtrs[i]) {
	    if (devicePtrs[i]->fbNum == -1) {
		memmove(psdpp + 1, psdpp, sizeof(psdpp) * (n + 1));
		*psdpp = devicePtrs[i];
	    } else
		n--;
	}
    xfree(devicePtrs);
}

static char *
promGetReg(int type)
{
    char *prop;
    int len;
    static char regstr[40];

    regstr[0] = 0;
    prop = promGetProperty("reg", &len);
    if (prop && len >= 4) {
	unsigned int *reg = (unsigned int *)prop;
	if (!promP1275 || (type == PROM_NODE_SBUS) || (type == PROM_NODE_EBUS))
	    sprintf (regstr, "@%x,%x", reg[0], reg[1]);
	else if (type == PROM_NODE_PCI) {
	    if ((reg[0] >> 8) & 7)
		sprintf (regstr, "@%x,%x", (reg[0] >> 11) & 0x1f, (reg[0] >> 8) & 7);
	    else
		sprintf (regstr, "@%x", (reg[0] >> 11) & 0x1f);
	} else if (len == 4)
	    sprintf (regstr, "@%x", reg[0]);
	else {
	    unsigned int regs[2];

	    /* Things get more complicated on UPA. If upa-portid exists,
	       then address is @upa-portid,second-int-in-reg, otherwise
	       it is @first-int-in-reg/16,second-int-in-reg (well, probably
	       upa-portid always exists, but just to be safe). */
	    memcpy (regs, reg, sizeof(regs));
	    prop = promGetProperty("upa-portid", &len);
	    if (prop && len == 4) {
		reg = (unsigned int *)prop;
		sprintf (regstr, "@%x,%x", reg[0], regs[1]);
	    } else
		sprintf (regstr, "@%x,%x", regs[0] >> 4, regs[1]);
	}
    }
    return regstr;
}

static int
promWalk2(char *path, int parent, int node, int searchNode, int type)
{
    int nextnode;
    int len, ntype = type;
    char *prop, *p;

    prop = promGetProperty("name", &len);
    *path = '/';
    if (!prop || len <= 0)
	return 0;
    if ((!strcmp(prop, "sbus") || !strcmp(prop, "sbi")) && !type)
	ntype = PROM_NODE_SBUS;
    else if (!strcmp(prop, "ebus") && type == PROM_NODE_PCI)
	ntype = PROM_NODE_EBUS;
    else if (!strcmp(prop, "pci") && !type)
	ntype = PROM_NODE_PCI;
    strcpy (path + 1, prop);
    p = promGetReg(type);
    if (*p)
	strcat (path, p);
    if (node == searchNode)
	return 1;
    nextnode = promGetChild(node);
    if (nextnode && promWalk2(strchr (path, 0), node, nextnode, searchNode, ntype))
	return 1;
    nextnode = promGetSibling(node);
    if (nextnode && promWalk2(path, parent, nextnode, searchNode, type))
	return 1;
    return 0;
}

char *
sparcPromNode2Pathname(sbusPromNodePtr pnode)
{
    char *ret;

    if (!pnode->node) return NULL;
    ret = xalloc(4096);
    if (!ret) return NULL;
    if (promWalk2(ret, promRootNode, promGetChild(promRootNode), pnode->node, 0))
	return ret;
    xfree(ret);
    return NULL;
}

static int
promWalk3(char *name, char *regstr, int parent, int type)
{
    int len, node, ret;
    char *prop, *p;

    for (;;) {
	prop = promGetProperty("name", &len);
	if (!prop || len <= 0)
	    return 0;
	if ((!strcmp(prop, "sbus") || !strcmp(prop, "sbi")) && !type)
	    type = PROM_NODE_SBUS;
	else if (!strcmp(prop, "ebus") && type == PROM_NODE_PCI)
	    type = PROM_NODE_EBUS;
	else if (!strcmp(prop, "pci") && !type)
	    type = PROM_NODE_PCI;
	for (node = promGetChild(parent); node; node = promGetSibling(node)) {
	    prop = promGetProperty("name", &len);
	    if (!prop || len <= 0)
		continue;
	    if (*name && strcmp(name, prop))
		continue;
	    if (*regstr) {
		p = promGetReg(type);
		if (! *p || strcmp(p + 1, regstr))
		    continue;
	    }
	    break;
	}
	if (!node) {
	    for (node = promGetChild(parent); node; node = promGetSibling(node)) {
		ret = promWalk3(name, regstr, node, type);
		if (ret) return ret;
	    }
	    return 0;
	}
	name = strchr(regstr, 0) + 1;
	if (! *name)
	    return node;
	p = strchr(name, '/');
	if (p)
	    *p = 0;
	else
	    p = strchr(name, 0);
	regstr = strchr(name, '@');
	if (regstr)
	    *regstr++ = 0;
	else
	    regstr = p;
	if (name == regstr)
	    return 0;
	parent = node;
    }
}

int
sparcPromPathname2Node(const char *pathName)
{
    int i;
    char *name, *regstr, *p;

    i = strlen(pathName);
    name = xalloc(i + 2);
    if (! name) return 0;
    strcpy (name, pathName);
    name [i + 1] = 0;
    if (name[0] != '/')
	return 0;
    p = strchr(name + 1, '/');
    if (p)
	*p = 0;
    else
	p = strchr(name, 0);
    regstr = strchr(name, '@');
    if (regstr)
	*regstr++ = 0;
    else
	regstr = p;
    if (name + 1 == regstr)
	return 0;
    promGetSibling(0);
    i = promWalk3(name + 1, regstr, promRootNode, 0);
    xfree(name);
    return i;
}

pointer
xf86MapSbusMem(sbusDevicePtr psdp, unsigned long offset, unsigned long size)
{
    pointer ret;

    if (psdp->fd == -1) {
	psdp->fd = open(psdp->device, O_RDWR);
	if (psdp->fd == -1)
	    return NULL;
    } else if (psdp->fd < 0)
	return NULL;

    ret = (pointer) mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE,
			  psdp->fd, offset);
    if (ret == (pointer) -1) {
	ret = (pointer) mmap (NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED,
			      psdp->fd, offset);
    }
    if (ret == (pointer) -1)
	return NULL;

    return ret;
}

void
xf86UnmapSbusMem(sbusDevicePtr psdp, pointer addr, unsigned long size)
{
    munmap (addr, size);
}

/* Tell OS that we are driving the HW cursor ourselves. */
void
xf86SbusHideOsHwCursor(sbusDevicePtr psdp)
{
    struct fbcursor fbcursor;
    unsigned char zeros[8];

    memset(&fbcursor, 0, sizeof(fbcursor));
    memset(&zeros, 0, sizeof(zeros));
    fbcursor.cmap.count = 2;
    fbcursor.cmap.red = zeros;
    fbcursor.cmap.green = zeros;
    fbcursor.cmap.blue = zeros;
    fbcursor.image = (char *)zeros;
    fbcursor.mask = (char *)zeros;
    fbcursor.size.x = 32;
    fbcursor.size.y = 1;
    fbcursor.set = FB_CUR_SETALL;
    ioctl(psdp->fd, FBIOSCURSOR, &fbcursor);
}

/* Set HW cursor colormap. */
void
xf86SbusSetOsHwCursorCmap(sbusDevicePtr psdp, int bg, int fg)
{
    struct fbcursor fbcursor;
    unsigned char red[2], green[2], blue[2];

    memset(&fbcursor, 0, sizeof(fbcursor));
    red[0] = bg >> 16;
    green[0] = bg >> 8;
    blue[0] = bg;
    red[1] = fg >> 16;
    green[1] = fg >> 8;
    blue[1] = fg;
    fbcursor.cmap.count = 2;
    fbcursor.cmap.red = red;
    fbcursor.cmap.green = green;
    fbcursor.cmap.blue = blue;
    fbcursor.set = FB_CUR_SETCMAP;
    ioctl(psdp->fd, FBIOSCURSOR, &fbcursor);
}
