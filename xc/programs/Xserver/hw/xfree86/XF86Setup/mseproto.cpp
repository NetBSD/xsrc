/* $XFree86: xc/programs/Xserver/hw/xfree86/XF86Setup/mseproto.cpp,v 1.1.2.9 1999/12/11 19:00:41 hohndel Exp $ */

#if defined(PC98)
set SerialMouseTypes [list \
	"Microsoft" \
	"Logitech" \
	"MouseMan" \
	"IntelliMouse" \
]
#else
set SerialMouseTypes [list \
	"Microsoft" \
	"MouseSystems" \
	"MMSeries" \
	"Logitech" \
	"MouseMan" \
	"MMHitTab" \
	"GlidePoint" \
	"IntelliMouse" \
	"ThinkingMouse" \
]
#endif

set BusMouseTypes [list \
	"BusMouse" \
]

#if defined(PC98)
set StandardPS2Types []
set ExtendedPS2Types []
#else
set StandardPS2Types [list \
	"PS/2" \
]

set ExtendedPS2Types [list \
	"IMPS/2" \
	"ThinkingMousePS/2" \
	"MouseManPlusPS/2" \
	"GlidePointPS/2" \
	"NetMousePS/2" \
	"NetScrollPS/2" \
]
#endif

set PnpMouseTypes [list \
	"Auto" \
]


#if defined(__FreeBSD__)
set ExtraMouseTypes [list \
	"SysMouse" \
]

#define MOUSE_TYPES $SerialMouseTypes $BusMouseTypes $StandardPS2Types \
			$PnpMouseTypes $ExtraMouseTypes
#elif defined(__NetBSD__)
set ExtraMouseTypes [list \
	"wsmouse" \
]

#define MOUSE_TYPES $SerialMouseTypes $BusMouseTypes $ExtraMouseTypes \
			$PnpMouseTypes 
#elif defined(__OpenBSD__)
set ExtraMouseTypes [ list \
		    "usb" \
]
#define MOUSE_TYPES $SerialMouseTypes $BusMouseTypes $StandardPS2Types \
			$ExtraMouseTypes $PnpMouseTypes 
#elif defined(Lynx)
#define MOUSE_TYPES $SerialMouseTypes $BusMouseTypes $StandardPS2Types \
			$ExtendedPS2Types
#elif defined(ISC)
#define MOUSE_TYPES $SerialMouseTypes $BusMouseTypes $StandardPS2Types
#elif defined(sun) && defined(i386) && defined(SVR4)
#define MOUSE_TYPES $SerialMouseTypes $BusMouseTypes $StandardPS2Types
#elif defined(DGUX)
#define MOUSE_TYPES $SerialMouseTypes $BusMouseTypes $StandardPS2Types \
			$PnpMouseTypes $ExtendedPS2Types
#elif defined (SVR4) || defined(SYSV)
#define MOUSE_TYPES $SerialMouseTypes $PnpMouseTypes
#else
#define MOUSE_TYPES $SerialMouseTypes $BusMouseTypes $StandardPS2Types \
			$PnpMouseTypes $ExtendedPS2Types
#endif

set SupportedMouseTypes [concat MOUSE_TYPES]
