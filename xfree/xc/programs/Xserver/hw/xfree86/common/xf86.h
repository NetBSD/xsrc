/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86.h,v 3.185 2005/02/26 18:31:48 dawes Exp $ */

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
 * This file contains declarations for public XFree86 functions and variables,
 * and definitions of public macros.
 *
 * "public" means available to video drivers.
 */

#ifndef _XF86_H
#define _XF86_H

#include "xf86str.h"
#include "xf86Opt.h"
#include <X11/Xfuncproto.h>
#ifndef IN_MODULE
#include <stdarg.h>
#else
#include "xf86_ansic.h"
#endif

#include "propertyst.h"

/* General parameters */
extern int xf86DoConfigure;
extern Bool xf86DoConfigurePass1;
extern int xf86ScreenIndex;		/* Index into pScreen.devPrivates */
extern int xf86CreateRootWindowIndex;	/* Index into pScreen.devPrivates */
extern int xf86PixmapIndex;
extern Bool xf86ResAccessEnter;
extern ScrnInfoPtr *xf86Screens;	/* List of pointers to ScrnInfoRecs */
extern const unsigned char byte_reversed[256];
extern ScrnInfoPtr xf86CurrentScreen;
extern Bool pciSlotClaimed;
extern Bool isaSlotClaimed;
extern Bool fbSlotClaimed;
#ifdef __sparc__
extern Bool sbusSlotClaimed;
#endif
extern confDRIRec xf86ConfigDRI;
extern Bool xf86inSuspend;

#define XF86SCRNINFO(p) ((ScrnInfoPtr)((p)->devPrivates[xf86ScreenIndex].ptr))

#define XF86FLIP_PIXELS() \
	do { \
	    if (xf86GetFlipPixels()) { \
		pScreen->whitePixel = (pScreen->whitePixel) ? 0 : 1; \
		pScreen->blackPixel = (pScreen->blackPixel) ? 0 : 1; \
	   } \
	while (0)

/* Some macros for messages. */
#define BOOLTOSTRING(b) ((b) ? "TRUE" : "FALSE")
#define PLURAL(n) ((n) == 1 ? "" : "s")
#define EMPTYIFNULL(s) ((s) ? (s) : "")


#define PIX24TOBPP(p) (((p) == Pix24Use24) ? 24 : \
			(((p) == Pix24Use32) ? 32 : 0))

/* variables for debugging */
#ifdef BUILDDEBUG
extern char* xf86p8bit[];
extern CARD32 xf86DummyVar1;
extern CARD32 xf86DummyVar2;
extern CARD32 xf86DummyVar3;
#endif

/* Function Prototypes */
#ifndef _NO_XF86_PROTOTYPES

/* xf86Bus.c */

Bool xf86CheckPciSlot(int bus, int device, int func);
int xf86ClaimPciSlot(int bus, int device, int func, DriverPtr drvp,
		     int chipset, GDevPtr dev, Bool active);
Bool xf86ParsePciBusString(const char *busID, int *bus, int *device,
			   int *func);
Bool xf86ComparePciBusString(const char *busID, int bus, int device, int func);
void xf86FormatPciBusNumber(int busnum, char *buffer);
pciVideoPtr *xf86GetPciVideoInfo(void);
pciConfigPtr *xf86GetPciConfigInfo(void);
void xf86SetPciVideo(pciVideoPtr, resType);
void xf86PrintResList(int verb, resPtr list);
resPtr xf86AddRangesToList(resPtr list, resRange *pRange, int entityIndex);
int xf86ClaimIsaSlot(DriverPtr drvp, int chipset, GDevPtr dev, Bool active);
int xf86GetIsaInfoForScreen(int scrnIndex);
int  xf86GetFbInfoForScreen(int scrnIndex);
Bool xf86ParseIsaBusString(const char *busID);
int xf86ClaimFbSlot(DriverPtr drvp, int chipset, GDevPtr dev, Bool active);
int xf86ClaimNoSlot(DriverPtr drvp, int chipset, GDevPtr dev, Bool active);
void xf86EnableAccess(ScrnInfoPtr pScrn);
void xf86SetCurrentAccess(Bool Enable, ScrnInfoPtr pScrn);
Bool xf86IsPrimaryPci(pciVideoPtr pPci);
Bool xf86IsPrimaryIsa(void);
int xf86CheckPciGAType(pciVideoPtr pPci);
/* new RAC */
resPtr xf86AddResToList(resPtr rlist, resRange *Range, int entityIndex);
resPtr xf86JoinResLists(resPtr rlist1, resPtr rlist2);
resPtr xf86DupResList(const resPtr rlist);
void xf86FreeResList(resPtr rlist);
void xf86ClaimFixedResources(resList list, int entityIndex);
Bool xf86DriverHasEntities(DriverPtr drvp);
void xf86ClearDriverEntities(DriverPtr drvp);
void xf86AddEntityToScreen(ScrnInfoPtr pScrn, int entityIndex);
void xf86SetEntityInstanceForScreen(ScrnInfoPtr pScrn, int entityIndex,
				    int instance);
