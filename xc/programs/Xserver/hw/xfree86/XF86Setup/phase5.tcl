# $XConsortium: phase5.tcl /main/4 1996/10/28 05:42:32 kaleb $
#
#
#
#
# $XFree86: xc/programs/Xserver/hw/xfree86/XF86Setup/phase5.tcl,v 3.7.2.4 1999/10/11 16:05:00 hohndel Exp $
#
# Copyright 1996 by Joseph V. Moss <joe@XFree86.Org>
#
# See the file "LICENSE" for information regarding redistribution terms,
# and for a DISCLAIMER OF ALL WARRANTIES.
#

#
# Phase V - Final commands after return to text mode
#

check_tmpdirs
clear_scrn
foreach fname [glob -nocomplain $TmpDir/*] {
	unlink $fname
}
rmdir $TmpDir
rmdir $XF86SetupDir

if { ![getuid] } {
    set idx -1
    if { [llength $DeviceIDs] == 1 } {
	set idx 0
    } else {
	if [string length $Scrn_Accel(Device)] {
	    set idx [lsearch $DeviceIDs $Scrn_Accel(Device)]
	} else {
	    set idx [lsearch $DeviceIDs $Scrn_SVGA(Device)]
	}
    }
    if { $idx >= 0 } {
	set devid [lindex $DeviceIDs $idx]
        global Device_$devid
        set server [set Device_${devid}(Server)]
	set linkname $Xwinhome/bin/X
	set lastlink $linkname
	for {set nlinks 0} \
		{[file exists $linkname] && [file type $linkname]=="link" \
		 && $nlinks<20} \
		{incr nlinks} {
	    set lastlink $linkname
	    set linkname [readlink $linkname]
	}
	catch {
            if { [file type $linkname]=="link" && ![file exists $linkname] } {
                set lastlink [readlink $linkname]
	    }
	}

	if { $nlinks < 20 } {
	    set servname [string range [file tail $linkname] 5 end]
	    if ![string compare $servname $server] {
		unset linkname
	    }
	} else {
	    set linkname $Xwinhome/bin/X
	}
    }
    if [info exists linkname] {
	set linkdir [file dirname $lastlink]
	set mklink [mesg "Do you want to create an 'X' link\
		to the $server server?\n\n(the link will be\
		created in the directory: $linkdir) Okay?" yesno]
	if $mklink {
	    set CWD [pwd]
	    cd $linkdir
	    catch "unlink X" ret
	    if !$pc98 {
		if [string match XFCom* $server] {
		    set serverpath $Xwinhome/bin/$server
		} else {
		    set serverpath $Xwinhome/bin/XF86_$server
		}
		if [catch "link $serverpath X" ret] {
		    mesg "Link creation failed!\n\
			    You'll have to do it yourself" okay
	        } else {
		    mesg "Link created successfully." okay
	        }
	    } else {
	        if [catch "link $Xwinhome/bin/XF98_$server X" ret] {
		    mesg "Link creation failed!\n\
			    You'll have to do it yourself" okay
	        } else {
		    mesg "Link created successfully." okay
	        }
	    }
	    cd $CWD
	}
    }
}

clear_scrn
puts "\n\nConfiguration complete."

exit 0

