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
 * $XFree86: xc/programs/Xserver/hw/xfree86/xf86cfg/loader.c,v 1.5.2.1 2001/05/28 21:36:44 paulo Exp $
 */
#define LOADER_PRIVATE
#include "loader.h"

/* XXX beware (or fix it) libc functions called here are the xf86 ones */

#ifdef USE_MODULES
static void AddModuleOptions(char*, OptionInfoPtr);
void xf86AddDriver(DriverPtr, void*, int);
Bool xf86ServerIsOnlyDetecting(void);
void xf86AddInputDriver(InputDriverPtr, pointer, int);
void xf86AddModuleInfo(ModuleInfoPtr, void*);
Bool xf86LoaderCheckSymbol(const char*);
void xf86LoaderReqSymLists(const char **, ...);
void xf86Msg(int, const char*, ...);

xf86cfgModuleOptions *module_options;

int xf86ShowUnresolved = 1;

LOOKUP miLookupTab[]      = {{0,0}};
LOOKUP dixLookupTab[]     = {{0,0}};
LOOKUP fontLookupTab[]    = {{0,0}};
LOOKUP extLookupTab[]     = {{0,0}};
LOOKUP xfree86LookupTab[] = {
       /* Loader functions */
   SYMFUNC(LoaderDefaultFunc)
   SYMFUNC(LoadSubModule)
   SYMFUNC(DuplicateModule)
   SYMFUNC(LoaderErrorMsg)
   SYMFUNC(LoaderCheckUnresolved)
   SYMFUNC(LoadExtension)
   SYMFUNC(LoadFont)
   SYMFUNC(LoaderReqSymbols)
   SYMFUNC(LoaderReqSymLists)
   SYMFUNC(LoaderRefSymbols)
   SYMFUNC(LoaderRefSymLists)
   SYMFUNC(UnloadSubModule)
   SYMFUNC(LoaderSymbol)
   SYMFUNC(LoaderListDirs)
   SYMFUNC(LoaderFreeDirList)
   SYMFUNC(LoaderGetOS)

   /*
    * these here are our own interfaces to libc functions
    */
   SYMFUNC(xf86abort)
   SYMFUNC(xf86abs)
   SYMFUNC(xf86acos)
   SYMFUNC(xf86asin)
   SYMFUNC(xf86atan)
   SYMFUNC(xf86atan2)
   SYMFUNC(xf86atof)
   SYMFUNC(xf86atoi)
   SYMFUNC(xf86atol)
   SYMFUNC(xf86bsearch)
   SYMFUNC(xf86ceil)
   SYMFUNC(xf86calloc)
   SYMFUNC(xf86clearerr)
   SYMFUNC(xf86close)
   SYMFUNC(xf86cos)
   SYMFUNC(xf86exit)
   SYMFUNC(xf86exp)
   SYMFUNC(xf86fabs)
   SYMFUNC(xf86fclose)
   SYMFUNC(xf86feof)
   SYMFUNC(xf86ferror)
   SYMFUNC(xf86fflush)
   SYMFUNC(xf86fgetc)
   SYMFUNC(xf86fgetpos)
   SYMFUNC(xf86fgets)
   SYMFUNC(xf86floor)
   SYMFUNC(xf86fmod)
   SYMFUNC(xf86fopen)
   SYMFUNC(xf86fprintf)
   SYMFUNC(xf86fputc)
   SYMFUNC(xf86fputs)
   SYMFUNC(xf86fread)
   SYMFUNC(xf86free)
   SYMFUNC(xf86freopen)
   SYMFUNC(xf86frexp)
   SYMFUNC(xf86fscanf)
   SYMFUNC(xf86fseek)
   SYMFUNC(xf86fsetpos)
   SYMFUNC(xf86ftell)
   SYMFUNC(xf86fwrite)
   SYMFUNC(xf86getc)
   SYMFUNC(xf86getenv)
   SYMFUNC(xf86getpagesize)
   SYMFUNC(xf86hypot)
   SYMFUNC(xf86ioctl)
   SYMFUNC(xf86isalnum)
   SYMFUNC(xf86isalpha)
   SYMFUNC(xf86iscntrl)
   SYMFUNC(xf86isdigit)
   SYMFUNC(xf86isgraph)
   SYMFUNC(xf86islower)
   SYMFUNC(xf86isprint)
   SYMFUNC(xf86ispunct)
   SYMFUNC(xf86isspace)
   SYMFUNC(xf86isupper)
   SYMFUNC(xf86isxdigit)
   SYMFUNC(xf86labs)
   SYMFUNC(xf86ldexp)
   SYMFUNC(xf86log)
   SYMFUNC(xf86log10)
   SYMFUNC(xf86lseek)
   SYMFUNC(xf86malloc)
   SYMFUNC(xf86memchr)
   SYMFUNC(xf86memcmp)
   SYMFUNC(xf86memcpy)
#if (defined(__powerpc__) && (defined(Lynx) || defined(linux))) || defined(__sparc__) || defined(__ia64__)
   /*
    * Some PPC, SPARC, and IA64 compilers generate calls to memcpy to handle
    * structure copies.  This causes a problem both here and in shared
    * libraries as there is no way to map the name of the call to the
    * correct function.
    */
   SYMFUNC(memcpy)
   /*
    * Some PPC, SPARC, and IA64 compilers generate calls to memset to handle 
    * aggregate initializations.
    */
   SYMFUNC(memset)
#endif
   SYMFUNC(xf86memmove)
   SYMFUNC(xf86memset)
   SYMFUNC(xf86mmap)
   SYMFUNC(xf86modf)
   SYMFUNC(xf86munmap)
   SYMFUNC(xf86open)
   SYMFUNC(xf86perror)
   SYMFUNC(xf86pow)
   SYMFUNC(xf86printf)
   SYMFUNC(xf86qsort)
   SYMFUNC(xf86read)
   SYMFUNC(xf86realloc)
   SYMFUNC(xf86remove)
   SYMFUNC(xf86rename)
   SYMFUNC(xf86rewind)
   SYMFUNC(xf86setbuf)
   SYMFUNC(xf86setvbuf)
   SYMFUNC(xf86sin)
   SYMFUNC(xf86snprintf)
   SYMFUNC(xf86sprintf)
   SYMFUNC(xf86sqrt)
   SYMFUNC(xf86sscanf)
   SYMFUNC(xf86strcat)
   SYMFUNC(xf86strcmp)
   SYMFUNC(xf86strcasecmp)
   SYMFUNC(xf86strcpy)
   SYMFUNC(xf86strcspn)
   SYMFUNC(xf86strerror)
   SYMFUNC(xf86strlen)
   SYMFUNC(xf86strncmp)
   SYMFUNC(xf86strncasecmp)
   SYMFUNC(xf86strncpy)
   SYMFUNC(xf86strpbrk)
   SYMFUNC(xf86strchr)
   SYMFUNC(xf86strrchr)
   SYMFUNC(xf86strspn)
   SYMFUNC(xf86strstr)
   SYMFUNC(xf86strtod)
   SYMFUNC(xf86strtok)
   SYMFUNC(xf86strtol)
   SYMFUNC(xf86strtoul)
   SYMFUNC(xf86tan)
   SYMFUNC(xf86tmpfile)
   SYMFUNC(xf86tolower)
   SYMFUNC(xf86toupper)
   SYMFUNC(xf86ungetc)
   SYMFUNC(xf86vfprintf)
   SYMFUNC(xf86vsnprintf)
   SYMFUNC(xf86vsprintf)
   SYMFUNC(xf86write)
  
/* non-ANSI C functions */
   SYMFUNC(xf86opendir)
   SYMFUNC(xf86closedir)
   SYMFUNC(xf86readdir)
   SYMFUNC(xf86rewinddir)
   SYMFUNC(xf86ffs)
   SYMFUNC(xf86strdup)
   SYMFUNC(xf86bzero)
   SYMFUNC(xf86usleep)
   SYMFUNC(xf86execl)

   SYMFUNC(xf86getsecs)
   SYMFUNC(xf86fpossize)      /* for returning sizeof(fpos_t) */

   SYMFUNC(xf86stat)
   SYMFUNC(xf86fstat)
   SYMFUNC(xf86access)
   SYMFUNC(xf86geteuid)
   SYMFUNC(xf86getegid)
   SYMFUNC(xf86getpid)
   SYMFUNC(xf86mknod)
   SYMFUNC(xf86chmod)
   SYMFUNC(xf86chown)
   SYMFUNC(xf86sleep)
   SYMFUNC(xf86mkdir)
   SYMFUNC(xf86shmget)
   SYMFUNC(xf86shmat)
   SYMFUNC(xf86shmdt)
   SYMFUNC(xf86shmctl)
   SYMFUNC(xf86setjmp)
   SYMFUNC(xf86longjmp)

    SYMFUNC(xf86AddDriver)
    SYMFUNC(xf86ServerIsOnlyDetecting)
    SYMFUNC(xf86AddInputDriver)
    SYMFUNC(xf86AddModuleInfo)
    SYMFUNC(xf86LoaderCheckSymbol)

    SYMFUNC(xf86LoaderReqSymLists)
    SYMFUNC(xf86Msg)
    SYMFUNC(ErrorF)
    {0,0}
};