int xf86GetNumEntityInstances(int entityIndex);
GDevPtr xf86GetDevFromEntity(int entityIndex, int instance);
void xf86RemoveEntityFromScreen(ScrnInfoPtr pScrn, int entityIndex);
EntityInfoPtr xf86GetEntityInfo(int entityIndex);
pciVideoPtr xf86GetPciInfoForEntity(int entityIndex);
int xf86GetPciEntity(int bus, int dev, int func);
Bool xf86SetEntityFuncs(int entityIndex, EntityProc init,
			EntityProc enter, EntityProc leave, pointer);
void xf86DeallocateResourcesForEntity(int entityIndex, unsigned long type);
resPtr xf86RegisterResources(int entityIndex, resList list, 
			     unsigned long Access);
Bool xf86CheckPciMemBase(pciVideoPtr pPci, memType base);
void xf86SetAccessFuncs(EntityInfoPtr pEnt, xf86SetAccessFuncPtr funcs,
			xf86SetAccessFuncPtr oldFuncs);
Bool xf86IsEntityPrimary(int entityIndex);
Bool xf86FixPciResource(int entityIndex, int prt, memType alignment,
			unsigned long type);
resPtr xf86ReallocatePciResources(int entityIndex, resPtr pRes);
resPtr xf86SetOperatingState(resList list, int entityIndex, int mask);
void xf86EnterServerState(xf86State state);
resRange xf86GetBlock(unsigned long type, memType size,
		      memType window_start, memType window_end,
		      memType align_mask, resPtr avoid);
resRange xf86GetSparse(unsigned long type, memType fixed_bits,
		       memType decode_mask, memType address_mask,
		       resPtr avoid);
memType xf86ChkConflict(resRange *rgp, int entityIndex);
Bool xf86IsPciDevPresent(int bus, int dev, int func);
ScrnInfoPtr xf86FindScreenForEntity(int entityIndex);
Bool xf86NoSharedResources(int screenIndex, resType res);
resPtr xf86FindIntersectOfLists(resPtr l1, resPtr l2);
pciVideoPtr xf86FindPciDeviceVendor(CARD16 vendorID, CARD16 deviceID,
				    char n, pciVideoPtr pvp_exclude);
pciVideoPtr xf86FindPciClass(CARD8 intf, CARD8 subClass, CARD16 class,
			     char n, pciVideoPtr pvp_exclude);
#ifdef INCLUDE_DEPRECATED
void xf86EnablePciBusMaster(pciVideoPtr pPci, Bool enable);
#endif
void xf86RegisterStateChangeNotificationCallback(xf86StateChangeNotificationCallbackFunc func, pointer arg);
Bool xf86DeregisterStateChangeNotificationCallback(xf86StateChangeNotificationCallbackFunc func);
#ifdef async
Bool xf86QueueAsyncEvent(void (*func)(pointer),pointer arg);
#endif

