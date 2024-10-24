/*
 * Copyright © 2017 Gražvydas Ignotas
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* #if defined(MISSING_64BIT_ATOMICS) && defined(HAVE_PTHREAD) */
#include <sys/types.h>
#ifndef __HAVE_ATOMIC64_OPS

#include <stdint.h>
#include <pthread.h>

#if defined(HAVE_FUNC_ATTRIBUTE_WEAK) && !defined(__CYGWIN__)
#define WEAK __attribute__((weak))
#else
#define WEAK
#endif

static pthread_mutex_t sync_mutex = PTHREAD_MUTEX_INITIALIZER;

WEAK long long
__atomic_fetch_add_8(volatile long long *ptr, long long val, int memorder)
{
   long long r;

   pthread_mutex_lock(&sync_mutex);
   *ptr += val;
   r = *ptr;
   pthread_mutex_unlock(&sync_mutex);

   return r;
}

WEAK long long
__atomic_fetch_sub_8(volatile long long *ptr, long long val, int memorder)
{
   long long r;

   pthread_mutex_lock(&sync_mutex);
   *ptr -= val;
   r = *ptr;
   pthread_mutex_unlock(&sync_mutex);

   return r;
}

WEAK long long
__atomic_load_8(const volatile long long *ptr, int memorder)
{
   long long r;

   pthread_mutex_lock(&sync_mutex);
   r = *ptr;
   pthread_mutex_unlock(&sync_mutex);

   return r;
}

#ifndef __clang__
WEAK long long
__sync_val_compare_and_swap_8(volatile long long *ptr, long long oldval, long long newval)
{
   long long r;

   pthread_mutex_lock(&sync_mutex);
   r = *ptr;
   if (*ptr == oldval)
      *ptr = newval;
   pthread_mutex_unlock(&sync_mutex);

   return r;
}
#endif

#endif