static DriverPtr driver;
static ModuleInfoPtr info;
static ModuleType type = GenericModule;

static void
AddModuleOptions(char *name, OptionInfoPtr option)
{
    xf86cfgModuleOptions *ptr;
    OptionInfoPtr tmp;

    ptr = XtNew(xf86cfgModuleOptions);
    ptr->name = XtNewString(name);
    ptr->type = type;
    if (option) {
	int count;

	for (count = 0, tmp = option; tmp->name != NULL; tmp++, count++)
	    ;
	++count;
	ptr->option = (XtPointer)XtCalloc(1, count * sizeof(OptionInfoRec));
	for (count = 0, tmp = option; tmp->name != NULL; count++, tmp++) {
	    memcpy(&ptr->option[count], tmp, sizeof(OptionInfoRec));
	    ptr->option[count].name = XtNewString(tmp->name);
	    if (tmp->type == OPTV_STRING || tmp->type == OPTV_ANYSTR)
		ptr->option[count].value.str = XtNewString(tmp->value.str);
	}
    }
    else
	ptr->option = NULL;

    ptr->next = module_options;
    module_options = ptr;
}

Bool
LoaderInitializeOptions(void)
{
    static int first = 1;
    static char *path = NULL, *modules = "lib/modules";
    int saveVerbose = xf86Verbose;

    if (first) {
	xf86Verbose = 10;
	LoaderInit();
	first = 0;
    }
    xf86Verbose = saveVerbose;

    if (XF86Module_path == NULL) {
	XF86Module_path = XtMalloc(strlen(XFree86Dir) + strlen(modules) + 2);
	sprintf(XF86Module_path, "%s/%s", XFree86Dir, modules);
    }
    if (path == NULL || strcmp(XF86Module_path, path)) {
	char **list, **l;
	const char *subdirs[] = {
	    ".",
	    "drivers",
	    "input",
	    NULL
	};
	int errmaj, errmin;
	ModuleDescPtr module;

	path = strdup(XF86Module_path);
	LoaderSetPath(path);

	list = LoaderListDirs(subdirs, NULL);
	if (list) {
#if 0
	    if (ptr) {
		while (module_options) {
		    module_options = module_options->next;
		    XtFree(ptr->name);
		    XtFree((XtPointer)ptr->option);
		    XtFree((XtPointer)ptr);
		    ptr = module_options;
		}
	    }
#endif

	    for (l = list; *l; l++) {
		driver = NULL;
		info = NULL;
		type = GenericModule;
		xf86Verbose = 0;
		if ((module = LoadModule(*l, NULL, NULL, NULL, NULL,
					 NULL, &errmaj, &errmin)) == NULL)
		    LoaderErrorMsg(NULL, *l, errmaj, errmin);
		else if (driver && driver->AvailableOptions)
		    AddModuleOptions(*l, (*driver->AvailableOptions)(-1, -1));
		else if (info && info->AvailableOptions)
		    AddModuleOptions(*l, (*info->AvailableOptions)(NULL));

		UnloadModule(module);
		xf86Verbose = saveVerbose;
	    }
	    LoaderFreeDirList(list);
	}
	else {
	    xf86Verbose = saveVerbose;
	    return (False);
	}
    }

    xf86Verbose = saveVerbose;

    return (True);
}

void
xf86AddDriver(DriverPtr drv, void *module, int flags)
{
    driver = drv;
    type = VideoModule;
}

Bool
xf86ServerIsOnlyDetecting(void)
{
    return (True);
}

void
xf86AddInputDriver(InputDriverPtr inp, void *module, int flags)
{
    type = InputModule;
}

void
xf86AddModuleInfo(ModuleInfoPtr inf, void *module)
{
    info = inf;
}

Bool
xf86LoaderCheckSymbol(const char *symbol)
{
    return LoaderSymbol(symbol) != NULL;
}

void
xf86LoaderReqSymLists(const char **list0, ...)
{
}

void xf86Msg(int type, const char *format, ...)
{
}
#endif