int xf86GetLastScrnFlag(int entityIndex);
void xf86SetLastScrnFlag(int entityIndex, int scrnIndex);
Bool xf86IsEntityShared(int entityIndex);
void xf86SetEntityShared(int entityIndex);
Bool xf86IsEntitySharable(int entityIndex);
void xf86SetEntitySharable(int entityIndex);
Bool xf86IsPrimInitDone(int entityIndex);
void xf86SetPrimInitDone(int entityIndex);
void xf86ClearPrimInitDone(int entityIndex);
int xf86AllocateEntityPrivateIndex(void);
DevUnion *xf86GetEntityPrivate(int entityIndex, int privIndex);

/* xf86Config.c */
void xf86ConfFreeScreenData(confScreenPtr pConfScreen);
void xf86ConfFreeGraphicsDeviceData(GDevPtr pGDev);
void xf86ConfFreeInputDeviceData(IDevPtr pIDev);
void xf86ConfFreeMonitorData(MonPtr pMonitor);
void xf86ConfFreeModeData(DisplayModePtr pMode);
void xf86ConfFreeModeSetData(confModeSetPtr pModeSet);
void xf86ConfFreeModeList(DisplayModePtr pModes);
void xf86ConfFreeDisplayData(DispPtr pDisplay);
void xf86ConfFreeXvAdaptorData(confXvAdaptorPtr pAdaptor);
void xf86ConfFreeXvPortData(confXvPortPtr pPort);
void xf86ConfFreeScreenLayoutData(screenLayoutPtr pScreenLayout);
void xf86ConfFreeServerLayoutData(serverLayoutPtr pServerLayout);
void xf86ConfFreeFilesData(confFilesPtr pFiles);
void xf86ConfFreeServerFlagsData(confFlagsPtr pFlags);
void xf86ConfFreeLoadModuleData(confLoadModulePtr pLoad);
void xf86ConfFreeModulesData(confModulesPtr pModules);
void xf86ConfFreeDRIData(confDRIPtr pDri);
void xf86ConfFreeDRIBufferData(confDRIBufferPtr pDriBuf);
void xf86ConfFreeVendorData(confVendorPtr pVendor);
void xf86ConfFreeVendorSubData(confVendorSubPtr pVendorSub);
confScreenPtr xf86ConfAllocScreen(void);
GDevPtr xf86ConfAllocGraphicsDevice(void);
IDevPtr xf86ConfAllocInputDevice(void);
MonPtr xf86ConfAllocMonitor(void);
DisplayModePtr xf86ConfAllocMode(void);
confModeSetPtr xf86ConfAllocModeSet(void);
DisplayModePtr xf86ConfAllocModeList(int n);
DispPtr xf86ConfAllocDisplay(void);
confXvAdaptorPtr xf86ConfAllocXvAdaptor(void);
confXvPortPtr xf86ConfAllocXvPort(void);
screenLayoutPtr xf86ConfAllocScreenLayout(void);
serverLayoutPtr xf86ConfAllocServerLayout(void);
confFilesPtr xf86ConfAllocFiles(void);
confFlagsPtr xf86ConfAllocServerFlags(void);
confLoadModulePtr xf86ConfAllocLoadModule(void);
confModulesPtr xf86ConfAllocModules(void);
confDRIPtr xf86ConfAllocDRI(void);
confDRIBufferPtr xf86ConfAllocDRIBuffer(void);
confVendorPtr xf86ConfAllocVendor(void);
confVendorSubPtr xf86ConfAllocVendorSub(void);
confScreenPtr xf86ConfDupScreen(const confScreenRec *pConfScreen, int depth);
GDevPtr xf86ConfDupGraphicsDevice(const GDevRec *pGDev);
IDevPtr xf86ConfDupInputDevice(const IDevRec *pIDev, int depth);
MonPtr xf86ConfDupMonitor(const MonRec *pMonitor, int depth);
DisplayModePtr xf86ConfDupMode(const DisplayModeRec *pMode, int depth);
confModeSetPtr xf86ConfDupModeSet(const confModeSetRec *pModeSet, int depth);
DisplayModePtr xf86ConfDupModeList(const DisplayModeRec *pModes, int depth);
DispPtr xf86ConfDupDisplay(const DispRec *pDisplay);
confXvAdaptorPtr xf86ConfDupXvAdaptor(const confXvAdaptorRec *pAdaptor);
confXvPortPtr xf86ConfDupXvPort(const confXvPortRec *pPort);
screenLayoutPtr xf86ConfDupScreenLayout(const screenLayoutRec *pScreenLayout,
					int depth);
