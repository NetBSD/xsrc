/* OS/2 REXX */
/* $XFree86: xc/programs/xinit/xinitrc.cmd,v 3.3 1996/08/20 12:33:26 dawes Exp $ */
'@echo off'
env = 'OS2ENVIRONMENT'
x11root = VALUE('X11ROOT',,env)
IF x11root = '' THEN DO
	SAY "The environment variable X11ROOT is not set. XFree86/OS2 won't run without it."
	EXIT
END
home = VALUE('HOME',,env)
IF home = '' THEN home = x11root

userresources = home'\.Xresources'
usermodmap    = home'\.Xmodmap'
sysresources  = x11root'\XFree86\lib\X11\xinit\.Xresources'
sysmodmap     = x11root'\XFree86\lib\X11\xinit\.Xmodmap'
xbitmapdir    = x11root'\XFree86\include\X11\bitmaps'
manpath       = VALUE('MANPATH',,env)

/* merge in defaults */
IF exists(sysresources) THEN
	'xrdb -merge 'sysresources

IF exists(sysmodmap) THEN
	'xmodmap 'sysmodmap

IF exists(userresources) THEN
	'xrdb -merge 'userresources

IF exists(usermodmap) THEN
	'xmodmap 'usermodmap

/* start some nice :-) test programs */
'xsetroot -bitmap 'xbitmapdir'\xos2'
/* also try out the following ones: 
 * 'xsetroot -bitmap 'xbitmapdir'\xfree1'
 * 'xsetroot -bitmap 'xbitmapdir'\xfree2'
 */
'start/min/k "X Clock" xclock -update 1 -geometry 100x100-1+1'
'start/min/k "Login Xterm" xterm -sb -geometry 80x25+0+0 -name login'
IF manpath \= '' THEN
	'start/min/k "X Manual" xman -geometry 100x100-105+1'
/* 'start/min/k "Xterm 1" xterm -sb -geometry 80x50+494+51' */
/* 'start/min/k "Xterm 2" xterm -sb -geometry 80x20+494-0' */
'twm'

EXIT

exists:
'DIR "'arg(1)'" >nul 2>&1'
if rc = 0 THEN RETURN 1
RETURN 0
