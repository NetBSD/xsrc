XCOMM $XFree86: xc/lib/Xft/XftConfig.cpp,v 1.6 2001/04/27 14:55:22 tsi Exp $

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
XCOMM Use Lucidux fonts for default faces
XCOMM
match any family == "serif"		edit family += "LuciduxSerif";
match any family == "sans"		edit family += "LuciduxSans";
match any family == "mono"		edit family += "LuciduxMono";

XCOMM
XCOMM Alias between XLFD families and font file family name, prefer local
XCOMM fonts
XCOMM
match any family == "charter"		edit family += "bitstream charter";
match any family == "bitstream charter" edit family =+ "charter";

match any family == "Lucidux Serif"	edit family += "LuciduxSerif";
match any family == "LuciduxSerif"	edit family =+ "Lucidux Serif";

match any family == "Lucidux Sans"	edit family += "LuciduxSans";
match any family == "LuciduxSans"	edit family =+ "Lucidux Sans";

match any family == "Lucidux Mono"	edit family += "LuciduxMono";
match any family == "LuciduxMono"	edit family =+ "Lucidux Mono";