serverLayoutPtr xf86ConfDupServerLayout(const serverLayoutRec *pServerLayout,
					int depth);
confFilesPtr xf86ConfDupFiles(const confFilesRec *pFiles);
confFlagsPtr xf86ConfDupServerFlags(const confFlagsRec *pFlags);
confLoadModulePtr xf86ConfDupLoadModule(const confLoadModuleRec *pLoad);
confModulesPtr xf86ConfDupModules(const confModulesRec *pModules);
confDRIPtr xf86ConfDupDRI(const confDRIRec *pDri);
confDRIBufferPtr xf86ConfDupDRIBuffer(const confDRIBufferRec *pDriBuffer);
confVendorPtr xf86ConfDupVendor(const confVendorRec *pVendor);
confVendorSubPtr xf86ConfDupVendorSub(const confVendorSubRec *pVendorSub);
serverLayoutPtr xf86ConfResolveServerLayout(ConfigHandle handle,
					    serverLayoutPtr pServerLayout,
					    int depth);
int xf86ConfCheckResolvedServerLayout(const serverLayoutRec *pServerLayout,
				      int depth, Bool strict);
serverLayoutPtr xf86ConfGetServerLayoutByName(ConfigHandle handle,
					      const char *name, int depth);
serverLayoutPtr xf86ConfGetNextServerLayout(ConfigHandle handle,
					    ConfigDataHandle prevLayoutHandle,
					    int depth);
confScreenPtr xf86ConfResolveScreen(ConfigHandle handle,
				    confScreenPtr pConfScreen, int depth);
int xf86ConfCheckResolvedScreen(const confScreenRec *pConfScreen, int depth,
				Bool strict);
confScreenPtr xf86ConfGetScreenByName(ConfigHandle handle, const char *name,
				      int depth);
confScreenPtr xf86ConfGetNextScreen(ConfigHandle handle,
				    ConfigDataHandle prevScreenHandle,
				    int depth);
MonPtr xf86ConfResolveMonitor(ConfigHandle handle, MonPtr pMonitor);
int xf86ConfCheckResolvedMonitor(const MonRec *pMonitor, int depth,
				 Bool strict);
MonPtr xf86ConfGetMonitorByName(ConfigHandle handle, const char *name,
				int depth);
MonPtr xf86ConfGetNextMonitor(ConfigHandle handle,
			      ConfigDataHandle prevMonitorHandle, int depth);
confModeSetPtr xf86ConfGetModeSetByName(ConfigHandle handle, const char *name);
confModeSetPtr xf86ConfGetNextModeSet(ConfigHandle handle,
				      ConfigDataHandle prevModeSetHandle);
GDevPtr xf86ConfGetGraphicsDeviceByName(ConfigHandle handle, const char *name);
GDevPtr xf86ConfGetNextGraphicsDevice(ConfigHandle handle,
				      ConfigDataHandle prevDeviceHandle);
IDevPtr xf86ConfGetInputDeviceByName(ConfigHandle handle, const char *name);
IDevPtr xf86ConfGetNextInputDevice(ConfigHandle handle,
				   ConfigDataHandle prevIDevHandle, int depth);
IDevPtr xf86ConfGetInputDeviceByDriver(ConfigHandle handle, const char *driver);
IDevPtr xf86ConfGetInputDeviceByOption(ConfigHandle handle, const char *option);
confXvAdaptorPtr xf86ConfGetXvAdaptorByName(ConfigHandle handle,
					    const char *name);
confXvAdaptorPtr xf86ConfGetNextXvAdaptor(ConfigHandle handle,
					  ConfigDataHandle prevAdaptorHandle);
confFilesPtr xf86ConfGetFilesByName(ConfigHandle handle, const char *name);
confFilesPtr xf86ConfGetNextFiles(ConfigHandle handle,
				  ConfigDataHandle prevFilesHandle);
confFilesPtr xf86ConfCombineFilesData(const confFilesRec *src1, MessageType m1,
				      const confFilesRec *src2, MessageType m2);
