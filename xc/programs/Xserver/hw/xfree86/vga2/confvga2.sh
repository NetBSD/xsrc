#!/bin/sh

# $XConsortium: confvga2.sh /main/1 1995/09/04 19:41:31 kaleb $
# $XFree86: xc/programs/Xserver/hw/xfree86/vga2/confvga2.sh,v 3.1 1996/02/04 09:11:53 dawes Exp $
#
# This script generates vga2Conf.c
#
# usage: configvga.sh driver1 driver2 ...
#

VGACONF=./vga2Conf.c

cat > $VGACONF <<EOF
/*
 * This file is generated automatically -- DO NOT EDIT
 */

#include "xf86.h"
#include "vga.h"

extern vgaVideoChipRec
EOF
Args="`echo $* | tr '[a-z]' '[A-Z]'`"
set - $Args
while [ $# -gt 1 ]; do
  echo "        $1," >> $VGACONF
  shift
done
echo "        $1;" >> $VGACONF
cat >> $VGACONF <<EOF

vgaVideoChipPtr vgaDrivers[] =
{
EOF
for i in $Args; do
  echo "        &$i," >> $VGACONF
done
echo "        NULL" >> $VGACONF
echo "};" >> $VGACONF
