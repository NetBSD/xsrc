.\" $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/r128.cpp,v 1.3 2000/12/11 20:18:07 dawes Exp $
.\" shorthand for double quote that works everywhere.
.ds q \N'34'
.TH R128 __drivermansuffix__ "Version 4.0.2"  "XFree86"
.SH NAME
r128 \- ATI Rage 128 video driver
.SH SYNOPSIS
.nf
.B "Section \*qDevice\*q"
.BI "  Identifier \*q"  devname \*q
.B  "  Driver \*qr128\*q"
\ \ ...
.B EndSection
.fi
.SH DESCRIPTION
.B r128
is an XFree86 driver for ATI Rage 128 based video cards.  It contains
full support for 8, 15, 16 and 24 bit pixel depths, hardware
acceleration of drawing primitives, hardware cursor, video modes up to
1800x1440 @ 70Hz, doublescan modes (e.g., 320x200 and 320x240), gamma
correction at all pixel depths, a fully programming dot clock and robust
text mode restoration for VT switching.
.SH SUPPORTED HARDWARE
The
.B r128
driver supports all ATI Rage 128 based video cards including the Rage
Fury AGP 32MB, the XPERT 128 AGP 16MB and the XPERT 99 AGP 8MB.
.SH CONFIGURATION DETAILS
Please refer to XF86Config(__filemansuffix__) for general configuration
details.  This section only covers configuration details specific to this
driver.
.PP
The driver auto-detects all device information necessary to initialize
the card.  However, if you have problems with auto-detection, you can
specify:
.PP
.RS 4
VideoRam - in kilobytes
.br
MemBase  - physical address of the linear framebuffer
.br
IOBase   - physical address of the MMIO registers
.br
ChipID   - PCI DEVICE ID
.RE
.PP
In addition, the following driver
.B Options
are supported:
.TP
.BI "Option \*qSWcursor\*q \*q" boolean \*q
Selects software cursor.  The default is
.B off.
.TP
.BI "Option \*qNoAccel\*q \*q" boolean \*q
Enables or disables all hardware acceleration.  The default is to
.B enable
hardware acceleration.
.TP
.BI "Option \*qDac6Bit\*q \*q" boolean \*q
Enables or disables the use of 6 bits per color component when in 8 bpp
mode (emulates VGA mode).  By default, all 8 bits per color component
are used.  The default is
.B off.
.TP
.BI "Option \*qVideoKey\*q \*q" integer \*q
This overrides the default pixel value for the YUV video overlay key.
The default value is
.B undefined.
.SH "SEE ALSO"
XFree86(1), XF86Config(__filemansuffix__), xf86config(1), Xserver(1), X(__miscmansuffix__)
.SH AUTHORS
.nf
Rickard E. (Rik) Faith   \fIfaith@precisioninsight.com\fP
Kevin E. Martin          \fIkevin@precisioninsight.com\fP