confModulesPtr xf86ConfGetModulesByName(ConfigHandle handle, const char *name);
confModulesPtr xf86ConfGetNextModules(ConfigHandle handle,
				      ConfigDataHandle prevModulesHandle);
confModulesPtr xf86ConfCombineModulesData(const confModulesRec *src1,
					  const confModulesRec *src2);
confFlagsPtr xf86ConfGetServerFlagsByName(ConfigHandle handle,
					  const char *name);
confFlagsPtr xf86ConfGetNextServerFlags(ConfigHandle handle,
					ConfigDataHandle prevFilesHandle);
confFlagsPtr xf86ConfCombineServerFlagsData(const confFlagsRec *src1,
					    const confFlagsRec *src2);
confDRIPtr xf86ConfGetDRIByName(ConfigHandle handle, const char *name);
confDRIPtr xf86ConfGetNextDRI(ConfigHandle handle,
			      ConfigDataHandle prevDRIHandle);
confVendorPtr xf86ConfGetVendorByName(ConfigHandle handle, const char *name);
confVendorPtr xf86ConfGetVendorByVendorName(ConfigHandle handle,
					    const char *vname);
confVendorPtr xf86ConfGetNextVendor(ConfigHandle handle,
				    ConfigDataHandle prevVendorHandle);

/* xf86Configure.c */
GDevPtr xf86AddBusDeviceToConfigure(const char *driver, BusType bus,
				    void *busData, int chipset);
GDevPtr xf86AddDeviceToConfigure(const char *driver, pciVideoPtr pVideo,
				 int chipset);
 
/* xf86Cursor.c */

void xf86LockZoom(ScreenPtr pScreen, int lock);
void xf86InitViewport(ScrnInfoPtr pScr);
void xf86SetViewport(ScreenPtr pScreen, int x, int y);
void xf86ZoomViewport(ScreenPtr pScreen, int zoom);
Bool xf86SwitchMode(ScreenPtr pScreen, DisplayModePtr mode);
void *xf86GetPointerScreenFuncs(void);
void xf86InitOrigins(void);
void xf86ReconfigureLayout(void);
 
/* xf86DPMS.c */

Bool xf86DPMSInit(ScreenPtr pScreen, DPMSSetProcPtr set, int flags);

/* xf86DGA.c */

Bool DGAInit(ScreenPtr pScreen, DGAFunctionPtr funcs, DGAModePtr modes, 
			int num);
xf86SetDGAModeProc xf86SetDGAMode;

/* xf86Events.c */

void SetTimeSinceLastInputEvent(void);
pointer xf86AddInputHandler(int fd, InputHandlerProc proc, pointer data);
int xf86RemoveInputHandler(pointer handler);
void xf86DisableInputHandler(pointer handler);
void xf86EnableInputHandler(pointer handler);
void xf86InterceptSignals(int *signo);
Bool xf86EnableVTSwitch(Bool new);
Bool xf86CommonSpecialKey(int key, Bool down, int modifiers);
void xf86ProcessActionEvent(ActionEvent action, void *arg);

/* xf86Helper.c */

/* XXX Need to check which GCC versions have the format(printf) attribute. */
#if (!defined(printf) || defined(printf_is_xf86printf)) && \
    defined(__GNUC__) && \
    ((__GNUC__ > 2) || ((__GNUC__ == 2) && (__GNUC_MINOR__ > 4)))
# define _printf_attribute(a,b) __attribute((format(printf,a,b)))
# undef printf
#else
# define _printf_attribute(a,b) /**/
#endif

void xf86AddDriver(DriverPtr driver, pointer module, int flags);
void xf86DeleteDriver(int drvIndex, Bool deferUnload);
ScrnInfoPtr xf86AllocateScreen(DriverPtr drv, int flags);
void xf86DeleteScreen(int scrnIndex, int flags);
int xf86AllocateScrnInfoPrivateIndex(void);
Bool xf86AddPixFormat(ScrnInfoPtr pScrn, int depth, int bpp, int pad);
Bool xf86SetDepthBpp(ScrnInfoPtr scrp, int depth, int bpp, int fbbpp,
		     int depth24flags);
