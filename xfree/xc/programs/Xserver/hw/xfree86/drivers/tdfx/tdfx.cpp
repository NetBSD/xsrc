.\" $XFree86: xc/programs/Xserver/hw/xfree86/drivers/tdfx/tdfx.cpp,v 1.6 2000/12/11 20:18:36 dawes Exp $ 
.\" shorthand for double quote that works everywhere.
.ds q \N'34'
.TH TDFX __drivermansuffix__ "Version 4.0.2"  "XFree86"
.SH NAME
tdfx \- 3Dfx video driver
.SH SYNOPSIS
.nf
.B "Section \*qDevice\*q"
.BI "  Identifier \*q"  devname \*q
.B  "  Driver \*qtdfx\*q"
\ \ ...
.B EndSection
.fi
.SH DESCRIPTION
.B tdfx 
is an XFree86 driver for 3Dfx video cards.
It supports the Voodoo Banshee, Voodoo3, Voodoo4 and Voodoo5 cards.
.SH SUPPORTED HARDWARE
The
.B tdfx
driver supports...
.SH CONFIGURATION DETAILS
Please refer to XF86Config(__filemansuffix__) for general configuration
details.  This section only covers configuration details specific to this
driver.
.SH "SEE ALSO"
XFree86(1), XF86Config(__filemansuffix__), xf86config(1), Xserver(1), X(__miscmansuffix__)
.SH AUTHORS
Authors include: ...
