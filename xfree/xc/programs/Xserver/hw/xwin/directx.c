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
/* $XFree86: xc/programs/Xserver/hw/xwin/directx.c,v 1.1 2000/08/10 17:40:37 dawes Exp $ */


#include <windows.h>
#include <stdio.h>
#include "xf_dx_xwin.h"

/* external references to funcs in InitInput.c and InitOutput.c */
extern SwitchFramebuffer( void * ptr );
extern GenerateInputEvent( type, ix, iy, button ); 

typedef BOOL (__cdecl *_DIInitKeyboard)();
typedef BOOL (__cdecl *_DIInitMouse)();
typedef void (__cdecl *_DITermKeyboard)();
typedef void (__cdecl *_DITermMouse)();
typedef BOOL (__cdecl *_DXStoreColors)(int i, int r, int g, int b);
typedef void (__cdecl *_ListModes)(void);
typedef void (__cdecl *_set_fn)( __SwitchFramebuffer switch_fb, __GenerateInputEvent gen_inp );
typedef char*( __cdecl *_winDXAllocateFramebufferMemory)
	(
	int*		width,
	int*		height,
	int*		depth,
	int*		pitch
	);
typedef void (__cdecl *_winfbBlockHandler)();
typedef int  (__cdecl *_winfbWakeupHandler)();


_DIInitKeyboard xf_DIInitKeyboard = NULL;
_DIInitMouse xf_DIInitMouse = NULL;
_DITermKeyboard xf_DITermKeyboard = NULL;
_DITermMouse xf_DITermMouse = NULL;
_DXStoreColors xf_DXStoreColors = NULL;
_ListModes xf_ListModes = NULL;
_set_fn xf_set_fn = NULL;
_winDXAllocateFramebufferMemory xf_winDXAllocateFramebufferMemory = NULL;
_winfbBlockHandler xf_winfbBlockHandler = NULL;
_winfbWakeupHandler xf_winfbWakeupHandler = NULL;
HINSTANCE h_xf_dx = NULL;

static BOOL initialized = FALSE;


void enable_xf_dx( )
{
	h_xf_dx = LoadLibrary( "xf_dx.dll" );
	if( h_xf_dx == NULL )
	{
		fprintf( stderr, "cannot load xf_dx.dll\n" );
		exit( 1 );
	}

	xf_DIInitKeyboard = 
		(_DIInitKeyboard)GetProcAddress( h_xf_dx, "DIInitKeyboard" );
	xf_DIInitMouse = 
		(_DIInitMouse)GetProcAddress( h_xf_dx, "DIInitMouse" );
	xf_DITermKeyboard = 
		(_DITermKeyboard)GetProcAddress( h_xf_dx, "DITermKeyboard" );
	xf_DITermMouse = 
		(_DITermMouse)GetProcAddress( h_xf_dx, "DITermMouse" );
	xf_DXStoreColors = 
		(_DXStoreColors)GetProcAddress( h_xf_dx, "DXStoreColors" );
	xf_ListModes = 
		(_ListModes)GetProcAddress( h_xf_dx, "ListModes" );
	xf_set_fn = 
		(_set_fn)GetProcAddress( h_xf_dx, "set_fn" );
	xf_winDXAllocateFramebufferMemory = 
		(_winDXAllocateFramebufferMemory)GetProcAddress( h_xf_dx, "winDXAllocateFramebufferMemory" );
	xf_winfbBlockHandler = 
		(_winfbBlockHandler)GetProcAddress( h_xf_dx, "winfbBlockHandler" );
	xf_winfbWakeupHandler = 
		(_winfbWakeupHandler)GetProcAddress( h_xf_dx, "winfbWakeupHandler" );

	xf_set_fn( 
		(__SwitchFramebuffer)SwitchFramebuffer,
		(__GenerateInputEvent)GenerateInputEvent
		);
	initialized = TRUE;
}


/* DX stubs */


BOOL	DIInitKeyboard()
{
	return xf_DIInitKeyboard();
}

BOOL	DIInitMouse()
{
	return xf_DIInitMouse();

}

void	DITermKeyboard()
{
	xf_DITermKeyboard();
}

void	DITermMouse()
{
	xf_DITermMouse();

}

BOOL	DXStoreColors(int i, int r, int g, int b)
{
	return xf_DXStoreColors( i, r, g, b );

}

void	ListModes(void)
{
	enable_xf_dx();
	xf_ListModes();
}

void	set_fn( __SwitchFramebuffer switch_fb, __GenerateInputEvent gen_inp )
{
	return xf_set_fn( switch_fb, gen_inp );
}

char*	winDXAllocateFramebufferMemory
	(
	int*		width,
	int*		height,
	int*		depth,
	int*		pitch
	)
{
	enable_xf_dx();
	return xf_winDXAllocateFramebufferMemory( width, height, depth, pitch );
}

void winfbBlockHandler()
{
	xf_winfbBlockHandler();	
}

int  winfbWakeupHandler()
{
	return xf_winfbWakeupHandler();
}