void xf86PrintDepthBpp(ScrnInfoPtr scrp);
Bool xf86SetWeight(ScrnInfoPtr scrp, rgb weight, rgb mask);
Bool xf86SetDefaultVisual(ScrnInfoPtr scrp, int visual);
Bool xf86SetGamma(ScrnInfoPtr scrp, Gamma newGamma);
void xf86SetDpi(ScrnInfoPtr pScrn, int x, int y);
void xf86SetBlackWhitePixels(ScreenPtr pScreen);
void xf86EnableDisableFBAccess(int scrnIndex, Bool enable);
void xf86VDrvMsgVerb(int scrnIndex, MessageType type, int verb,
		     const char *format, va_list args);
void xf86DrvMsgVerb(int scrnIndex, MessageType type, int verb,
		    const char *format, ...) _printf_attribute(4,5);
void xf86DrvMsg(int scrnIndex, MessageType type, const char *format, ...)
		_printf_attribute(3,4);
void xf86MsgVerb(MessageType type, int verb, const char *format, ...)
		_printf_attribute(3,4);
void xf86Msg(MessageType type, const char *format, ...) _printf_attribute(2,3);
void xf86ErrorFVerb(int verb, const char *format, ...) _printf_attribute(2,3);
void xf86ErrorF(const char *format, ...) _printf_attribute(1,2);
const char *xf86TokenToString(SymTabPtr table, int token);
int xf86StringToToken(SymTabPtr table, const char *string);
void xf86ShowClocks(ScrnInfoPtr scrp, MessageType from);
void xf86PrintChipsets(const char *drvname, const char *drvmsg,
		       SymTabPtr chips);
int xf86MatchDevice(const char *drivername, GDevPtr **driversectlist);
int xf86MatchPciInstances(const char *driverName, int vendorID, 
		      SymTabPtr chipsets, PciChipsets *PCIchipsets,
		      GDevPtr *devList, int numDevs, DriverPtr drvp,
		      int **foundEntities);
int xf86MatchIsaInstances(const char *driverName, SymTabPtr chipsets,
			  IsaChipsets *ISAchipsets, DriverPtr drvp,
			  FindIsaDevProc FindIsaDevice, GDevPtr *devList,
			  int numDevs, int **foundEntities);
void xf86GetClocks(ScrnInfoPtr pScrn, int num,
		   Bool (*ClockFunc)(ScrnInfoPtr, int),
		   void (*ProtectRegs)(ScrnInfoPtr, Bool),
		   void (*BlankScreen)(ScrnInfoPtr, Bool),
		   IOADDRESS vertsyncreg, int maskval,
		   int knownclkindex, int knownclkvalue);
void xf86SetPriority(Bool up);
const char *xf86GetVisualName(int visual);
int xf86GetVerbosity(void);
Pix24Flags xf86GetPix24(void);
int xf86GetDepth(void);
rgb xf86GetWeight(void);
Gamma xf86GetGamma(void);
Bool xf86GetFlipPixels(void);
const char *xf86GetServerName(void);
Bool xf86ServerIsExiting(void);
Bool xf86ServerIsResetting(void);
Bool xf86ServerIsInitialising(void);
Bool xf86ServerIsOnlyDetecting(void);
Bool xf86ServerIsOnlyProbing(void);
Bool xf86CaughtSignal(void);
Bool xf86GetVidModeAllowNonLocal(void);
Bool xf86GetVidModeEnabled(void);
Bool xf86GetModInDevAllowNonLocal(void);
Bool xf86GetModInDevEnabled(void);
Bool xf86GetAllowMouseOpenFail(void);
Bool xf86IsPc98(void);
void xf86DisableRandR(void);
CARD32 xf86GetVersion(void);
CARD32 xf86GetModuleVersion(pointer module);
pointer xf86LoadDrvSubModule(DriverPtr drv, const char *name);
pointer xf86LoadSubModule(ScrnInfoPtr pScrn, const char *name);
pointer xf86LoadOneModule(char *name, pointer optlist);
void xf86UnloadSubModule(pointer mod);
Bool xf86LoaderCheckSymbol(const char *name);
void xf86LoaderReqSymLists(const char **, ...);
void xf86LoaderReqSymbols(const char *, ...);
void xf86LoaderRefSymLists(const char **, ...);
void xf86LoaderRefSymbols(const char *, ...);
void xf86SetBackingStore(ScreenPtr pScreen);
void xf86SetSilkenMouse(ScreenPtr pScreen);
int xf86NewSerialNumber(WindowPtr p, pointer unused);
pointer xf86FindXvOptions(int scrnIndex, int adapt_index, char *port_name,
			  char **adaptor_name, pointer *adaptor_options);
