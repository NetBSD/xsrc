/*
 * (c) Copyright 2000 by Peter Busch
 *                      <pbusch@dfki.de>
 *
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
 * PETER BUSCH  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Peter Busch shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from Peter Busch.
 *
*/
/* $XFree86: xc/programs/Xserver/hw/xwin/xf_dx.h,v 1.1 2000/08/10 17:40:38 dawes Exp $ */

#ifndef __XF_DX_H__
#define __XF_DX_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef BOOL (__cdecl *_DIInitKeyboard)();

typedef BOOL (__cdecl *_DIInitMouse)();

typedef void (__cdecl *_DITermKeyboard)();

typedef void (__cdecl *_DITermMouse)();

typedef BOOL (__cdecl *_DXStoreColors)( int i, int r, int g, int b );

typedef void (__cdecl *_ListModes)();

typedef char*( __cdecl *_AllocateFramebuffer)
	(
	int*		width,
	int*		height,
	int*		depth,
	int*		pitch
	);

typedef void (__cdecl *_winfbBlockHandler)();

typedef int  (__cdecl *_winfbWakeupHandler)();

typedef void (__cdecl *_SwitchFramebuffer)( void * ptr );

typedef void (__cdecl *_GenerateInputEvent)( 
	unsigned long type,
	int ix,
	int iy,
	int button
	); 

typedef void (__cdecl *_set_fn)( 
	_SwitchFramebuffer switch_fb, 
	_GenerateInputEvent gen_inp
	);


/*
* interface of the xf_dx_library
*/

typedef struct _xf_dx_rec
{
	_DIInitKeyboard			xf_dx_init_keybd;
	_DIInitMouse			xf_dx_init_mouse;
	_DITermKeyboard			xf_dx_term_keybd;
	_DITermMouse			xf_dx_term_mouse;
	_DXStoreColors			xf_dx_store_colors;
	_ListModes				xf_dx_list_modes;
	_AllocateFramebuffer	xf_dx_alloc_fb;
	_winfbBlockHandler		xf_dx_block_proc;
	_winfbWakeupHandler		xf_dx_wakeup_proc;
	_SwitchFramebuffer		xf_dx_switch_buffer;
	_GenerateInputEvent		xf_dx_gen_input; 
	_set_fn					xf_dx_set_fn;
} xf_dx_rec, *xf_dx_ptr;

int __cdecl xf_dx_init( xf_dx_ptr *dx_ptr );


#ifdef __cplusplus
} /* extern "C" { */
#endif


#endif /* __XF_DX_H__ */
