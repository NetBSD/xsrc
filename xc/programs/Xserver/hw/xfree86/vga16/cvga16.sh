#!/bin/sh

# $XConsortium: cvga16.sh /main/1 1995/09/04 19:40:40 kaleb $
# $XFree86: xc/programs/Xserver/hw/xfree86/vga16/cvga16.sh,v 3.1 1996/02/04 09:10:44 dawes Exp $
#
# This script generates vga16Conf.c
#
# usage: configvga.sh driver1 driver2 ...
#

VGACONF=./vga16Conf.c

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
