.\" $XFree86: xc/programs/Xserver/hw/xfree86/drivers/savage/savage.cpp,v 1.2 2000/12/11 20:18:25 dawes Exp $ 
.\" shorthand for double quote that works everywhere.
.ds q \N'34'
.TH SAVAGE __drivermansuffix__ "Version 4.0.2"  "XFree86"
.SH NAME
savage \- S3 Savage video driver
.SH SYNOPSIS
.nf
.B "Section \*qDevice\*q"
.BI "  Identifier \*q"  devname \*q
.B  "  Driver \*qsavage\*q"
\ \ ...
.B EndSection
.fi
.SH DESCRIPTION
.B savage 
is an XFree86 driver for the S3 Savage family video accelerator chips.  The
.B savage
driver supports PCI and AGP boards with the following chips:
.TP 16
.BI Savage3D
(8a20 and 8a21) 
.TP 16
.B Savage4
(8a22) 
.TP 16
.B Savage2000
(9102) 
.TP 16
.B Savage/MX
(8c10 and 8c11) 
.TP 16
.B Savage/IX
(8c12 and 8c13) 
.TP 16
.B ProSavage PM133
(8a25)
.TP 16
.B ProSavage KM133
(8a26)
.SH CONFIGURATION DETAILS
Please refer to XF86Config(__filemansuffix__) for general configuration
details.  This section only covers configuration details specific to this
driver.
.PP
The following driver
.B Options
are supported:
.TP
.BI "Option \*qHWCursor\*q \*q" boolean \*q
.TP
.BI "Option \*qSWCursor\*q \*q" boolean \*q
These two options interact to specify hardware or software cursor.  If the
SWCursor option is specified, any HWCursor setting is ignored.  Thus, either
\*qHWCursor off\*q or \*qSWCursor on\*q will force the use of the software 
cursor.  On Savage/MX and Savage/IX chips which are connected to LCDs, a
software cursor will be forced, because the Savage hardware cursor does not 
correctly track the automatic panel expansion feature.
Default: hardware cursor.
.TP
.BI "Option \*qNoAccel\*q \*q" boolean \*q
Disable or enable acceleration.  Default: acceleration is enabled.
.TP
.BI "Option \*qRotate\*q \*qCW\*q"
.TP
.BI "Option \*qRotate\*q \*qCCW\*q"
Rotate the desktop 90 degrees clockwise or counterclockwise.  This option 
forces the ShadowFB option on, and disables acceleration.
Default: no rotation.
.TP
.BI "Option \*qShadowFB\*q \*q" boolean \*q
Enable or disable use of the shadow framebuffer layer.  See
shadowfb(__drivermansuffix__) for further information.  This option
disables acceleration.  Default: off.
.TP
.BI "Option \*qUseBIOS\*q \*q" boolean \*q
Enable or disable use of the video BIOS to change modes.  Ordinarily, the 
.B savage 
driver tries to use the video BIOS to do mode switches.  This generally 
produces the best results with the mobile chips (/MX and /IX), since the BIOS
knows how to handle the critical but unusual timing requirements of the 
various LCD panels supported by the chip.  To do this, the driver searches
through the BIOS mode list, looking for the mode which most closely matches
the XF86Config mode line.  Some purists find this scheme objectionable.  If 
you would rather have the
.B savage
driver use your mode line timing exactly, turn off the UseBios option.  
Default: on (use the BIOS).
.SH FILES
savage_drv.o
.SH "SEE ALSO"
XFree86(1), XF86Config(__filemansuffix__), xf86config(1), Xserver(1), X(__miscmansuffix__)
.SH AUTHORS
Authors include Tim Roberts (timr@probo.com) and Ani Joshi (ajoshi@unixbox.com)
for the 4.0 version, and Tim Roberts and S. Marineau for the 3.3 driver from 
which this was derived.
