# $XConsortium: srvflags.tcl /main/2 1996/10/25 10:21:38 kaleb $
#
#
#
#
# $XFree86: xc/programs/Xserver/hw/xfree86/XF86Setup/srvflags.tcl,v 3.4.2.2 1998/02/21 06:07:02 robin Exp $
#
# Copyright 1996 by Joseph V. Moss <joe@XFree86.Org>
#
# See the file "LICENSE" for information regarding redistribution terms,
# and for a DISCLAIMER OF ALL WARRANTIES.
#

#
# Configuration of misc server flags
#

proc Other_create_widgets { win } {
	global ServerFlags otherZap otherZoom otherTrapSignals
	global otherXvidtune otherInpDevMods
	global pc98_EGC messages

	set w [winpathprefix $win]
	frame $w.other -width 640 -height 420 \
		-relief ridge -borderwidth 5
	frame $w.srvflags -bd 2 -relief sunken
	if !$pc98_EGC {
		pack  $w.srvflags -in $w.other \
			-fill both -expand yes -padx 20m -pady 20m
	} else {
		pack  $w.srvflags -in $w.other \
			-fill both -expand yes -padx 20m -pady 5m
		$w.other configure -height 400
	}
	label $w.srvflags.title -text $messages(srvflags.1)
	pack  $w.srvflags.title -side top -fill both -expand yes
	frame $w.srvflags.line -height 2 -bd 2 -relief sunken
	pack  $w.srvflags.line -side top -fill x -pady 2m
	checkbutton $w.srvflags.zap         -indicatoron true \
		-text $messages(srvflags.2) \
		-variable otherZap -anchor w
	checkbutton $w.srvflags.zoom        -indicatoron true \
		-text $messages(srvflags.3) \
		-variable otherZoom -anchor w
	checkbutton $w.srvflags.trapsignals -indicatoron true \
		-text $messages(srvflags.4) \
		-variable otherTrapSignals -anchor w
	checkbutton $w.srvflags.nonlocalxvidtune -indicatoron true \
		-text $messages(srvflags.5) \
		-variable otherXvidtune -anchor w
	checkbutton $w.srvflags.nonlocalmodindev -indicatoron true \
		-text $messages(srvflags.6) \
		-variable otherInpDevMods -anchor w
	pack $w.srvflags.zap $w.srvflags.zoom $w.srvflags.trapsignals \
		-anchor w -expand yes -fill x -padx 15m
	pack $w.srvflags.nonlocalxvidtune $w.srvflags.nonlocalmodindev \
		-anchor w -expand yes -fill x -padx 15m
	set otherZap	[expr [string length $ServerFlags(DontZap)] == 0]
	set otherZoom	[expr [string length $ServerFlags(DontZoom)] == 0]
	set otherTrapSignals	[expr [string length \
				$ServerFlags(NoTrapSignals)] > 0]
	set otherXvidtune	[expr [string length \
				$ServerFlags(AllowNonLocalXvidtune)] > 0]
	set otherInpDevMods	[expr [string length \
				$ServerFlags(AllowNonLocalModInDev)] > 0]

}

proc Other_activate { win } {
	set w [winpathprefix $win]
	pack $w.other -side top -fill both -expand yes
}

proc Other_deactivate { win } {
	global ServerFlags otherZap otherZoom otherTrapSignals
	global otherXvidtune otherInpDevMods

	set w [winpathprefix $win]
	pack forget $w.other
	set ServerFlags(DontZap)  [expr $otherZap?"":"DontZap"]
	set ServerFlags(DontZoom) [expr $otherZoom?"":"DontZoom"]
	set ServerFlags(NoTrapSignals) \
		[expr $otherTrapSignals?"NoTrapSignals":""]
	set ServerFlags(AllowNonLocalXvidtune) \
		[expr $otherXvidtune?"AllowNonLocalXvidtune":""]
	set ServerFlags(AllowNonLocalModInDev) \
		[expr $otherInpDevMods?"AllowNonLocalModInDev":""]
}

