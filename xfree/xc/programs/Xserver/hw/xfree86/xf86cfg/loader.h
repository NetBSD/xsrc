/*
 * Copyright (c) 2000 by Conectiva S.A. (http://www.conectiva.com)
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
 * CONECTIVA LINUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 * Except as contained in this notice, the name of Conectiva Linux shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from
 * Conectiva Linux.
 *
 * Author: Paulo César Pereira de Andrade <pcpa@conectiva.com.br>
 *
 * $XFree86: xc/programs/Xserver/hw/xfree86/xf86cfg/loader.h,v 1.1 2000/10/20 14:59:05 alanh Exp $
 */
#ifdef USE_MODULES
#include "config.h"
#include "stubs.h"

#ifndef _xf86cfg_loader_h
#define _xf86cfg_loader_h

#ifdef LOADER_PRIVATE
#include <sym.h>

#define XFree86LOADER		/* not really */
#include <xf86_ansic.h>

/* common/xf86Module.h */
pointer LoadModule(const char *, const char *, const char **,
                   const char **, pointer, const pointer *,
                   int *, int *);
pointer LoadSubModule(pointer, const char *, const char **,
                      const char **, pointer, const pointer *,
                      int *, int *);
void UnloadSubModule(pointer);
void LoadFont(pointer);
void UnloadModule (pointer);
pointer LoaderSymbol(const char *);
char **LoaderListDirs(const char **, const char **);
void LoaderFreeDirList(char **);
void LoaderErrorMsg(const char *, const char *, int, int);
void LoadExtension(pointer, Bool);
void LoaderRefSymLists(const char **, ...);
void LoaderRefSymbols(const char *, ...);
void LoaderReqSymLists(const char **, ...);
void LoaderReqSymbols(const char *, ...);
int LoaderCheckUnresolved(int);
void LoaderGetOS(const char **name, int *major, int *minor, int *teeny);

typedef pointer (*ModuleSetupProc)(pointer, pointer, int *, int *);
typedef void (*ModuleTearDownProc)(pointer);

/* loader/loader.h */
void LoaderDefaultFunc(void);

/* loader/loaderProcs.h */
typedef struct module_desc {
    struct module_desc *child;
    struct module_desc *sib;
    struct module_desc *parent;
    struct module_desc *demand_next;
    char *name;
    char *filename;
    char *identifier;
    XID client_id;
    int in_use;
    int handle;
    ModuleSetupProc SetupProc;
    ModuleTearDownProc TearDownProc;
    void *TearDownData; /* returned from SetupProc */
    const char *path;
} ModuleDesc, *ModuleDescPtr;

void LoaderInit(void);

ModuleDescPtr LoadDriver(const char *, const char *, int, pointer, int *,
                         int *);
ModuleDescPtr DuplicateModule(ModuleDescPtr mod, ModuleDescPtr parent);
void UnloadDriver (ModuleDescPtr);
void FreeModuleDesc (ModuleDescPtr mod);
ModuleDescPtr NewModuleDesc (const char *);
ModuleDescPtr AddSibling (ModuleDescPtr head, ModuleDescPtr new);
void LoaderSetPath(const char *path);
void LoaderSortExtensions(void);
#endif /* LOADER_PRIVATE */

/* common/xf86Opt.h */
typedef struct {
    double freq;
    int units;
} OptFrequency;

typedef union {
    unsigned long       num;
    char *              str;
    double              realnum;
    Bool		bool;
    OptFrequency	freq;
} ValueUnion;

typedef enum {
    OPTV_NONE = 0,
    OPTV_INTEGER,
    OPTV_STRING,                /* a non-empty string */
    OPTV_ANYSTR,                /* Any string, including an empty one */
    OPTV_REAL,
    OPTV_BOOLEAN,
    OPTV_FREQ
} OptionValueType;

typedef enum {
    OPTUNITS_HZ = 1,
    OPTUNITS_KHZ,
    OPTUNITS_MHZ
} OptFreqUnits;

typedef struct {
    int                 token;
    const char*         name;
    OptionValueType     type;
    ValueUnion          value;
    Bool                found;
} OptionInfoRec, *OptionInfoPtr;

#ifdef LOADER_PRIVATE
/* common/xf86str.h */
typedef struct _DriverRec {
    int			driverVersion;
    char *		driverName;
    void		(*Identify)(int flags);
    Bool		(*Probe)(struct _DriverRec *drv, int flags);
    OptionInfoPtr	(*AvailableOptions)(int chipid, int bustype);
    void *		module;
    int			refCount;
} DriverRec, *DriverPtr;
#endif /* LOADER_PRIVATE */

typedef struct _xf86cfgDriverOptions {
    char *name;
    OptionInfoPtr option;
    struct _xf86cfgDriverOptions *next;
} xf86cfgDriverOptions;

extern xf86cfgDriverOptions *video_driver_info;

Bool LoaderInitializeOptions(void);
#endif /* USE_MODULES */

#endif /* _xf86cfg_loader_h */
