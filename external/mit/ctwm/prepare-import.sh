#!/bin/sh
# $NetBSD: prepare-import.sh,v 1.1 2021/04/11 11:00:29 nia Exp $
#
# Run this script and check for additional files and
# directories to prune, only relevant content is included.

set -e

cd dist
rm -f CMakeLists.txt Doxyfile.in
rm -f README.html CHANGES.html
rm -f system.ctwmrc examples/*.ctwmrc
rm -f tools/*.sh tools/*.pl tools/*.astyle
rm -f minibuild/*.mk minibuild/*.sh minibuild/*.md
rm -f gen/ctwm.1.html
rm -f image_jpeg.c image_jpeg.h
rm -f sound.c sound.h
