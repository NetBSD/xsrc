#!/bin/sh
# $XFree86: xc/programs/Xserver/hw/xfree86/accel/s3_virge/confS3V.sh,v 3.0 1996/10/03 08:33:08 dawes Exp $
#
# This script generates s3vConf.c
#
# usage: confS3V.sh driver1 driver2 ...
#
# $XConsortium: confS3V.sh /main/2 1995/11/12 19:05:26 kaleb $

S3CONF=./s3vConf.c

cat > $S3CONF <<EOF
/*
 * This file is generated automatically -- DO NOT EDIT
 */

#include "s3v.h"

extern s3VideoChipRec
EOF
Args="`echo $* | tr '[a-z]' '[A-Z]'`"
set - $Args
while [ $# -gt 1 ]; do
  echo "        $1," >> $S3CONF
  shift
done
echo "        $1;" >> $S3CONF
cat >> $S3CONF <<EOF

s3VideoChipPtr s3Drivers[] =
{
EOF
for i in $Args; do
  echo "        &$i," >> $S3CONF
done
echo "        NULL" >> $S3CONF
echo "};" >> $S3CONF
