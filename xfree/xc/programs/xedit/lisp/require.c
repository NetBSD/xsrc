/*
 * Copyright (c) 2001 by The XFree86 Project, Inc.
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
 * THE XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project shall
 * not be used in advertising or otherwise to promote the sale, use or other
 * dealings in this Software without prior written authorization from the
 * XFree86 Project.
 *
 * Author: Paulo César Pereira de Andrade
 */

/* $XFree86: xc/programs/xedit/lisp/require.c,v 1.7 2001/10/18 03:15:22 paulo Exp $ */

#include "require.h"

/*
 * Initialization
 */
static char *BadArgumentAt = "bad argument %s, at %s";

/*
 * Implementation
 */
LispObj *
Lisp_Load(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *file = CAR(list), *verbose, *print, *ifdoesnotexist;

    if (!STRING_P(file))
	LispDestroy(mac, BadArgumentAt, LispStrObj(mac, file), fname);

    LispGetKeys(mac, fname, "VERBOSE:PRINT:IF-DOES-NOT-EXIST", CDR(list),
		&verbose, &print, &ifdoesnotexist);

    return (_LispLoadFile(mac, STRPTR(file), fname,
			  verbose != NIL, print != NIL, ifdoesnotexist != NIL));
}

LispObj *
Lisp_Require(LispMac *mac, LispObj *list, char *fname)
{
    LispObj *feat = CAR(list), *nam, *obj;
    char filename[1024], *ext;
    int len;

    if (!STRING_P(feat) && !SYMBOL_P(feat))
	LispDestroy(mac, BadArgumentAt, LispStrObj(mac, feat), fname);

    if (CDR(list) != NIL) {
	nam = CAR(CDR(list));

	if (!STRING_P(nam))
	    LispDestroy(mac, "%s is not of type string, at %s",
			LispStrObj(mac, nam), fname);
    }
    else
	nam = feat;

    for (obj = MOD; obj != NIL; obj = CDR(obj)) {
	if (STRPTR(CAR(obj)) == STRPTR(feat))
	    return (feat);
    }

    if (STRPTR(nam)[0] != '/') {
#ifdef LISPDIR
	snprintf(filename, sizeof(filename), "%s", LISPDIR);
#else
	getcwd(filename, sizeof(filename));
#endif
    }
    else
	filename[0] = '\0';
    *(filename + sizeof(filename) - 5) = '\0';	/* make sure there is place for ext */
    len = strlen(filename);
    if (!len || filename[len - 1] != '/') {
	strcat(filename, "/");
	++len;
    }

    snprintf(filename + len, sizeof(filename) - len - 5, "%s", STRPTR(nam));

    ext = filename + strlen(filename);

#ifdef SHARED_MODULES
    strcpy(ext, ".so");
    if (access(filename, R_OK) == 0) {
	LispModule *module;
	char data[64];
	int len;

	if (mac->module == NULL) {
	    /* export our own symbols */
	    if (dlopen(NULL, RTLD_LAZY | RTLD_GLOBAL) == NULL)
		LispDestroy(mac, dlerror());
	}

#if 0
	if (mac->interactive)
	    fprintf(lisp_stderr, "; Loading %s\n", filename);
#endif
	module = (LispModule*)LispMalloc(mac, sizeof(LispModule));
	if ((module->handle = dlopen(filename, RTLD_LAZY | RTLD_GLOBAL)) == NULL)
	    LispDestroy(mac, dlerror());
	snprintf(data, sizeof(data), "%sLispModuleData", STRPTR(nam));
	if ((module->data = (LispModuleData*)dlsym(module->handle, data)) == NULL) {
	    dlclose(module->handle);
	    LispDestroy(mac, "cannot find LispModuleData for %s",
			LispStrObj(mac, feat));
	}
	LispMused(mac, module);
	module->next = mac->module;
	mac->module = module;
	if (module->data->load)
	    (module->data->load)(mac);

	return (Lisp_Provide(mac, CONS(feat, NIL), "PROVIDE"));
    }
#endif

    strcpy(ext, ".lsp");
    (void)_LispLoadFile(mac, filename, fname, 0, 0, 0);

    return (feat);
}
