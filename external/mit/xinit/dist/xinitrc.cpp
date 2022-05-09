XCOMM!SHELL_CMD
XHASH $NetBSD: xinitrc.cpp,v 1.16 2022/05/09 07:00:15 nia Exp $

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
*VT100.font: -*-spleen-*-*-*-*-$fontsize-*-*-*-*-*-*-*
*VT100.utf8Fonts.font: -*-spleen-*-*-*-*-$fontsize-*-*-*-*-*-*-*
*VT100.font1: -*-spleen-medium-r-*-*-6-*-*-*-*-*-*-*
*VT100.font2: -*-spleen-medium-r-*-*-8-*-*-*-*-*-*-*
*VT100.font3: -*-spleen-medium-r-*-*-12-*-*-*-*-*-*-*
*VT100.font4: -*-spleen-medium-r-*-*-16-*-*-*-*-*-*-*
*VT100.font5: -*-spleen-medium-r-*-*-24-*-*-*-*-*-*-*
*VT100.font6: -*-spleen-medium-r-*-*-32-*-*-*-*-*-*-*
*VT100.font7: -*-spleen-medium-r-*-*-64-*-*-*-*-*-*-*
*VT100.utf8Fonts.font1: -*-spleen-medium-r-*-*-6-*-*-*-*-*-*-*
*VT100.utf8Fonts.font2: -*-spleen-medium-r-*-*-8-*-*-*-*-*-*-*
*VT100.utf8Fonts.font3: -*-spleen-medium-r-*-*-12-*-*-*-*-*-*-*
*VT100.utf8Fonts.font4: -*-spleen-medium-r-*-*-16-*-*-*-*-*-*-*
*VT100.utf8Fonts.font5: -*-spleen-medium-r-*-*-24-*-*-*-*-*-*-*
*VT100.utf8Fonts.font6: -*-spleen-medium-r-*-*-32-*-*-*-*-*-*-*
*VT100.utf8Fonts.font7: -*-spleen-medium-r-*-*-64-*-*-*-*-*-*-*
Xman*font: -*-spleen-*-*-*-*-$fontsize-*-*-*-*-*-*-*
Xman*directoryFontNormal: -*-spleen-*-*-*-*-$fontsize-*-*-*-*-*-*-*
Xman*manualFontNormal: -*-spleen-*-*-*-*-$fontsize-*-*-*-*-*-*-*
Xman*manualFontBold: -*-spleen-*-*-*-*-$fontsize-*-*-*-*-*-*-*
Xman*manualFontItalic: -*-spleen-*-*-*-*-$fontsize-*-*-*-*-*-*-*
EOF
elif [ $fontsize -gt 13 ]; then
    XRDB -merge - <<EOF
*VT100.font: -misc-fixed-medium-r-normal-*-18-*-*-*-*-*-iso10646-1
*VT100.fontBold: -misc-fixed-bold-r-normal-*-18-*-*-*-*-*-iso10646-1
*VT100.utf8Fonts.font: -misc-fixed-medium-r-normal-*-18-*-*-*-*-*-iso10646-1
*VT100.utf8Fonts.fontBold: -misc-fixed-bold-r-normal-*-18-*-*-*-*-*-iso10646-1
Xman*font: -misc-fixed-medium-r-normal-*-18-*-*-*-*-*-iso8859-1
Xman*directoryFontNormal: -misc-fixed-medium-r-normal-*-18-*-*-*-*-*-iso8859-1
Xman*manualFontNormal: -misc-fixed-medium-r-normal-*-18-*-*-*-*-*-iso8859-1
Xman*manualFontBold: -misc-fixed-bold-r-normal-*-18-*-*-*-*-*-iso8859-1
Xman*manualFontItalic: -misc-fixed-bold-r-normal-*-18-*-*-*-*-*-iso8859-1
EOF
else
    XRDB -merge - <<EOF
*VT100.font: -misc-fixed-medium-r-normal-*-13-*-*-*-*-*-iso10646-1
*VT100.fontBold: -misc-fixed-bold-r-normal-*-13-*-*-*-*-*-iso10646-1
*VT100.utf8Fonts.font: -misc-fixed-medium-r-normal-*-13-*-*-*-*-*-iso10646-1
*VT100.utf8Fonts.fontBold: -misc-fixed-bold-r-normal-*-13-*-*-*-*-*-iso10646-1
Xman*font: -misc-fixed-medium-r-normal-*-13-*-*-*-*-*-iso8859-1
Xman*directoryFontNormal: -misc-fixed-medium-r-normal-*-13-*-*-*-*-*-iso8859-1
Xman*manualFontNormal: -misc-fixed-medium-r-normal-*-13-*-*-*-*-*-iso8859-1
Xman*manualFontBold: -misc-fixed-bold-r-normal-*-13-*-*-*-*-*-iso8859-1
Xman*manualFontItalic: -misc-fixed-medium-o-normal-*-13-*-*-*-*-*-iso8859-1
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