void xf86GetOS(const char **name, int *major, int *minor, int *teeny);
ScrnInfoPtr xf86ConfigPciEntity(ScrnInfoPtr pScrn, int scrnFlag,
				int entityIndex,PciChipsets *p_chip,
				resList res, EntityProc init,
				EntityProc enter, EntityProc leave,
				pointer private);
ScrnInfoPtr xf86ConfigIsaEntity(ScrnInfoPtr pScrn, int scrnFlag,
				int entityIndex, IsaChipsets *i_chip,
				resList res, EntityProc init,
				EntityProc enter, EntityProc leave,
				pointer private); 
ScrnInfoPtr xf86ConfigFbEntity(ScrnInfoPtr pScrn, int scrnFlag, 
			       int entityIndex, EntityProc init, 
			       EntityProc enter, EntityProc leave, 
			       pointer private);
/* Obsolete! don't use */
Bool xf86ConfigActivePciEntity(ScrnInfoPtr pScrn,
				int entityIndex,PciChipsets *p_chip,
				resList res, EntityProc init,
				EntityProc enter, EntityProc leave,
				pointer private);
/* Obsolete! don't use */
Bool xf86ConfigActiveIsaEntity(ScrnInfoPtr pScrn,
				int entityIndex, IsaChipsets *i_chip,
				resList res, EntityProc init,
				EntityProc enter, EntityProc leave,
				pointer private); 
void xf86ConfigPciEntityInactive(EntityInfoPtr pEnt, PciChipsets *p_chip,
				 resList res, EntityProc init,
				 EntityProc enter, EntityProc leave,
				 pointer private);
void xf86ConfigIsaEntityInactive(EntityInfoPtr pEnt, IsaChipsets *i_chip,
				 resList res, EntityProc init,
				 EntityProc enter, EntityProc leave,
				 pointer private);
void xf86ConfigFbEntityInactive(EntityInfoPtr pEnt, EntityProc init, 
				EntityProc enter, EntityProc leave, 
				pointer private);
Bool xf86IsScreenPrimary(int scrnIndex);
int  xf86RegisterRootWindowProperty(int ScrnIndex, Atom	property, Atom type,
				    int format, unsigned long len,
				    pointer value);
Bool xf86IsUnblank(int mode);
MonPtr xf86GetMonitorByNumber(const ScrnInfoRec *pScrn, int monNum);
DispPtr xf86GetDisplayByMonitorNum(const ScrnInfoRec *pScrn, int monNum);
Bool xf86GetNextMonitor(const ScrnInfoRec *pScrn, MonPtr *pMonitor,
			DispPtr *pDisplay);

#ifdef XFree86LOADER
void xf86AddModuleInfo(ModuleInfoPtr info, pointer module);
void xf86DeleteModuleInfo(int idx);
#endif

#undef _printf_attribute
#if defined(printf_is_xf86printf) && !defined(printf)
#define printf xf86printf
#endif

/* xf86Debug.c */
#ifdef BUILDDEBUG
 void xf86Break1(void);
