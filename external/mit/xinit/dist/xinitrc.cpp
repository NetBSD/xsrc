XCOMM!SHELL_CMD
XHASH $NetBSD: xinitrc.cpp,v 1.15 2022/05/08 09:09:58 nia Exp $

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
*VT100.allowBoldFonts:  false
*VT100.font:    -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
*VT100.fontdefault:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
*VT100.utf8Fonts.font:  -*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
*VT100.utf8Fonts.fontdefault:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
*VT100.font1:	-*-spleen-medium-r-*-*-6-*-*-*-*-*-*-*
*VT100.font2:	-*-spleen-medium-r-*-*-8-*-*-*-*-*-*-*
*VT100.font3:	-*-spleen-medium-r-*-*-12-*-*-*-*-*-*-*
*VT100.font4:	-*-spleen-medium-r-*-*-16-*-*-*-*-*-*-*
*VT100.font5:	-*-spleen-medium-r-*-*-24-*-*-*-*-*-*-*
*VT100.font6:	-*-spleen-medium-r-*-*-32-*-*-*-*-*-*-*
*VT100.font7:	-*-spleen-medium-r-*-*-64-*-*-*-*-*-*-*
*VT100.utf8Fonts.font1:	-*-spleen-medium-r-*-*-6-*-*-*-*-*-*-*
*VT100.utf8Fonts.font2:	-*-spleen-medium-r-*-*-8-*-*-*-*-*-*-*
*VT100.utf8Fonts.font3:	-*-spleen-medium-r-*-*-12-*-*-*-*-*-*-*
*VT100.utf8Fonts.font4:	-*-spleen-medium-r-*-*-16-*-*-*-*-*-*-*
*VT100.utf8Fonts.font5:	-*-spleen-medium-r-*-*-24-*-*-*-*-*-*-*
*VT100.utf8Fonts.font6:	-*-spleen-medium-r-*-*-32-*-*-*-*-*-*-*
*VT100.utf8Fonts.font7:	-*-spleen-medium-r-*-*-64-*-*-*-*-*-*-*
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
Xman*font:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xman*manualFontNormal:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xman*manualFontBold:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xman*manualFontItalic:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
Xman*directoryFontNormal:	-*-spleen-medium-r-*-*-$fontsize-*-*-*-*-*-*-*
EOF
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
