/*
 * Mesa 3-D graphics library
 *
 * Copyright (C) 1999-2008  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


/*
 * This file manages the OpenGL API dispatch layer.
 * The dispatch table (struct _glapi_table) is basically just a list
 * of function pointers.
 * There are functions to set/get the current dispatch table for the
 * current thread and to manage registration/dispatch of dynamically
 * added extension functions.
 *
 * It's intended that this file and the other glapi*.[ch] files are
 * flexible enough to be reused in several places:  XFree86, DRI-
 * based libGL.so, and perhaps the SGI SI.
 *
 * NOTE: There are no dependencies on Mesa in this code.
 *
 * Versions (API changes):
 *   2000/02/23  - original version for Mesa 3.3 and XFree86 4.0
 *   2001/01/16  - added dispatch override feature for Mesa 3.5
 *   2002/06/28  - added _glapi_set_warning_func(), Mesa 4.1.
 *   2002/10/01  - _glapi_get_proc_address() will now generate new entrypoints
 *                 itself (using offset ~0).  _glapi_add_entrypoint() can be
 *                 called afterward and it'll fill in the correct dispatch
 *                 offset.  This allows DRI libGL to avoid probing for DRI
 *                 drivers!  No changes to the public glapi interface.
 */

#include "c11/threads.h"
#include "util/u_thread.h"
#include "u_current.h"

#ifndef MAPI_MODE_UTIL

#include "table.h"
#include "stub.h"

#else

extern void init_glapi_relocs_once(void);
extern void (*__glapi_noop_table[])(void);

#define table_noop_array __glapi_noop_table
#define stub_init_once() init_glapi_relocs_once()

#endif

/**
 * \name Current dispatch and current context control variables
 *
 * Depending on whether or not multithreading is support, and the type of
 * support available, several variables are used to store the current context
 * pointer and the current dispatch table pointer.  In the non-threaded case,
 * the variables \c _glapi_Dispatch and \c _glapi_Context are used for this
 * purpose.
 *
 * In the "normal" threaded case, the variables \c _glapi_Dispatch and
 * \c _glapi_Context will be \c NULL if an application is detected as being
 * multithreaded.  Single-threaded applications will use \c _glapi_Dispatch
 * and \c _glapi_Context just like the case without any threading support.
 * When \c _glapi_Dispatch and \c _glapi_Context are \c NULL, the thread state
 * data \c _gl_DispatchTSD and \c ContextTSD are used.  Drivers and the
 * static dispatch functions access these variables via \c _glapi_get_dispatch
 * and \c _glapi_get_context.
 *
 * There is a race condition in setting \c _glapi_Dispatch to \c NULL.  It is
 * possible for the original thread to be setting it at the same instant a new
 * thread, perhaps running on a different processor, is clearing it.  Because
 * of that, \c ThreadSafe, which can only ever be changed to \c GL_TRUE, is
 * used to determine whether or not the application is multithreaded.
 * 
 * In the TLS case, the variables \c _glapi_Dispatch and \c _glapi_Context are
 * hardcoded to \c NULL.  Instead the TLS variables \c _glapi_tls_Dispatch and
 * \c _glapi_tls_Context are used.  Having \c _glapi_Dispatch and
 * \c _glapi_Context be hardcoded to \c NULL maintains binary compatability
 * between TLS enabled loaders and non-TLS DRI drivers.
 */
/*@{*/
#if defined(USE_ELF_TLS)

__THREAD_INITIAL_EXEC struct _glapi_table *u_current_table
#if defined(__NetBSD__)
    = NULL; /* non-zero initializers not supported with dlopen */
#else
    = (struct _glapi_table *) table_noop_array;
#endif

__THREAD_INITIAL_EXEC void *u_current_context;

#else

struct _glapi_table *u_current_table =
   (struct _glapi_table *) table_noop_array;
void *u_current_context;

tss_t u_current_table_tsd;
static tss_t u_current_context_tsd;
static int ThreadSafe;

#endif /* defined(USE_ELF_TLS) */
/*@}*/


void
u_current_destroy(void)
{
#if !defined(USE_ELF_TLS)
   tss_delete(u_current_table_tsd);
   tss_delete(u_current_context_tsd);
#endif
}


#if !defined(USE_ELF_TLS)

static void
u_current_init_tsd(void)
{
   tss_create(&u_current_table_tsd, NULL);
   tss_create(&u_current_context_tsd, NULL);
}

/**
 * Mutex for multithread check.
 */
static mtx_t ThreadCheckMutex = _MTX_INITIALIZER_NP;

static thread_id knownID;

/**
 * We should call this periodically from a function such as glXMakeCurrent
 * in order to test if multiple threads are being used.
 */
void
u_current_init(void)
{
   static int firstCall = 1;

   if (ThreadSafe)
      return;

   mtx_lock(&ThreadCheckMutex);
   if (firstCall) {
      u_current_init_tsd();

      knownID = util_get_thread_id();
      firstCall = 0;
   }
   else if (!util_thread_id_equal(knownID, util_get_thread_id())) {
      ThreadSafe = 1;
      u_current_set_table(NULL);
      u_current_set_context(NULL);
   }
   mtx_unlock(&ThreadCheckMutex);
}

#else

void
u_current_init(void)
{
}

#endif



/**
 * Set the current context pointer for this thread.
 * The context pointer is an opaque type which should be cast to
 * void from the real context pointer type.
 */
void
u_current_set_context(const void *ptr)
{
   u_current_init();

#if defined(USE_ELF_TLS)
   u_current_context = (void *) ptr;
#else
   tss_set(u_current_context_tsd, (void *) ptr);
   u_current_context = (ThreadSafe) ? NULL : (void *) ptr;
#endif
}

/**
 * Get the current context pointer for this thread.
 * The context pointer is an opaque type which should be cast from
 * void to the real context pointer type.
 */
void *
u_current_get_context_internal(void)
{
#if defined(USE_ELF_TLS)
   return u_current_context;
#else
   if (ThreadSafe)
      return tss_get(u_current_context_tsd);
   else if (!util_thread_id_equal(knownID, util_get_thread_id()))
      return NULL;
   else
      return u_current_context;
#endif
}

/**
 * Set the global or per-thread dispatch table pointer.
 * If the dispatch parameter is NULL we'll plug in the no-op dispatch
 * table (__glapi_noop_table).
 */
void
u_current_set_table(const struct _glapi_table *tbl)
{
   u_current_init();

   stub_init_once();

   if (!tbl)
      tbl = (const struct _glapi_table *) table_noop_array;

#if defined(USE_ELF_TLS)
   u_current_table = (struct _glapi_table *) tbl;
#else
   tss_set(u_current_table_tsd, (void *) tbl);
   u_current_table = (ThreadSafe) ? NULL : (void *) tbl;
#endif
}

/**
 * Return pointer to current dispatch table for calling thread.
 */
struct _glapi_table *
u_current_get_table_internal(void)
{
#if defined(USE_ELF_TLS)
#  if defined(__NetBSD__)
   return (likely(u_current_table) ? u_current_table : (struct _glapi_table *) table_noop_array);
#  else
   return u_current_table;
#  endif
#else
   if (ThreadSafe)
      return (struct _glapi_table *) tss_get(u_current_table_tsd);
   else if (!util_thread_id_equal(knownID, util_get_thread_id()))
      return (struct _glapi_table *) table_noop_array;
   else
      return (struct _glapi_table *) u_current_table;
#endif
}
