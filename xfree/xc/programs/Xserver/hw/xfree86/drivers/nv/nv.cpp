.\" $XFree86: xc/programs/Xserver/hw/xfree86/drivers/nv/nv.cpp,v 1.11 2000/12/11 20:18:21 dawes Exp $ 
.\" shorthand for double quote that works everywhere.
.ds q \N'34'
.TH NV __drivermansuffix__ "Version 4.0.2"  "XFree86"
.SH NAME
nv \- NVIDIA video driver
.SH SYNOPSIS
.nf
.B "Section \*qDevice\*q"
.BI "  Identifier \*q"  devname \*q
.B  "  Driver \*qnv\*q"
\ \ ...
.B EndSection
.fi
.SH DESCRIPTION
.B nv 
is an XFree86 driver for NVIDIA video cards.  The driver is fully
accelerated, and provides support for the following framebuffer depths:
8, 15, 16 (except Riva128) and 24.  All
visual types are supported for depth 8, TrueColor
visuals are supported for the other depths.  Multi-head configurations
are supported.
.SH SUPPORTED HARDWARE
The
.B nv
driver supports PCI and AGP video cards based on the following NVIDIA chips:
.TP 22
.B RIVA 128
NV3
.TP 22
.B RIVA TNT
NV4
.TP 22
.B RIVA TNT2
NV5
.TP 22
.B GeForce 256, QUADRO 
NV10
.SH CONFIGURATION DETAILS
Please refer to XF86Config(__filemansuffix__) for general configuration
details.  This section only covers configuration details specific to this
driver.
.PP
The driver auto-detects the chipset type and the amount of video memory
present for all chips.
.PP
The following driver
.B Options
are supported:
.TP
.BI "Option \*qHWCursor\*q \*q" boolean \*q
Enable or disable the HW cursor.  Default: on.
.TP
.BI "Option \*qNoAccel\*q \*q" boolean \*q
Disable or enable acceleration.  Default: acceleration is enabled.
.TP
.BI "Option \*qUseFBDev\*q \*q" boolean \*q
Enable or disable use of on OS-specific fb interface (and is not supported
on all OSs).  See fbdevhw(__drivermansuffix__) for further information.
Default: off.
.TP
.BI "Option \*qRotate\*q \*qCW\*q"
.TP
.BI "Option \*qRotate\*q \*qCCW\*q"
Rotate the display clockwise or counterclockwise.  This mode is unaccelerated.
Default: no rotation.
.TP
.BI "Option \*qShadowFB\*q \*q" boolean \*q
Enable or disable use of the shadow framebuffer layer.  See
shadowfb(__drivermansuffix__) for further information.  Default: off.
.SH "SEE ALSO"
XFree86(1), XF86Config(__filemansuffix__), xf86config(1), Xserver(1), X(__miscmansuffix__)
.SH AUTHORS
Authors include: David McKay, Jarno Paananen, Chas Inman, Dave Schmenk, 
Mark Vojkovich
