/*
* Copyright 2000 (C) Peter Busch
* All rights reserved.
NO WARRANTY; NO LIABILITY FOR DAMAGES:  THE MATERIALS ARE PROVIDED "AS IS" 
WITHOUT ANY EXPRESS OR IMPLIED WARRANTY OF ANY KIND INCLUDING WARRANTIES
OF SATISFACTORY QUALITY, MERCHANTABILITY, NONINFRINGEMENT OF THIRD-PARTY
INTELLECTUAL PROPERTY, OR FITNESS FOR ANY PARTICULAR PURPOSE.
IN NO EVENT  SHALL Peter Busch BE LIABLE FOR ANY DAMAGES WHATSOEVER
(INCLUDING, WITHOUT  LIMITATION, DIRECT OR INDIRECT DAMAGES, DAMAGES FOR
LOSS OF PROFITS, BUSINESS  INTERRUPTION, LOSS OF INFORMATION) ARISING OUT
OF THE USE OF OR INABILITY TO  USE THE MATERIALS, EVEN IF
Peter Busch HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH  DAMAGES.
Without limiting the generality of the foregoing, no warranty is made
that the enclosed software will generate computer programs with the
characteristics or specifications desired by you or that the demo
software will be error-free.
THESE DISCLAIMERS OF WARRANTY CONSTITUTE AN ESSENTIAL PART OF THIS
AGREEMENT. 
*/
/* $XFree86: xc/programs/Xserver/hw/vfb/ntux_xf.c,v 3.1 2000/08/10 17:40:31 dawes Exp $ */

/*
* ntux_xf.c
* support code for the output enabled X vfb server
* contains the interface between the vfb source files and the ntux_ddx library
* Copyright (C) Peter Busch
* All rights reserved.
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* This code is BETA and will be part of the CYGWIN/XFREE Project.
* It may not be used for purposes other than to contribute to the 
* CYGWIN/XFree project without prior written notice from the
* author.
* Any commercial use of this code will be permitted
* provided you contact Peter Busch at pbusch@dfki.de
*/



#include <windows.h>
#include <stdio.h>
#include "keysym.h"


typedef char *	(* _get_surface)( unsigned long size );
typedef int		(* _init_input)( void );
typedef void	(* _init_mouse)( );
typedef void	(* _init_keyboard)( );
typedef void	(* _term_mouse)( );
typedef void	(* _term_keyboard)( );
typedef void	(* _get_WinMap)( char *pKeySyms, unsigned char *p_modMap );


HINSTANCE h_ntux_ddx = NULL;

_get_surface	ntux_surface = NULL;
_init_input		init_input = NULL;
_init_mouse		ntux_init_mouse = NULL;
_init_keyboard	ntux_init_keyboard = NULL;
_term_mouse		ntux_term_mouse = NULL;
_term_keyboard	ntux_term_keyboard = NULL;
_get_WinMap		ntux_getWinMap = NULL;

int	enable_ntux_xf( )
{
	h_ntux_ddx = LoadLibrary( "ntux_ddx.dll" );
	if( h_ntux_ddx == NULL )
		return 0;

	ntux_surface = (_get_surface)GetProcAddress( h_ntux_ddx, "get_surface" );
	init_input = (_init_input)GetProcAddress( h_ntux_ddx, "init_input" );
	ntux_init_mouse = (_init_mouse)GetProcAddress( h_ntux_ddx, "init_mouse" );
	ntux_init_keyboard = (_init_keyboard)GetProcAddress( h_ntux_ddx, "init_keyboard" );
	ntux_term_mouse = (_term_mouse)GetProcAddress( h_ntux_ddx, "term_mouse" );
	ntux_term_keyboard = (_term_keyboard)GetProcAddress( h_ntux_ddx, "term_keyboard" );
	ntux_getWinMap = (_get_WinMap)GetProcAddress( h_ntux_ddx, "get_WinMap" );

	OutputDebugString( "\nntux_ddx successfully loaded\n\n" );
	return 1;
}


char *get_framebuf( unsigned long size )
{
	char	msg[256];
	if( ntux_surface != NULL )
	{
		fprintf( stderr, "get_framebuf : calling %08x with size = %08x\n",
					ntux_surface, size
					);
		return ntux_surface( size );
	}
	return NULL;
}


void init_mouse( )
{
	if( ntux_init_mouse != NULL )
		return ntux_init_mouse();
	return ;
}

void init_keyboard( )
{
	if( ntux_init_keyboard != NULL )
		return ntux_init_keyboard();
	return ;
}

void term_mouse( )
{
	if( ntux_term_mouse != NULL )
		return ntux_term_mouse();
	return ;
}

void term_keyboard( )
{
	if( ntux_term_keyboard != NULL )
		return ntux_term_keyboard();
	return ;
}

void get_WinMappings( char *pKeySyms, unsigned char *p_modMap )
{
	if( ntux_getWinMap != NULL )
		return ntux_getWinMap( pKeySyms, p_modMap );
	return ;
}



