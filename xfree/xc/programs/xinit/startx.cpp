XCOMM!/bin/sh

XCOMM $XConsortium: startx.cpp,v 1.4 91/08/22 11:41:29 rws Exp $
XCOMM $XFree86: xc/programs/xinit/startx.cpp,v 3.5 2000/12/02 18:06:58 herrb Exp $
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
clientargs=""
serverargs=""

#ifdef SCO
if [ -f $scoclientrc ]; then
    clientargs=$scoclientrc
else
#endif
if [ -f $userclientrc ]; then
    clientargs=$userclientrc
else if [ -f $sysclientrc ]; then
    clientargs=$sysclientrc
fi
fi
#ifdef SCO
fi
#endif

if [ -f $userserverrc ]; then
    serverargs=$userserverrc
else if [ -f $sysserverrc ]; then
    serverargs=$sysserverrc
fi
fi

display=:0
whoseargs="client"
while [ "x$1" != "x" ]; do
    case "$1" in
	/''*|\.*)	if [ "$whoseargs" = "client" ]; then
		    if [ "x$clientargs" = x ]; then
			clientargs="$1"
		    else
			clientargs="$clientargs $1"
		    fi
		else
		    if [ "x$serverargs" = x ]; then
			serverargs="$1"
		    else
			serverargs="$serverargs $1"
		    fi
		fi ;;
	--)	whoseargs="server" ;;
	*)	if [ "$whoseargs" = "client" ]; then
		    clientargs="$clientargs $1"
		else
		    case "$1" in
			:[0-9]*) display="$1"; serverargs="$serverargs $1";;
			*) serverargs="$serverargs $1" ;;
		    esac
		fi ;;
    esac
    shift
done

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
if [ X"$XAUTHORITY" = X ]; then
    authfile="$HOME/.Xauthority"
else
    authfile="$XAUTHORITY"
fi
serverargs="$serverargs -auth $authfile"
xauth add $display . $mcookie
xauth add `HOSTNAME`$display . $mcookie
#endif

xinit $clientargs -- $serverargs

/*
 * various machines need special cleaning up
 */
#ifdef macII
Xrepair
screenrestore
#endif

#if defined(sun) && !defined(i386)
kbd_mode -a
#endif
