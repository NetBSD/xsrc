XCOMM!SHELL_CMD
XHASH $NetBSD: xinitrc.cpp,v 1.19 2022/05/27 19:05:28 nia Exp $

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

fontsize=$(/usr/X11R7/libexec/ctwm_font_size)
if ! [ -n "$fontsize" ]; then
	fontsize=16
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
else
    XRDB -merge - <<EOF
XHASH ifdef COLOR
*customization: -color
XHASH endif
*VT100.foreground: grey90
*VT100.background: black
*SimpleMenu*font:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
*SimpleMenu*menuLabel.font:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Bitmap*font:    -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Editres*font:   -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Viewres*font:   -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
XCalc*font:     -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
XClipboard*font:        -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
XConsole*font:  -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
XFontSel*font:  -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
XLoad*font:     -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xedit*font:     -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xfd*font:       -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xgc*font:       -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xmag*font:      -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xmessage*font:  -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xmh*font:       -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
EOF
if [ $fontsize -gt 18 ]; then
    XRDB -merge - <<EOF
Xft.dpi: $(/usr/bin/printf '96 * (%d / 16)\n' "$fontsize" | /usr/bin/bc /dev/stdin)
*VT100.faceName: xft:Monospace:pixelsize=$fontsize
EOF
elif [ $fontsize -gt 13 ]; then
    XRDB -merge - <<EOF
*VT100.font: -misc-fixed-medium-r-normal-*-18-*-*-*-*-*-iso10646-1
*VT100.fontBold: -misc-fixed-bold-r-normal-*-18-*-*-*-*-*-iso10646-1
*VT100.utf8Fonts.font: -misc-fixed-medium-r-normal-*-18-*-*-*-*-*-iso10646-1
*VT100.utf8Fonts.fontBold: -misc-fixed-bold-r-normal-*-18-*-*-*-*-*-iso10646-1
EOF
else
    XRDB -merge - <<EOF
*VT100.font: -misc-fixed-medium-r-normal-*-13-*-*-*-*-*-iso10646-1
*VT100.fontBold: -misc-fixed-bold-r-normal-*-13-*-*-*-*-*-iso10646-1
*VT100.utf8Fonts.font: -misc-fixed-medium-r-normal-*-13-*-*-*-*-*-iso10646-1
*VT100.utf8Fonts.fontBold: -misc-fixed-bold-r-normal-*-13-*-*-*-*-*-iso10646-1
EOF
fi
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

XSETROOT -cursor_name left_ptr
XSETROOT -solid 'rgb:00/22/44'
XCLOCK -digital -strftime '%a %Y-%m-%d %H:%M' \
	-face "spleen:pixelsize=$fontsize" -g +0+0 &
UXTERM &
exec CTWM -W