void xf86Break2(void);
void xf86Break3(void);
CARD8  xf86PeekFb8(CARD8  *p);
CARD16 xf86PeekFb16(CARD16 *p);
CARD32 xf86PeekFb32(CARD32 *p);
void xf86PokeFb8(CARD8  *p, CARD8  v);
void xf86PokeFb16(CARD16 *p, CARD16 v);
void xf86PokeFb32(CARD16 *p, CARD32 v);
CARD8  xf86PeekMmio8(pointer Base, unsigned long Offset);
CARD16 xf86PeekMmio16(pointer Base, unsigned long Offset);
CARD32 xf86PeekMmio32(pointer Base, unsigned long Offset);
void xf86PokeMmio8(pointer Base, unsigned long Offset, CARD8  v);
void xf86PokeMmio16(pointer Base, unsigned long Offset, CARD16 v);
void xf86PokeMmio32(pointer Base, unsigned long Offset, CARD32 v);
extern void xf86SPTimestamp(xf86TsPtr* timestamp, char* string);
extern void xf86STimestamp(xf86TsPtr* timestamp);
#endif
 
/* xf86Init.c */

PixmapFormatPtr xf86GetPixFormat(ScrnInfoPtr pScrn, int depth);
int xf86GetBppFromDepth(ScrnInfoPtr pScrn, int depth);

/* xf86Mode.c */

int xf86GetNearestClock(ScrnInfoPtr scrp, int freq, Bool allowDiv2,
			int DivFactor, int MulFactor, int *divider);
const char *xf86ModeStatusToString(ModeStatus status);
const char *xf86ModeTypeToString(int mType);
ModeStatus xf86LookupMode(ScrnInfoPtr scrp, DisplayModePtr modep,
			  ClockRangePtr clockRanges, LookupModeFlags strategy);
ModeStatus xf86CheckModeForMonitor(DisplayModePtr mode, MonPtr monitor);
ModeStatus xf86InitialCheckModeForDriver(ScrnInfoPtr scrp, DisplayModePtr mode,
					 ClockRangePtr clockRanges,
					 LookupModeFlags strategy,
					 int maxPitch, int virtualX,
					 int virtualY);
ModeStatus xf86CheckModeForDriver(ScrnInfoPtr scrp, DisplayModePtr mode,
				  int flags);
Bool xf86SetMonitorParameters(ScrnInfoPtr pScrn, MonPtr monitor,
			      int hSize, int vSize, int refresh);
Bool xf86AddEDIDModes(ScrnInfoPtr pScrn, MonPtr monitor, int flags);
int xf86ValidateModes(ScrnInfoPtr scrp, DisplayModePtr availModes,
		      char **modeNames, ClockRangePtr clockRanges,
		      int *linePitches, int minPitch, int maxPitch,
		      int minHeight, int maxHeight, int pitchInc,
		      int virtualX, int virtualY, int apertureSize,
		      LookupModeFlags strategy);
void xf86DeleteMode(DisplayModePtr *modeList, DisplayModePtr mode);
void xf86PruneDriverModes(ScrnInfoPtr scrp);
void xf86SetCrtcForModes(ScrnInfoPtr scrp, int adjustFlags);
void xf86PrintModes(ScrnInfoPtr scrp);
void xf86ShowClockRanges(ScrnInfoPtr scrp, ClockRangePtr clockRanges);
Bool xf86ModeIsPresent(const char *modeName, const DisplayModeRec *modeList,
		       int inclTypeMask, int exclTypeMask);
void xf86AddModeAfter(DisplayModePtr *ppOld, DisplayModePtr pNew);
void xf86AddModeBefore(DisplayModePtr *ppOld, DisplayModePtr pNew);
void xf86AddModeToMonitor(MonPtr pMonitor, DisplayModePtr pNew);


/* xf86Option.c */

void xf86CollectOptions(ScrnInfoPtr pScrn, pointer extraOpts);


/* xf86RandR.c */
#ifdef RANDR
Bool xf86RandRInit (ScreenPtr    pScreen);
void xf86RandRSetInitialMode (ScreenPtr pScreen);
#endif

/* xf86VidModeExtentionInit.c */

Bool VidModeExtensionInit(ScreenPtr pScreen);

/* xf86Versions.c */
CARD32 xf86GetBuiltinInterfaceVersion(BuiltinInterface iface, int flag);
Bool xf86RegisterBuiltinInterfaceVersion(BuiltinInterface iface,
					 CARD32 version, int flags);


#endif /* _NO_XF86_PROTOTYPES */

#endif /* _XF86_H */
