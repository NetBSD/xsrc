XCOMM $XFree86: xc/lib/Xft/XftConfig.cpp,v 1.8 2001/11/21 23:41:12 keithp Exp $

dir XFT_TYPE1_DIR

XCOMM
XCOMM alias 'fixed' for 'mono'
XCOMM
match any family == "fixed"		edit family =+ "mono";

XCOMM
XCOMM Check users config file
XCOMM
includeif	"~/.xftconfig"

XCOMM
XCOMM Use Luxi fonts for default faces
XCOMM
match any family == "serif"		edit family += "Luxi Serif";
match any family == "sans"		edit family += "Luxi Sans";
match any family == "mono"		edit family += "Luxi Mono";

XCOMM
XCOMM Alias between XLFD families and font file family name, prefer local
XCOMM fonts
XCOMM
match any family == "charter"		edit family += "bitstream charter";
match any family == "bitstream charter" edit family =+ "charter";

XCOMM
XCOMM Alias older name for Luxi fonts
XCOMM
match any family == "LuciduxSerif" edit family += "Luxi Serif";
match any family == "Lucidux Serif" edit family += "Luxi Serif";

match any family == "LuciduxSans" edit family += "Luxi Sans";
match any family == "Lucidux Sans" edit family += "Luxi Sans";

match any family == "LuciduxMono" edit family += "Luxi Mono";
match any family == "Lucidux Mono" edit family += "Luxi Mono";
