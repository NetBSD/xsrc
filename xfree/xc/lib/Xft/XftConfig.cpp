# $XFree86: xc/lib/Xft/XftConfig.cpp,v 1.5 2000/12/14 23:03:53 keithp Exp $

dir XFT_TYPE1_DIR

#
# alias 'fixed' for 'mono'
#
match any family == "fixed"		edit family =+ "mono";

#
# Check users config file
#
includeif	"~/.xftconfig"

#
# Use Lucidux fonts for default faces
#
match any family == "serif"		edit family += "LuciduxSerif";
match any family == "sans"		edit family += "LuciduxSans";
match any family == "mono"		edit family += "LuciduxMono";

#
# Alias between XLFD families and font file family name, prefer local
# fonts
#
match any family == "charter"		edit family += "bitstream charter";
match any family == "bitstream charter" edit family =+ "charter";

match any family == "Lucidux Serif"	edit family += "LuciduxSerif";
match any family == "LuciduxSerif"	edit family =+ "Lucidux Serif";

match any family == "Lucidux Sans"	edit family += "LuciduxSans";
match any family == "LuciduxSans"	edit family =+ "Lucidux Sans";

match any family == "Lucidux Mono"	edit family += "LuciduxMono";
match any family == "LuciduxMono"	edit family =+ "Lucidux Mono";
