XCOMM!/bin/sh

XCOMM $Xorg: startx.cpp,v 1.3 2000/08/17 19:54:29 cpqbld Exp $
XCOMM 
XCOMM This is just a sample implementation of a slightly less primitive 
XCOMM interface than xinit.  It looks for user .xinitrc and .xserverrc
XCOMM files, then system xinitrc and xserverrc files, else lets xinit choose
XCOMM its default.  The system xinitrc should probably do things like check
XCOMM for .Xresources files and merge them in, startup up a window manager,
XCOMM and pop a clock and serveral xterms.
XCOMM
XCOMM Site administrators are STRONGLY urged to write nicer versions.
XCOMM 
XCOMM $XFree86: xc/programs/xinit/startx.cpp,v 3.8 2001/04/27 11:04:53 dawes Exp $

#ifdef SCO

XCOMM Check for /usr/bin/X11 and BINDIR in the path, if not add them.
XCOMM This allows startx to be placed in a place like /usr/bin or /usr/local/bin
XCOMM and people may use X without changing their PATH

XCOMM First our compiled path

bindir=BINDIR
if expr $PATH : ".*`echo $bindir | sed 's?/?\\/?g'`.*" > /dev/null 2>&1; then
	:
else
	PATH=$PATH:BINDIR
fi

XCOMM Now the "SCO" compiled path

if expr $PATH : '.*\/usr\/bin\/X11.*' > /dev/null 2>&1; then
	:
else
	PATH=$PATH:/usr/bin/X11
fi

XCOMM Set up the XMERGE env var so that dos merge is happy under X

if [ -f /usr/lib/merge/xmergeset.sh ]; then
	. /usr/lib/merge/xmergeset.sh
else if [ -f /usr/lib/merge/console.disp ]; then
	XMERGE=`cat /usr/lib/merge/console.disp`
	export XMERGE
fi
fi

scoclientrc=$HOME/.startxrc
#endif

userclientrc=$HOME/.xinitrc
userserverrc=$HOME/.xserverrc
sysclientrc=XINITDIR/xinitrc
sysserverrc=XINITDIR/xserverrc
defaultclientargs=""
defaultserverargs=""
clientargs=""
serverargs=""

#ifdef SCO
if [ -f $scoclientrc ]; then
    defaultclientargs=$scoclientrc
else
#endif
if [ -f $userclientrc ]; then
    defaultclientargs=$userclientrc
else if [ -f $sysclientrc ]; then
    defaultclientargs=$sysclientrc
fi
fi
#ifdef SCO
fi
#endif

if [ -f $userserverrc ]; then
    defaultserverargs=$userserverrc
else if [ -f $sysserverrc ]; then
    defaultserverargs=$sysserverrc
fi
fi

display=:0
whoseargs="client"
while [ "x$1" != "x" ]; do
    case "$1" in
    --)
	whoseargs="server"
	;;
    *)
	if [ "$whoseargs" = "client" ]; then
	    clientargs="$clientargs $1"
	else
	    serverargs="$serverargs $1"
	    case "$1" in
	    :[0-9]*)
		display="$1"
		;;
	    esac
	fi
	;;
    esac
    shift
done

if [ x"$clientargs" = x ]; then
    clientargs="$defaultclientargs"
fi
if [ x"$serverargs" = x ]; then
    serverargs="$defaultserverargs"
fi
    
if [ X"$XAUTHORITY" = X ]; then
    export XAUTHORITY=$HOME/.Xauthority
fi

removelist=

#if defined(HAS_COOKIE_MAKER) && defined(MK_COOKIE)
XCOMM set up default Xauth info for this machine
#ifndef HOSTNAME
#ifdef __linux__
#define HOSTNAME hostname -f
#else
#define HOSTNAME hostname
#endif
#endif
mcookie=`MK_COOKIE`
for displayname in $display `HOSTNAME`$display; do
    if ! xauth list "$displayname" | grep "$displayname " >/dev/null 2>&1; then
	xauth add $displayname . $mcookie
	removelist="$displayname $removelist"
    fi
done
#endif

xinit $clientargs -- $serverargs

if [ x"$removelist" != x ]; then
    xauth remove $removelist
fi

/*
 * various machines need special cleaning up
 */
#ifdef __linux__
if command -v deallocvt > /dev/null 2>&1; then
    deallocvt
fi
#endif

#ifdef macII
Xrepair
screenrestore
#endif

#if defined(sun) && !defined(i386)
kbd_mode -a
#endif
