XCOMM!SHELL_CMD

userresources=$HOME/.Xresources
usermodmap=$HOME/.Xmodmap
sysresources=XINITDIR/.Xresources
sysmodmap=XINITDIR/.Xmodmap

XCOMM merge in defaults and keymaps

if [ -f $sysresources ]; then
#ifdef __APPLE__
    if [ -x /usr/bin/cpp ] ; then
        XRDB -merge $sysresources
    else
        XRDB -nocpp -merge $sysresources
    fi
#else
    XRDB -merge $sysresources
#endif
fi

if [ -f $sysmodmap ]; then
    XMODMAP $sysmodmap
fi

if [ -f "$userresources" ]; then
#ifdef __APPLE__
    if [ -x /usr/bin/cpp ] ; then
        XRDB -merge "$userresources"
    else
        XRDB -nocpp -merge "$userresources"
    fi
#else
    XRDB -merge "$userresources"
#endif
fi

if [ -f "$usermodmap" ]; then
    XMODMAP "$usermodmap"
fi

XCOMM start some nice programs

if [ -d XINITDIR/xinitrc.d ] ; then
	for f in XINITDIR/xinitrc.d/?*.sh ; do
		[ -x "$f" ] && . "$f"
	done
	unset f
fi

TWM &
XCLOCK -geometry 50x50-1+1 &
XTERM -geometry 80x50+494+51 &
XTERM -geometry 80x20+494-0 &
exec XTERM -geometry 80x66+0+0 -name login
