#!/bin/sh

#
# $XFree86: xc/programs/Xserver/hw/xfree86/etc/Xinstall.sh,v 1.89 2005/03/17 01:11:56 dawes Exp $
#
# Copyright © 2000 by Precision Insight, Inc.
# Copyright © 2000, 2001 by VA Linux Systems, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# Except as contained in this notice, the name of the copyright holder(s)
# and author(s) shall not be used in advertising or otherwise to promote
# the sale, use or other dealings in this Software without prior written
# authorization from the copyright holder(s) and author(s).
#

#
# Copyright © 1996-2005 by The XFree86 Project, Inc.
# All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject
# to the following conditions:
#
#   1.  Redistributions of source code must retain the above copyright
#       notice, this list of conditions, and the following disclaimer.
#
#   2.  Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer
#       in the documentation and/or other materials provided with the
#       distribution, and in the same place and form as other copyright,
#       license and disclaimer information.
#
#   3.  The end-user documentation included with the redistribution,
#       if any, must include the following acknowledgment: "This product
#       includes software developed by The XFree86 Project, Inc
#       (http://www.xfree86.org/) and its contributors", in the same
#       place and form as other third-party acknowledgments.  Alternately,
#       this acknowledgment may appear in the software itself, in the
#       same form and location as other such third-party acknowledgments.
#
#   4.  Except as contained in this notice, the name of The XFree86
#       Project, Inc shall not be used in advertising or otherwise to
#       promote the sale, use or other dealings in this Software without
#       prior written authorization from The XFree86 Project, Inc.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE XFREE86 PROJECT, INC OR ITS CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
# OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
# OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
# BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
# OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

#
# This script should be used to install XFree86 4.5.0.
#
# Parts of this script are based on the old preinst.sh and postinst.sh
# scripts.
#
# Set tabs to 4 spaces to view/edit this file.
#
# Authors:	David Dawes <dawes@xfree86.org>
#

# Fallbacks for when the bindist version can't be auto-detected.
# These should be updated for each release.

if [ X"$1" != "X-check" ]; then
	if [ X$XINST_LOGFILE = X ]; then
		BACKUP_SUFFIX=$$
		XINST_LOGFILE=XFree86-install.log.$$
		export XINST_LOGFILE
		export BACKUP_SUFFIX
		echo ""
		echo "Logging this install to $XINST_LOGFILE"
		echo ""
		sleep 1
		exec sh $0 "$@" 2>&1 | tee $XINST_LOGFILE; exit
	fi
fi

if [ X$XINST_LOGFILE != X ]; then
	logmsg="The log for this installation is $XINST_LOGFILE"
	trap "echo ''; echo $logmsg; echo ''; exit" 0
fi

SNAPSHOT=n

if [ $SNAPSHOT = y ]; then
	FULLPREFIX=XXX
	VERSION=4.4.99.XXX
	PATCHLEVEL=0
	FULLVERSION=$VERSION
else
	FULLPREFIX=4.5
	PATCHLEVEL=0
	VERSION=$FULLPREFIX.$PATCHLEVEL
	FULLVERSION=$FULLPREFIX.0
fi
SCRIPTVERSION=$VERSION

BINDISTFULLPREFIX=
BINDISTPATCHLEVEL=
BINDISTVERSION=
BINDISTFULLVERSION=

ROOTDIR=

DEBUG=

TESTROOT=/home1/test

if [ X"$1" = "X-test" -o X"$XINST_TEST" != X ]; then
	if [ X"$1" = "X-test" ]; then
		shift
		case "$1" in
		/*)
			TESTROOT="$1"
			shift
			;;
		esac
	else
		case "$XINST_TEST" in
		/*)
			TESTROOT="$XINST_TEST"
			;;
		esac
	fi
	ROOTDIR=$TESTROOT
	echo ""
	echo "Running in test mode, with root set to $TESTROOT"
	sleep 2
	echo ""
	if [ ! -d $TESTROOT ]; then
		echo "$TESTROOT doesn't exist (for test mode)"
		exit 1
	fi
	for i in usr etc var; do
		if [ ! -d $TESTROOT/$i ]; then
			echo "$TESTROOT/$i doesn't exist, creating it"
			mkdir $TESTROOT/$i
		fi
	done
fi

RUNDIR=$ROOTDIR/usr/X11R6
ETCDIR=$ROOTDIR/etc
VARDIR=$ROOTDIR/var

OLDFILES=" \
	$RUNDIR/include/freetype2/ft2build.h \
	$RUNDIR/lib/X11/fonts/misc/heb6x13.pcf.gz \
	$RUNDIR/lib/X11/fonts/misc/heb8x13.pcf.gz \
	$RUNDIR/lib/X11/fonts/misc/7x14rk.pcf.gz \
	$RUNDIR/lib/X11/fonts/misc/7x13euro.pcf.gz \
	$RUNDIR/lib/X11/fonts/misc/7x13euroB.pcf.gz \
	"

OLDDIRS=" \
	$RUNDIR/lib/X11/xkb/compiled \
	$RUNDIR/lib/X11/xkb/geometry/hp \
	"

OLDMODULES=" \
	xie.so \
	pex5.so \
	glx.so \
	"

BASEDIST=" \
	Xbin.tgz \
	Xlib.tgz \
	Xman.tgz \
	Xdoc.tgz \
	Xfnts.tgz \
	Xfenc.tgz \
	"

UPDDIST=" \
	Xupd.tgz \
	"

UPDATEDIST=" \
	Xupdate.tgz \
	Xdrivers.tgz \
	Xdocupd.tgz \
	"

ETCDIST="Xetc.tgz"
RCDIST="Xrc.tgz"

VARDIST=""

SERVDIST=" \
	Xxserv.tgz \
	Xmod.tgz \
	"
OPTDIST=" \
	Xfsrv.tgz \
	Xnest.tgz \
	Xprog.tgz \
	Xprt.tgz \
	Xvfb.tgz \
	Xf100.tgz \
	Xfcyr.tgz \
	Xfscl.tgz \
	Xhtml.tgz \
	Xps.tgz \
	Xpdf.tgz \
	"

ETCDLINKS=" \
	app-defaults \
	fs \
	lbxproxy \
	proxymngr \
	rstart \
	twm \
	xdm \
	xinit \
	xkb \
	xsm \
	xserver \
	"

ETCFLINKS=""

ETCFONTFILES=" \
	fonts.conf \
	fonts.dtd \
	"

XKBDIR="$ETCDIR/X11/xkb"
XKBDBDIR=

FONTDIRS=" \
	local \
	misc
	"

XSERVERCMD="$RUNDIR/bin/XFree86"

VERSIONFILE=".XFree86_Version"

WDIR="`pwd`"

NoEtcX11=
DOUPDATE=
DOBASE=

OPTS=""

# Check how to suppress newlines with echo (from perl's Configure)
((echo "xxx\c"; echo " ") > .echotmp) 2> /dev/null
if [ ! -f .echotmp ]; then
	echo "Can't write to the current directory.  Aborting";
	exit 1
fi
if grep c .echotmp >/dev/null 2>&1; then
	n='-n'
	c=''
else
	n=''
	c='\c'
fi
rm -f .echotmp

Echo()
{
	echo $n "$@""$c"
}

Mkdir()
{
	if [ X"$DEBUG" != X ]; then
		echo "=> " mkdir "$@"
	fi
	mkdir "$@"
}

Ln()
{
	if [ X"$DEBUG" != X ]; then
		echo "=> " ln "$@"
	fi
	ln "$@"
}

Rm()
{
	if [ X"$DEBUG" != X ]; then
		echo "=> " rm "$@"
	fi
	rm "$@"
}

Cp()
{
	if [ X"$DEBUG" != X ]; then
		echo "=> " cp "$@"
	fi
	cp "$@"
}

ContinueNo()
{
	Echo "Do you wish to continue? (y/n) [n] "
	read response
	case "$response" in
	[yY]*)
		echo ""
		;;
	*)
		echo "Aborting the installation."
		exit 2
		;;
	esac
}

ContinueYes()
{
	Echo "Do you wish to continue? (y/n) [y] "
	read response
	case "$response" in
	[nN]*)
		echo "Aborting the installation."
		exit 2
		;;
	*)
		echo ""
		;;
	esac
}

Description()
{
	case $1 in
	Xfsrv*)
		echo "font server";;
	Xnest*)
		echo "Nested X server";;
	Xprog*)
		echo "programmer support";;
	Xprt*)
		echo "X print server";;
	Xvfb*)
		echo "Virtual framebuffer X server";;
	Xtinyx*)
		echo "TinyX X servers";;
	Xdrm*)
		echo "DRM kernel module source";;
	Xf100*)
		echo "100dpi fonts";;
	Xfcyr*)
		echo "Cyrillic fonts";;
	Xfscl*)
		echo "Scaled fonts (Speedo, Type1 and TTF)";;
	Xhtml*)
		echo "Docs in HTML";;
	Xjdoc*)
		echo "Docs in Japanese";;
	Xps*)
		echo "Docs in PostScript";;
	Xpdf*)
		echo "Docs in PDF";;
	Xaout*)
		echo "a.out compatibility libraries";;
	Xquartz*)
		echo "Mac OS X Quartz X server and extensions";;
	Xupd.tgz)
		echo "Post-release updates";;
	*)
		echo "unknown";;
	esac
}

CheckUtil()
{
	if [ X"$1" = X-t ]; then
		shift
		testonly=1
	else
		testonly=""
	fi

	# check if 'which' works
	if which /bin/sh > /dev/null 2>&1; then
		if which $1 > /dev/null 2>&1; then
			return 0
		else
			if [ "$testonly" ]; then
				return 1
			else
				echo "Cannot find the \"$1\" command."
				echo "Aborting"
				exit 1
			fi
		fi
	else
		return 0
	fi
}

ReadLink()
{
	rltmp="`ls -l $1`"
	rl=`expr "$rltmp" : '.*-> \([^ 	]*\)'`
	echo $rl
}

GetOsInfo()
{
	CheckUtil uname

	echo "Checking which OS you're running..."

	OsName="`uname`"
	OsVersion="`uname -r`"
	case "$OsName" in
	SunOS) # Assumes SunOS 5.x
		OsArch="`uname -p`"
		;;
	*)
		OsArch="`uname -m`"
		;;
	esac
	# Some SVR4.0 versions have a buggy uname that reports the node name
	# for the OS name.  Try to catch that here.  Need to check what is
	# reported for non-buggy versions.
	if [ "$OsName" = "`uname -n`" -a -f /stand/unix ]; then
		OsName=UNIX_SV
	fi
	echo "uname reports '$OsName' version '$OsVersion', architecture '$OsArch'."

	# Find the object type, where needed

	case "$OsName" in
	FreeBSD|NetBSD|OpenBSD)
		CheckUtil file
		if file -L /bin/sh | grep ELF > /dev/null 2>&1; then
			OsObjFormat=ELF
		else
			OsObjFormat=a.out
		fi
		;;
	esac

	if [ X"$OsObjFormat" != X ]; then
		Echo "Object format is '$OsObjFormat'.  "
		needNL=YES
	fi

	# test's flag for symlinks
	#
	# For OSs that don't support symlinks, choose a type that is guaranteed to
	# return false for regular files and directories.

	case "$OsName" in
	FreeBSD)
		case "$OsVersion" in
		2.*)
			L="-h"
			;;
		*)
			L="-L"
			;;
		esac
		;;
	SunOS)
		L="-h"				# /bin/sh built-in doesn't do -L
		;;
	OS-with-no-symlinks)	# Need to set this correctly
		L="-b"
		NoSymlinks=YES
		;;
	*)
		L="-L"
		;;
	esac

	if [ X"$1" = X-l ]; then
		# Find the libc version, where needed
		case "$OsName" in
		Linux)
			tmp="`ldd /bin/sh | grep libc.so 2> /dev/null`"
			LibcPath=`expr "$tmp" : '[^/]*\(/[^ ]*\)'`

			# If strings is available, it is the best way to get the
			# libc version.

			if CheckUtil -t strings; then
				tmp="`strings $LibcPath | grep -i 'c library'`"
			else
				if [ $L $LibcPath ]; then
					tmp=`ReadLink $LibcPath`
					# This assumes libc-maj.min.teeny.so
					tmp="`echo $tmp | sed 's/-/ /'`"
				else
					tmp=""
				fi
			fi

			OsLibcMajor=`expr "$tmp" : '.* \([0-9][0-9]*\)'`
			OsLibcMinor=`expr "$tmp" : '.* [0-9][0-9]*\.\([0-9][0-9]*\)'`
			OsLibcTeeny=`expr "$tmp" : '.* [0-9][0-9]*\.[0-9][0-9]*\.\([0-9][0-9]*\)'`
			case "$OsLibcMajor" in
			2)
				# 2 is the glibc version
				OsLibcMajor=6
				;;
			"")
				echo "Cannot find the glibc version."
				echo "Aborting."
				exit 1
			esac
			;;
		esac

		if [ X"$OsLibcMajor" != X ]; then
			Echo "libc version is '$OsLibcMajor"
			if [ X"$OsLibcMinor" != X ]; then
				Echo ".$OsLibcMinor"
				if [ X"$OsLibcTeeny" != X ]; then
					Echo ".$OsLibcTeeny"
					if [ $OsLibcTeeny -gt 80 ]; then
						OsLibcMinor=`expr $OsLibcMinor + 1`
					fi
				fi
				Echo "'"
				Echo " ($OsLibcMajor.$OsLibcMinor)"
			else
				Echo "'"
			fi
			echo "."
		fi
	fi

#	if [ X"$needNL" = XYES ]; then
#		echo ""
#	fi
	echo ""
}

DoOsChecks()
{
	# Do some OS-specific checks

	case "$OsName" in
	Linux)
		# Check ldconfig
		LDSO=`/sbin/ldconfig -v -n | awk '{ print $3 }'`
		# if LDSO is empty ldconfig may be Version 2
		if [ X"$LDSO" = X ]; then
			LDSO=`/sbin/ldconfig -V | awk 'NR == 1 { print $4 }'`
		fi
		LDSOMIN=`echo $LDSO | awk -F[.-] '{ print $3 }'`
		LDSOMID=`echo $LDSO | awk -F[.-] '{ print $2 }'`
		LDSOMAJ=`echo $LDSO | awk -F[.-] '{ print $1 }'`
		if [ "$LDSOMAJ" -gt 1 ]; then
			: OK
		else
			if [ "$LDSOMID" -gt 7 ]; then
				: OK
			else
				if [ "$LDSOMIN" -ge 14 ]; then
					: OK
				else
					echo ""
					echo "Before continuing, you will need to get a"
					echo "current version of ld.so.  Version 1.7.14 or"
					echo "newer will do."
					NEEDSOMETHING=YES
				fi
			fi
		fi
		# The /dev/tty0 check is left out.  Presumably nobody has a system where
		# this is missing any more.
		;;
	esac
}

FindDistName()
{
	case "$OsName" in
	CYGWIN*)
		case "$OsArch" in
		i*86)
			DistName="Cygwin"
			;;
		*)
			Message="Cygwin binaries are only available for ix86 platforms"
			;;
		esac
		;;
	Darwin)
		case "$OsArch" in
		Power*)
			case "$OsVersion" in
			1.[2-3]*)
				DistName="Darwin-ppc-1.x"
				;;
			1.4* | 5.*)
				DistName="Darwin-ppc-5.x"
				;;
			6.[0-2]*)
				if [ -d /System/Library/Frameworks/ApplicationServices.framework ]; then
					Message="No binaries available for Mac OS X 10.2.0 - 10.2.2"
				else
					DistName="Darwin-ppc-6.x"
				fi
				;;
			6.[3-9]*)
				DistName="Darwin-ppc-6.x"
				;;
			7.*)
				DistName="Darwin-ppc-7.x"
				;;
			[8-9].*)
				Message="No Darwin/ppc binaries available for this OS version.  Try Darwin-ppc-7.x"
				;;
			*)
				Message="No Darwin/ppc binaries available for this OS version"
				;;
			esac
			;;
		x86*)
			case "$OsVersion" in
			1.4* | 5.*)
				DistName="Darwin-ix86-5.x"
				;;
			6.* | 7.*)
				DistName="Darwin-ix86-7.x"
				;;
			[8-9].*)
				Message="No Darwin/ix86 binaries available for this OS version.  Try Darwin-ix86-7.x"
				;;
			*)
				Message="No Darwin/ix86 binaries available for this OS version"
				;;
			esac
			;;
		*)
			Message="Darwin binaries are not available for this architecture"
			;;
		esac
		;;
	DGUX)	# Check this string
		case "$OsArch" in
		i*86)
			DistName="DGUX-ix86"
			;;
		*)
			Message="DGUX binaries are only available for ix86 platforms"
			;;
		esac
		;;
	FreeBSD)
		case "$OsArch" in
		i386)
			case "$OsVersion" in
			2.2*)
				DistName="FreeBSD-2.2.x"
				;;
			3.*)
				case "$OsObjFormat" in
				ELF)
					DistName="FreeBSD-3.x"
					;;
				*)
					Message="FreeBSD 3.x binaries are only available in ELF format"
					;;
				esac
				;;
			4.*)
				DistName="FreeBSD-4.x"
				;;
			5.*)
				DistName="FreeBSD-5.x"
				;;
			*)
				Message="FreeBSD/i386 binaries are not available for this version"
				;;
			esac
			;;
		alpha)
			case "$OsVersion" in
			3.*)
				DistName="FreeBSD-alpha-3.x"
				;;
			4.*)
				DistName="FreeBSD-alpha-4.x"
				;;
			5.*)
				DistName="FreeBSD-alpha-5.x"
				;;
			*)
				Message="FreeBSD/alpha binaries are not available for this version"
				;;
			esac
			;;
		*)
			Message="FreeBSD binaries are not available for this architecture"
			;;
		esac
		;;
	Linux)
		case "$OsArch" in
		i*86)
			case "$OsLibcMajor" in
			5)
				DistName="Linux-ix86-libc5"
				;;
			6)
				case "$OsLibcMinor" in
				0)
					DistName="Linux-ix86-glibc20"
					;;
				1)
					DistName="Linux-ix86-glibc21"
					;;
				2)
					DistName="Linux-ix86-glibc22"
					;;
				3)
					DistName="Linux-ix86-glibc23"
					;;
				*)
					Message="No dist available for glibc 2.$OsLibcMinor.  Try Linux-ix86-glibc23"
					;;
				esac
				;;
			*)
				Message="No Linux/ix86 binaries for this libc version"
				;;
			esac
			;;
		ppc)
			case "$OsLibcMajor.$OsLibcMinor" in
			6.1)
				DistName="Linux-ppc-glibc21"
				;;
			6.*)
				Message="No Linux/ppc binaries for glibc 2.$OsLibcMinor.  Try Linux-ppc-glibc21"
				;;
			*)
				Message="No Linux/ppc binaries for this libc version"
				;;
			esac
			;;
		alpha)
			case "$OsLibcMajor.$OsLibcMinor" in
			6.1)
				DistName="Linux-alpha-glibc21"
				;;
			6.2)
				DistName="Linux-alpha-glibc22"
				;;
			6.*)
				Message="No Linux/alpha binaries for glibc 2.$OsLibcMinor.  Try Linux-alpha-glibc22"
				;;
			*)
				Message="No Linux/alpha binaries for this libc version"
				;;
			esac
			;;
		mips)
			case "$OsLibcMajor.$OsLibcMinor" in
			6.0)
				DistName="Linux-mips-glibc20"
				;;
			*)
				Message="No Linux/Mips binaries for this libc version"
				;;
			esac
			;;
		x86_64|amd64)
			case "$OsLibcMajor.$OsLibcMinor" in
			6.2)
				DistName="Linux-amd64-glibc22"
				;;
			6.[3-9]*)
				Message="No dist available for glibc 2.$OsLibcMinor.  Try Linux-amd64-glibc22"
				;;
			*)
				Message="No Linux/AMD64 binaries for this libc version"
				;;
			esac
			;;
		
		*)
			Message="No Linux binaries available for this architecture"
			;;
		esac
		;;
	LynxOS)	# Check this
		DistName="LynxOS"
		;;
	NetBSD)
		case "$OsArch" in
		i386)
			case "$OsVersion" in
			1.[4-9]*)	# Check this
				case "$OsObjFormat" in
				a.out)
					DistName="NetBSD-1.4.x"
					;;
				*)
					DistName="NetBSD-1.6 or NetBSD-1.5"
					;;
				esac
				;;
			2.*)
				DistName="NetBSD-2.0"
				;;
			*)
				Message="No NetBSD/i386 binaries available for this version"
				;;
			esac
			;;
		*)
			Message="No NetBSD binaries available for this architecture"
			;;
		esac
		;;
	OpenBSD)
		case "$OsArch" in
		i386)
			case "$OsVersion" in
			2.*)
				DistName="OpenBSD-2.8"
				;;
			3.*)	# Check this
				DistName="OpenBSD-3.4"
				;;
			*)
				Message="No OpenBSD/i386 binaries available for this version.   Try OpenBSD-3.4."
				;;
			esac
			;;
		*)
			Message="No OpenBSD binaries available for this architecture"
			;;
		esac
		;;
	SunOS)
		case "$OsArch" in
		i386)
			case "$OsVersion" in
			5.[67]*)
				DistName="Solaris"
				;;
			5.8*)
				DistName="Solaris-8"
				;;
			5.9*)
				DistName="Solaris-9"
				;;
			5.10*)
				Message="No Solaris/x86 binaries available for this version.  Try Solaris-9."
				;;
			*)
				Message="No Solaris/x86 binaries available for this version."
				;;
			esac
			;;
		sparc)
			case "$OsVersion" in
			5.9*)
				DistName="Solaris-sparc-9"
				;;
			5.8*|5.10*)
				Message="No Solaris/sparc binaries available for this version.  Try Solaris-sparc-9."
				;;
			*)
				Message="No Solaris/sparc binaries available for this version."
				;;
			esac
			;;
		*)
			Message="No SunOS/Solaris binaries available for this architecture"
			;;
		esac
		;;
	UNIX_SV)
		case "$OsArch" in
		i386)
			case "$OsVersion" in
			4.0*)
				DistName="SVR4.0"
				;;
			*)
				# More detailed version check??
				DistName="UnixWare"
				;;
			esac
			;;
		*)
			Message="No SYSV binaries available for this architecture"
			;;
		esac
		;;
	*)
		Message="No binaries available for this OS"
		;;
	esac

	if [ X"$DistName" != X ]; then
		echo "Binary distribution name is '$DistName'"
		echo ""
		echo "If you don't find a binary distribution with this name, then"
		echo "binaries for your platform are not available from XFree86.org."
		echo ""
	else
		if [ X"$Message" = X ]; then
			echo "Can't find which binary distribution you should use."
			echo "Please send the output of this script to XFree86@XFree86.org."
			echo ""
		else
			echo "$Message"
			echo ""
		fi
	fi
}

GetBindistVersion()
{
	VERSTARBALL=
	if [ X$DOBASE = XYES ]; then
		if [ -f Xbin.tgz ]; then
			VERSTARBALL=Xbin.tgz
		fi
	else
		if [ X$DOUPDATE = XYES ]; then
			if [ -f Xupdate.tgz ]; then
				VERSTARBALL=Xupdate.tgz
			fi
		fi
	fi
	Rm -f $VERSIONFILE
	if [ X$VERSTARBALL != X ]; then
		"$TAR" xzf $VERSTARBALL $VERSIONFILE
	fi
	if [ -f $VERSIONFILE ]; then
		BINDISTVERSION=`cat $VERSIONFILE`
		echo ""
		echo "Bindist version is $BINDISTVERSION"
		BINDISTFULLPREFIX=`expr $BINDISTVERSION : '\([0-9]*\.[0-9]*\)\.'`
		BINDISTPATCHLEVEL=`expr $BINDISTVERSION : '[0-9]*\.[0-9]*\.\([0-9]*\)'`
		case $BINDISTPATCHLEVEL in
		99)
			BINDISTFULLVERSION=$BINDISTVERSION
			;;
		*)
			BINDISTFULLVERSION=$BINDISTFULLPREFIX.0
			;;
		esac
	else
		echo "Warning: can't detect the bindist version"
	fi

}

CheckInstallType()
{
	# Check for explicit options

	if [ X"$1" != X ]; then
		case $1 in
		-update)
			DOUPDATE=YES
			DOBASE=NO
			OPTS="$OPTS $1"
			shift
			;;
		-base|-full)
			DOBASE=YES
			DOUPDATE=NO
			OPTS="$OPTS $1"
			shift
			;;
		esac
	fi

	# Auto-detect based on what files are present

	if [ X"$DOUPDATE" = X ]; then
		for i in $BASEDIST; do
			if [ -f $i ]; then
				DOBASE=YES
			fi
		done
		for i in $UPDATEDIST; do
			if [ -f $i ]; then
				DOUPDATE=YES
			fi
		done
		if [ X"$DOBASE" = XYES -a X"$DOUPDATE" = XYES ]; then
			echo ""
			echo "You appear to have binaries in the current directory for"
			echo "both a full release and an update release.  The full release"
			echo "must be installed before installing the update release."
			echo ""
			echo "The full release can be installed by re-running this script"
			echo "with the '-base' option (sh $0 -base)."
			echo ""
			echo "The update release can be installed by re-running this script"
			echo "with the '-update' option (sh $0 -update)."
			echo ""
			exit 1
		fi
	fi
	GetBindistVersion
}

InstallUpdate()
{
	# Check that there's an existing installation.

	missingDir=

	for d in $RUNDIR $RUNDIR/lib $RUNDIR/lib/X11 $RUNDIR/bin; do
		if [ ! -d $d ]; then
			missingDir="$missingDir $d"
		fi
	done

	if [ X$missingDir != X ]; then
		echo ""
		echo "You don't appear to have an existing installation of XFree86."
		echo "You must install the most recent full release ($FULLVERSION)"
		echo "before installing this update release."
		echo ""
		exit 1
	fi

	if [ -f $XSERVERCMD ]; then
		existingVer=`$XSERVERCMD -version 2>&1 | grep "XFree86 Version" | \
						awk '{print $3}'`
		case $existingVer in
		${FULLPREFIX}.*)
			;;
		*)
			echo ""
			echo "This update release should be installed on top of the most"
			echo "recent full release ($FULLVERSION).  Your current version"
			echo "appears to be $existingVer.  Please install $FULLVERSION"
			echo "before installing this update release."
			echo ""
			exit 1
		esac
	fi

	echo ""
	echo "Installing the update binary distribution"
	echo ""
	for i in $UPDATEDIST $EXTRAUPDATE; do
		(cd $RUNDIR; "$EXTRACT" "$WDIR"/$i)
	done

	# Make sure that $RUNDIR/lib isn't group/other writable
	chmod og-w $RUNDIR/lib

	# Need to run ldconfig on some OSs
	case "$OsName" in
	FreeBSD|NetBSD|OpenBSD)
		echo ""
		echo "Running ldconfig"
		# Make sure the directory isn't group-writable
		chmod g-w $RUNDIR/lib
		/sbin/ldconfig -m $RUNDIR/lib
		;;
	Linux)
		echo ""
		echo "Running ldconfig"
		/sbin/ldconfig $RUNDIR/lib
		;;
	esac

	# Run mkfontdir in the local and misc directories to make sure that
	# the fonts.dir files are up to date after the installation.
	echo ""
	for i in $FONTDIRS $EXTRAFONTDIRS; do
		if [ -d $RUNDIR/lib/X11/fonts/$i ]; then
			Echo "Updating the fonts.dir file in $RUNDIR/lib/X11/fonts/$i..."
			$RUNDIR/bin/mkfontdir $RUNDIR/lib/X11/fonts/$i
			echo ""
		fi
	done
	# update Fontconfig cache
	Echo "Updating the index of Freetype fonts..."
	$RUNDIR/bin/fc-cache -v

	echo ""
	echo "Update installation complete."
}

CheckUtil expr
CheckUtil grep

if [ X"$1" = "X-check" ]; then
	GetOsInfo -l
	FindDistName
	exit 0
fi

echo ""
echo "		Welcome to the XFree86(R) $SCRIPTVERSION installer"
echo ""
echo "You are strongly advised to backup your existing XFree86 installation"
echo "before proceeding.  This includes the $ROOTDIR/usr/X11R6, $ROOTDIR/etc/X11"
echo "and $ROOTDIR/etc/fonts directories.  The installation process will"
echo "overwrite existing files in those directories, and this may include"
echo "some configuration files that have been customised either by you"
echo "or by your OS distribution.  Although an attempt is made to make backup"
echo "copies of these files and directories before overwriting them, this is"
echo "not a failsafe procedure.  The backup copies will have a suffix"
echo "\".$BACKUP_SUFFIX\".  The installation is logged.  Refer to the log"
echo "file \"$XINST_LOGFILE\" in the current directory if you"
echo "have problems during or after the installation."
echo ""
echo "When making your own backup of the installation directories before "
echo "proceeding with this installation, do it by making a copy of those"
echo "directories and their contents.  Do not simply rename them.  Otherwise"
echo "you may find that many of your existing applications will no longer be"
echo "visible."
echo ""
echo "If you are installing a version different from $SCRIPTVERSION, you"
echo "may need an updated version of this installer script."
echo ""
ContinueNo

# Should check if uid is zero

# Check if $DISPLAY is set, and warn

if [ X"$DISPLAY" != X ]; then
	echo "\$DISPLAY is set, which may indicate that you are running this"
	echo "installation from an X session.  It is recommended that X not be"
	echo "running while doing the installation."
	echo ""
	ContinueNo
fi

# First, do some preliminary checks

GetOsInfo

CheckUtil sed
CheckUtil fgrep 
CheckUtil basename

# Make OS-specific adjustments

case "$OsName" in
CYGWIN*)
	SERVDIST="Xxserv.tgz"
	;;
Darwin)
	UPDATEDIST="Xupdate.tgz Xdocupd.tgz"
        # On Mac OS X, we require Quartz support
        if [ -d /System/Library/Frameworks/ApplicationServices.framework ]; then
            SERVDIST="Xxserv.tgz Xquartz.tgz"
        else
            SERVDIST="Xxserv.tgz"
        fi
	;;
FreeBSD)
	VARDIST="Xvar.tgz"
	XKBDBDIR="$VARDIR/db/xkb"
	EXTRAOPTDIST="Xdrm.tgz"
	;;
OpenBSD)
	VARDIST="Xvar.tgz"
	XKBDBDIR="$VARDIR/db/xkb"
	;;
NetBSD)
	EXTRAOPTDIST="Xdrm.tgz"
	NoEtcX11="YES"
	;;
Interactive)	# Need the correct name for this
	EXTRADIST="Xbin1.tgz"
	EXTRAOPTDIST="Xxdm.tgz"
	;;
Linux)
	VARDIST="Xvar.tgz"
	XKBDBDIR="$VARDIR/lib/xkb"
	EXTRAOPTDIST="Xdrm.tgz Xtinyx.tgz"
	;;
esac

# Check for extract and extract.exe, and check that they are usable.
#
# This test may not be fool-proof.  A FreeBSD/ELF binary downloaded in
# ASCII mode passed it :-(.
#
if [ -f extract ]; then
	ExtractExists=YES
	chmod +x extract
	if ./extract --version | sed 1q | \
	  fgrep "extract (XFree86 version" > /dev/null 2>&1; then
		ExtractOK=YES
	else
		echo "extract doesn't work properly, renaming it to 'extract.bad'"
		Rm -f extract.bad
		mv extract extract.bad
	fi
fi
if [ X"$ExtractOK" != XYES ]; then
	if [ -f extract.exe ]; then
		ExtractExeExists=YES
		Rm -f extract
		Cp extract.exe extract
		chmod +x extract
		if ./extract --version | sed 1q | \
		  fgrep "extract (XFree86 version" > /dev/null 2>&1; then
			ExtractOK=YES
		else
			echo "extract.exe doesn't work properly, renaming it to"
			echo "'extract.exe.bad'"
			Rm -f extract.exe.bad
			mv extract.exe extract.exe.bad
			Rm -f extract
		fi
	fi
fi
if [ X"$ExtractOK" != XYES ]; then
	echo ""
	if [ X"$ExtractExists" = XYES -a X"$ExtractExeExists" = XYES ]; then
		echo "The versions of 'extract' and 'extract.exe' you have do not run'"
		echo "correctly.  Make sure that you have downloaded the correct"
		echo "binaries for your system.  To find out which is correct,"
		echo "run 'sh $0 -check'."
	fi
	if [ X"$ExtractExists" = XYES -a X"$ExtractExeExists" != XYES ]; then
		echo "The version of 'extract' you have does not run correctly."
		echo "This is most commonly due to problems downloading this file"
		echo "with some web browsers.  You may get better results if you"
		echo "download the version called 'extract.exe' and try again."
	fi
	if [ X"$ExtractExists" != XYES -a X"$ExtractExeExists" = XYES ]; then
		echo "The version of 'extract.exe' you have does not run correctly."
		echo "Make sure that you have downloaded the correct binaries for your"
		echo "system.  To find out which is correct, run 'sh $0 -check'."
	fi
	if [ X"$ExtractExists" != XYES -a X"$ExtractExeExists" != XYES ]; then
		echo "You need to download the 'extract' (or 'extract.exe') utility"
		echo "and put it in this directory."
	fi
	echo ""
	echo "When you have corrected the problem, please re-run 'sh $0$OPTS'"
	echo "to proceed with the installation."
	echo ""
	exit 1
fi

# Link extract to gnu-tar so it can also be used as a regular tar
case "$OsName" in
CYGWIN*)
	Rm -f gnu-tar
	Ln -s extract.exe gnu-tar
	;;
*)
	Rm -f gnu-tar
	Cp extract gnu-tar
	;;
esac

EXTRACT="$WDIR"/extract
TAR="$WDIR"/gnu-tar

CheckInstallType "$@"

if [ X"$DOUPDATE" = XYES ]; then
	REQUIREDFILES=" \
		extract \
		$UPDATEDIST \
		$EXTRAUPDATE
		"
else
	REQUIREDFILES=" \
		extract \
		$BASEDIST \
		$ETCDIST \
		$RCDIST \
		$VARDIST \
		$SERVDIST \
		$EXTRADIST \
		"
fi

echo "Checking for required files ..."
Needed=""

for i in $REQUIREDFILES; do
	if [ ! -f $i ]; then
		Needed="$Needed $i"
	fi
done
if [ X"$Needed" != X ]; then
	echo ""
	echo "The files:"
	echo ""
	echo "$Needed"
	echo ""
	echo "must be present in the current directory to proceed with the"
	echo "installation.  You should be able to find it at the same place"
	echo "you picked up the rest of the XFree86 binary distribution."
	echo "Please re-run 'sh $0$OPTS' to proceed with the installation when"
	echo "you have them."
	echo ""
	exit 1
fi

DoOsChecks

if [ X"$NEEDSOMETHING" != X ]; then
	echo ""
	echo "Please re-run 'sh $0$OPTS' to proceed with the installation after you"
	echo "have made the required updates."
	echo ""
	exit 1
fi

echo ""

# Set version variables if the bindist version was successfully auto-detected.

if [ X$BINDISTVERSION != X ]; then
	VERSION=$BINDISTVERSION
fi
if [ X$BINDISTFULLVERSION != X ]; then
	FULLVERSION=$BINDISTFULLVERSION
fi
if [ X$BINDISTPATCHLEVEL != X ]; then
	PATCHLEVEL=$BINDISTPATCHLEVEL
fi
if [ X$BINDISTFULLPREFIX != X ]; then
	FULLPREFIX=$BINDISTFULLPREFIX
fi

if [ X"$DOUPDATE" = XYES ]; then
	InstallUpdate
	exit 0
fi

if [ -d $RUNDIR -a -d $RUNDIR/bin -a -d $RUNDIR/lib ]; then
	echo ""
	echo "You appear to have an existing installation of X.  Continuing will"
	echo "overwrite it.  You will, however, have the option of being prompted"
	echo "before most configuration files are overwritten."
	echo ""
	ContinueYes
else
	echo ""
	echo "No existing installation of X was found.  I hope you didn't just"
	echo "move it, or you'll likely run into problems later."
	echo ""
	ContinueYes
fi

# Create $RUNDIR, $ETCDIR/X11 and $ETCDIR/fonts if they don't already exist

if [ ! -d $RUNDIR ]; then
	NewRunDir=YES
	echo "Creating $RUNDIR"
	Mkdir $RUNDIR
fi
if [ ! -d $RUNDIR/lib ]; then
	echo "Creating $RUNDIR/lib"
	Mkdir $RUNDIR/lib
fi
if [ ! -d $RUNDIR/lib/X11 ]; then
	echo "Creating $RUNDIR/lib/X11"
	Mkdir $RUNDIR/lib/X11
fi
if [ X$NoEtcX11 != XYES -a ! -d $ETCDIR/X11 ]; then
	NewEtcDir=YES
	echo "Creating $ETCDIR/X11"
	Mkdir $ETCDIR/X11
fi
if [ ! -d $ETCDIR/fonts ]; then
	echo "Creating $ETCDIR/fonts"
	Mkdir $ETCDIR/fonts
fi

if [ X"$OLDFILES" != X ]; then
	echo ""
	echo "Removing some old files that are no longer required..."
	for i in $OLDFILES; do
		if [ -f $i ]; then
			echo "	removing old file $i"
			Rm -f $i
		fi
	done
	echo ""
fi

if [ X"$OLDDIRS" != X ]; then
	echo ""
	echo "Removing some old directories that are no longer required..."
	for i in $OLDDIRS; do
		if [ -d $i ]; then
			echo "	removing old directory $i"
			Rm -fr $i
		fi
	done
	echo ""
fi

# Check for config file directories that may need to be moved.

EtcDirToMove=
EtcFileToMove=
if [ X$NoEtcX11 != XYES -a X"$NoSymLinks" != XYES ]; then
	for i in $ETCDLINKS; do
		if [ -d $RUNDIR/lib/X11/$i -a ! $L $RUNDIR/lib/X11/$i ]; then
			EtcDirToMove="$EtcDirToMove $i"
		fi
	done
	for i in $ETCFLINKS; do
		if [ -f $RUNDIR/lib/X11/$i -a ! $L $RUNDIR/lib/X11/$i ]; then
			EtcFileToMove="$EtcFileToMove $i"
		fi
	done
fi

EtcX11IsUsedBy=
if [ X$NoEtcX11 != XYES -a X"$NoSymLinks" != XYES ]; then
	for i in $ETCDLINKS; do
		if [ -d $ETCDIR/X11/$i -a ! $L $ETCDIR/X11/$i ]; then
			EtcX11IsUsedBy="$EtcX11IsUsedBy $i"
		fi
	done
	for i in $ETCFLINKS; do
		if [ -f $ETCDIR/X11/$i -a ! $L $ETCDIR/X11/$i ]; then
			EtcX11IsUsedBy="$EtcX11IsUsedBy $i"
		fi
	done
fi

if [ X"$EtcDirToMove" != X -o X"$EtcFileToMove" != X ]; then
	echo "XFree86 now installs most customisable configuration files under"
	echo "$ETCDIR/X11 instead of under $RUNDIR/lib/X11, and has symbolic links"
	echo "under $RUNDIR/lib/X11 that point to $ETCDIR.  You currently have"
	echo "files under the following subdirectories of $RUNDIR/lib/X11:"
	echo ""
	echo "$EtcDirToMove $EtcFileToMove"
	echo ""
	echo "Do you want to move them to $ETCDIR/X11 and create the necessary"
	Echo "links? (y/n) [y] "
	read response
	case "$response" in
	[nN]*)
		echo ""
		echo "Note: this means that these run-time config files will remain"
		echo "in the old $RUNDIR/lib/X11 location."
		DoNotMove=YES;
		;;
	*)
		echo ""
		DoNotMove=
		;;
	esac
	echo ""
	if [ X"$NoSymLinks" != XYES -a X"$DoNotMove" != XYES ]; then
		for i in $EtcDirToMove; do
			echo "Moving $RUNDIR/lib/X11/$i to $ETCDIR/X11/$i ..."
			if [ $L $ETCDIR/X11/$i ]; then
				Rm -f $ETCDIR/X11/$i
			fi
			if [ ! -d $ETCDIR/X11/$i ]; then
				Mkdir $ETCDIR/X11/$i
			fi
			"$TAR" -C $RUNDIR/lib/X11/$i -c -f - . | \
				"$TAR" -C $ETCDIR/X11/$i -v -x -p -U -f - && \
				Rm -fr $RUNDIR/lib/X11/$i && \
				Ln -s $ETCDIR/X11/$i $RUNDIR/lib/X11/$i
		ContinueYes
		done
		for i in $EtcFileToMove; do
			echo "Moving $RUNDIR/lib/X11/$i to $ETCDIR/X11/$i ..."
			Cp -p $RUNDIR/lib/X11/$i $ETCDIR/X11/$i && \
				Rm -fr $RUNDIR/lib/X11/$i && \
				Ln -s $ETCDIR/X11/$i $RUNDIR/lib/X11/$i
		ContinueYes
		done
	fi
fi

# Extract Xetc.tgz into a temporary location, and prompt for moving the
# files.

echo "Extracting $ETCDIST and $RCDIST into a temporary location ..."
Rm -fr .etctmp
Mkdir .etctmp
(cd .etctmp; "$EXTRACT" "$WDIR"/$ETCDIST; "$EXTRACT" "$WDIR"/$RCDIST)
echo ""
echo "Configuration files will now be installed.  There are some cases where"
echo "installing these files over existing files will adversely affect your"
echo "previous desktop configuration, and some other cases where the new files"
echo "are essential for the correct operation of the new installation."
echo "These cases will noted before any old files are overwritten."
echo ""
for i in $ETCDLINKS; do
	if [ -d .etctmp/X11/$i ]; then
		DoCopy=YES
	else
		DoCopy=NO
	fi
	Existing=NO
	if [ $DoCopy = YES -a \( -d $RUNDIR/lib/X11/$i -o -d $ETCDIR/X11/$i \) ]
	then
		Existing=YES
		case $i in
		xkb)
			echo ""
			echo "If the XKB configuration files are not updated you may"
			echo "experience problems running the latest XFree86 server."
			echo ""
			;;
		xinit|xdm)
			echo ""
			echo "Installing the $i configuration files may prevent your"
			echo "previous desktop configuration from coming up."
			echo "If you have this problem, restore the saved originals."
			echo ""
			;;
		esac
		Echo "Do you want to overwrite the $i config files? (y/n) [n] "
		read response
		echo ""
		case "$response" in
		[yY]*)
			: OK
			;;
		*)
			DoCopy=NO
			;;
		esac
	fi
	if [ $DoCopy = YES ]; then
		# If no directory, create it.
		if [ ! -d $RUNDIR/lib/X11/$i -a ! -d $ETCDIR/X11/$i ]; then
			if [ X"$EtcX11IsUsedBy" = X ]; then
				Mkdir $RUNDIR/lib/X11/$i
			else
				Mkdir $ETCDIR/X11/$i
			fi
		fi
		if [ -d $RUNDIR/lib/X11/$i -a ! $L $RUNDIR/lib/X11/$i ]; then
			targetdir=$RUNDIR/lib/X11
		else
			targetdir=$ETCDIR/X11
		fi
		if [ X$Existing = XYES -a -d $targetdir/$i ]; then
			backupdir=$targetdir/$i.$BACKUP_SUFFIX
			Rm -fr $backupdir
			Mkdir $backupdir
			echo "Making a backup of $targetdir/$i to $backupdir"
			"$TAR" -C $targetdir/$i -c -f - . | \
				"$TAR" -C $backupdir -x -p -U -f -
		fi

		# Create link to other location if necessary
		if [ X$NoEtcX11 != XYES -a X"$NoSymLinks" != XYES ]; then
			if [ -d $ETCDIR/X11/$i -a ! -d $RUNDIR/lib/X11/$i ]; then
				Ln -s $ETCDIR/X11/$i $RUNDIR/lib/X11/$i
			else
				if [ -d $RUNDIR/lib/X11/$i -a ! -d $ETCDIR/X11/$i ]; then
					Ln -s $RUNDIR/lib/X11/$i $ETCDIR/X11/$i
				fi
			fi
		fi
		echo "Installing the $i config files ..."
		"$TAR" -C .etctmp/X11/$i -c -f - . | \
			"$TAR" -C $targetdir/$i -x -p -U -f -
	fi
done
for i in $ETCFLINKS; do
	if [ -f .etctmp/X11/$i ]; then
		DoCopy=YES
	else
		DoCopy=NO
	fi
	if [ $DoCopy = YES -a -f $RUNDIR/lib/X11/$i ]; then
		Echo "Do you want to overwrite the $i config file? (y/n) [n] "
		read response
		echo ""
		case "$response" in
		[yY]*)
			: OK
			;;
		*)
			DoCopy=NO
			;;
		esac
	fi
	if [ $DoCopy = YES ]; then
		echo "Installing the $i config file ..."
		if [ ! -f $RUNDIR/lib/X11/$i -a ! -f $ETCDIR/X11/$i ]; then
			if [ X"$EtcX11IsUsedBy" = X ]; then
				targetdir=$RUNDIR/lib/X11
			else
				targetdir=$ETCDIR/X11
			fi
		else
			if [ -f $RUNDIR/lib/X11/$i -a ! $L $RUNDIR/lib/X11/$i ]; then
				targetdir=$RUNDIR/lib/X11
			else
				targetdir=$ETCDIR/X11
			fi
		fi
	
		if [ -f $targetdir/$i ]; then
			backupfile=$targetdir/$i.$BACKUP_SUFFIX
			echo "Making a backup of $targetdir/$i to $backupfile"
			rm -f $backupfile
			Cp -p $targetdir/$i $backupfile
		fi
		Cp -p .etctmp/X11/$i $targetdir/$i
		if [ X"$NoSymLinks" != XYES ]; then
			if [ ! -f $RUNDIR/lib/X11/$i ]; then
				Ln -s $ETCDIR/X11/$i $RUNDIR/lib/X11/$i
			else
				if [ ! -f $ETCDIR/X11/$i ]; then
					Ln -s $RUNDIR/lib/X11/$i $ETCDIR/X11/$i
				fi
			fi
		fi
	fi
done

for i in $ETCFONTFILES; do
	if [ -f .etctmp/fonts/$i ]; then
		DoCopy=YES
	else
		DoCopy=NO
	fi
	if [ $DoCopy = YES -a -f $ETCDIR/fonts/$i ]; then
		Echo "Do you want to overwrite the $i config file? (y/n) [n] "
		read response
		echo ""
		case "$response" in
		[yY]*)
			: OK
			;;
		*)
			DoCopy=NO
			;;
		esac
	fi
	if [ $DoCopy = YES ]; then
		targetdir=$ETCDIR/fonts
		if [ -f $targetdir/$i ]; then
			backupfile=$targetdir/$i.$BACKUP_SUFFIX
			echo "Making a backup of $targetdir/$i to $backupfile"
			rm -f $backupfile
			Cp -p $targetdir/$i $backupfile
		fi
		echo "Installing the $i config file ..."
		Cp -p .etctmp/fonts/$i $ETCDIR/fonts/$i
	fi
done
Rm -fr .etctmp

echo ""
echo "Installing the mandatory parts of the binary distribution"
echo ""
for i in $BASEDIST $SERVDIST; do
	(cd $RUNDIR; "$EXTRACT" "$WDIR"/$i)
done
if [ X"$VARDIST" != X ]; then
	(cd $VARDIR; "$EXTRACT" "$WDIR"/$VARDIST)
fi

if [ X"$XKBDIR" != X -a X"$XKBDBDIR" != X ]; then
	Rm -fr $RUNDIR/lib/X11/xkb/compiled
	Ln -s $XKBDBDIR $RUNDIR/lib/X11/xkb/compiled
fi

echo "Checking for post-release updates ..."
for i in $UPDDIST; do
	if [ -f $i ]; then
		Echo "Do you want to install update $i `(Description $i)`? (y/n) [y] "
		read response
		echo ""
		case "$response" in
		[nN]*)
			: skip this one
			;;
		*)
			(cd $RUNDIR; "$EXTRACT" "$WDIR"/$i)
			;;
		esac
	fi
done

echo "Checking for optional components to install ..."
for i in $OPTDIST $EXTRAOPTDIST; do
	if [ -f $i ]; then
		Echo "Do you want to install $i (`Description $i`)? (y/n) [y] "
		read response
		echo ""
		case "$response" in
		[nN]*)
			: skip this one
			;;
		*)
			(cd $RUNDIR; "$EXTRACT" "$WDIR"/$i)
			;;
		esac
	fi
done

echo ""
echo "Searching for a termcap file.  This may take a few seconds..."

# Check if the system has a termcap file
TERMCAP1DIR=$ROOTDIR/usr/share
TERMCAP2=$ROOTDIR/etc/termcap
if [ -d $TERMCAP1DIR ]; then
	TERMCAP1=`find $TERMCAP1DIR -type f -name termcap -print 2> /dev/null`
	if [ x"$TERMCAP1" != x ]; then
		TERMCAPFILE="$TERMCAP1"
	fi
fi
if [ x"$TERMCAPFILE" = x ]; then
	if [ -f $TERMCAP2 ]; then
		TERMCAPFILE="$TERMCAP2"
	fi
fi

# Override this for some OSs

case "$OsName" in
OpenBSD)
	TERMCAPFILE=""
	;;
esac

if [ X"$TERMCAPFILE" != X ]; then
	echo ""
	echo "You appear to have a termcap file: $TERMCAPFILE"
	echo "This should be edited manually to replace the xterm entries"
	echo "with those in $RUNDIR/lib/X11/etc/xterm.termcap"
	echo ""
	echo "Note: the new xterm entries are required to take full advantage"
	echo "of new features, but they may cause problems when used with"
	echo "older versions of xterm.  A terminal type 'xterm-r6' is included"
	echo "for compatibility with the standard X11R6 version of xterm."
	echo ""
	ContinueYes
fi

# Check for terminfo, and update the xterm entry
TINFODIR=$ROOTDIR/usr/lib/terminfo
# Does this list need to be updated?
OLDTINFO=" \
	x/xterm \
	x/xterms \
	x/xterm-24 \
	x/xterm-vi \
	x/xterm-65 \
	x/xterm-bold \
	x/xtermm \
	x/xterm-boldso \
	x/xterm-ic \
	x/xterm-r6 \
	x/xterm-old \
	x/xterm-r5 \
	v/vs100"

if [ -d $TINFODIR ]; then
	echo ""
	echo "You appear to have a terminfo directory: $TINFODIR"
	echo "New xterm terminfo entries can be installed now."
	echo ""
	echo "Note: the new xterm entries are required to take full advantage"
	echo "of new features, but they may cause problems when used with"
	echo "older versions of xterm.  A terminal type 'xterm-r6' is included"
	echo "for compatibility with the standard X11R6 version of xterm."
	echo ""
	echo "Do you wish to have the new xterm terminfo entries installed"
	Echo "now (y/n)? [n] "
	read response
	echo ""
	case "$response" in
	[yY]*)
		echo ""
		for t in $OLDTINFO; do
			if [ -f $TINFODIR/$t ]; then
				echo "Moving old terminfo file $TINFODIR/$t to $TINFODIR/$t.bak"
				Rm -f $TINFODIR/$t.bak
				mv -f $TINFODIR/$t $TINFODIR/$t.bak
			fi
		done
		echo ""
		echo "Installing new terminfo entries for xterm."
		echo ""
		echo "On some systems you may get warnings from tic about 'meml'"
		echo "and 'memu'.  These warnings can safely be ignored."
		echo ""
		tic $RUNDIR/lib/X11/etc/xterm.terminfo
		;;
	*)
		echo ""
		echo "Not installing new terminfo entries for xterm."
		echo "They can be installed later by running:"
		echo ""
		echo "  tic $RUNDIR/lib/X11/etc/xterm.terminfo"
		;;
	esac
fi

if [ -f $RUNDIR/lib/libGL.so ]; then
	existing=""
	if [ -f $ROOTDIR/usr/lib/libGL.so ]; then
		existing="$existing /usr/lib/libGL.so"
	fi
	if [ -f $ROOTDIR/usr/lib/libGL.so.1 ]; then
		existing="$existing /usr/lib/libGL.so.1"
	fi
	if [ -d $ROOTDIR/usr/include/GL ]; then
		existing="$existing /usr/include/GL"
	fi
	echo ""
	echo "On some platforms (e.g., Linux), the OpenGL standard requires"
	echo "that the GL shared library and header files be visible from the"
	echo "standard system lib and include directories ($ROOTDIR/usr/lib and"
	echo "$ROOTDIR/usr/include).  This can be done by installing links in those"
	echo "directories to the files that have been installed under $RUNDIR."
	echo ""
	echo "NOTE: installing these links will overwrite existing files or"
	echo "links."
	if [ X"$existing" != X ]; then
		echo ""
		echo "The follwing links/files/directories already exist:"
		echo ""
		ls -ld $existing
	fi
	echo ""
	Echo "Do you wish to have the (new) links installed (y/n)? [n] "
	read response
	case "$response" in
	[yY]*)
		if [ ! -d $ROOTDIR/usr/lib ]; then
			echo "Creating $ROOTDIR/usr/lib"
			Mkdir $ROOTDIR/usr/lib
		fi
		if [ ! -d $ROOTDIR/usr/include ]; then
			echo "Creating $ROOTDIR/usr/include"
			Mkdir $ROOTDIR/usr/include
		fi
		Rm -f $ROOTDIR/usr/lib/libGL.so
		if [ ! -f $ROOTDIR/usr/lib/libGL.so ]; then
			echo "Creating link from $RUNDIR/lib/libGL.so to $ROOTDIR/usr/lib/libGL.so"
			Ln -s $RUNDIR/lib/libGL.so $ROOTDIR/usr/lib/libGL.so
		else
			echo "Could not remove existing $ROOTDIR/usr/lib/libGL.so, so the new"
			echo "link has not been created."
		fi
		Rm -f $ROOTDIR/usr/lib/libGL.so.1
		if [ ! -f $ROOTDIR/usr/lib/libGL.so.1 ]; then
			echo "Creating link from $RUNDIR/lib/libGL.so.1 to $ROOTDIR/usr/lib/libGL.so.1"
			Ln -s $RUNDIR/lib/libGL.so.1 $ROOTDIR/usr/lib/libGL.so.1
		else
			echo "Could not remove existing $ROOTDIR/usr/lib/libGL.so.1, so the new"
			echo "link has not been created."
		fi
		if [ -d $RUNDIR/include/GL ]; then
			Rm -f $ROOTDIR/usr/include/GL
			if [ ! -d $ROOTDIR/usr/include/GL ]; then
				echo "Creating link from $RUNDIR/include/GL to $ROOTDIR/usr/include/GL"
				Ln -s $RUNDIR/include/GL $ROOTDIR/usr/include/GL
			else
				echo "Could not remove existing $ROOTDIR/usr/include/GL, so the new"
				echo "link has not been created."
			fi
		fi
		;;
	esac
fi

# Check for documentation files with old extensions and remove them.
for i in $RUNDIR/lib/X11/doc/*.txt; do
	base=`basename $i .txt`
	old="RUNDIR/lib/X11/doc/$base.TXT"
	if [ -f "$old" ]; then
		echo "Removing old file $old (replaced by $base.txt)"
		Rm -f "$old"
	fi
done

for i in $RUNDIR/lib/X11/doc/PostScript/*.PS; do
	base=`basename $i .ps`
	old="RUNDIR/lib/X11/doc/$base.PS"
	if [ -f "$old" ]; then
		echo "Removing old file $old (replaced by $base.ps)"
		Rm -f "$old"
	fi
done


# Need to run ldconfig on some OSs
case "$OsName" in
FreeBSD|NetBSD|OpenBSD)
	echo ""
	echo "Running ldconfig"
	# Make sure the directory isn't group-writable
	chmod g-w $RUNDIR/lib
	/sbin/ldconfig -m $RUNDIR/lib
	;;
Linux)
	echo ""
	echo "Running ldconfig"
	/sbin/ldconfig $RUNDIR/lib
	;;
esac

# Run mkfontdir in the local and misc directories to make sure that
# the fonts.dir files are up to date after the installation.
echo ""
for i in $FONTDIRS $EXTRAFONTDIRS; do
	if [ -d $RUNDIR/lib/X11/fonts/$i ]; then
		Echo "Updating the fonts.dir file in $RUNDIR/lib/X11/fonts/$i..."
		$RUNDIR/bin/mkfontdir $RUNDIR/lib/X11/fonts/$i
		echo ""
	fi
done


if [ -f $RUNDIR/bin/rstartd ]; then
	echo ""
	echo "If you are going to use rstart and $RUNDIR/bin isn't in the"
	echo "default path for commands run remotely via rsh, you will need"
	echo "a link to rstartd installed in $ROOTDIR/usr/bin."
	echo ""
	Echo "Do you wish to have this link installed (y/n)? [n] "
	read response
	echo ""
	case "$response" in
	[yY]*)
		if [ ! -d $ROOTDIR/usr/bin ]; then
			echo "Creating $ROOTDIR/usr/bin"
			Mkdir $ROOTDIR/usr/bin
		fi
		echo "Creating link from $RUNDIR/bin/rstartd to $ROOTDIR/usr/bin/rstartd"
		Rm -f $ROOTDIR/usr/bin/rstartd
		Ln -s $RUNDIR/bin/rstartd $ROOTDIR/usr/bin/rstartd
		;;
	esac
fi


# Finally, check for old 3.3.x modules that will conflict with 4.x
if [ -d $RUNDIR/lib/modules ]; then
	for i in $OLDMODULES; do
		if [ -f $RUNDIR/lib/modules/$i ]; then
			ModList="$ModList $i"
		fi
	done
	if [ X"$ModList" != X ]; then
		echo ""
		echo "The following 3.3.x X server modules were found in"
		echo "$RUNDIR/lib/modules, and they may cause problems when running"
		echo "$VERSION:"
		echo ""
		echo "  $ModList"
		echo ""
		echo "Do you want them moved to $RUNDIR/lib/modules/old?"
		echo "Note: that if you want to use them with 3.3.x again, you'll"
		Echo "need to move them back manually. (y/n) [n] "
		read response
		echo ""
		case "$response" in
		[yY]*)
			if [ ! -d $RUNDIR/lib/modules/old ]; then
				echo ""
				echo "Creating $RUNDIR/lib/modules/old"
				Mkdir $RUNDIR/lib/modules/old
			else
				echo ""
			fi
			if [ -d $RUNDIR/lib/modules/old ]; then
				for i in $ModList; do
					echo "Moving $i to $RUNDIR/lib/modules/old"
					mv $RUNDIR/lib/modules/$i $RUNDIR/lib/modules/old/$i
				done
			else
				echo "Failed to create directory $RUNDIR/lib/modules/old"
			fi
			;;
		*)
			echo ""
			echo "Make sure that you rename, move or delete the old modules"
			echo "before running $VERSION."
		esac
	fi
	# Some distributions have old codeconv modules
	if [ -d $RUNDIR/lib/modules/codeconv ]; then
		if [ -f $RUNDIR/lib/modules/codeconv/ISO8859_1.so ]; then
			echo ""
			echo "Warning: it looks like there are some old *.so modules"
			echo "in $RUNDIR/lib/modules/codeconv.  You may need to rename,"
			echo "move or delete them if you use the xtt font module."
		fi
	fi
fi

case "$OsName" in
FreeBSD|NetBSD)
	DRMBUILDDIR="$RUNDIR/src/drm/bsd/kernel"
	;;
Linux)
	DRMBUILDDIR="$RUNDIR/src/drm/linux/kernel"
	;;
esac

DoDrmBuild=

if [ -f $DRMBUILDDIR/Makefile ]; then
	echo ""
	echo "If you have source for your current kernel installed, you can"
	echo "have new DRM kernel modules built and installed.  Note: this will"
	echo "overwrite any existing DRM kernel modules you may have installed."
	echo "If you'd prefer to save them before installing new ones, answer 'n'"
	echo "here and follow the build/install instructions manually later."
	case "$OsName" in
	Linux)
		echo ""
		echo "The Linux DRM module build process assumes not only that you"
		echo "have the source for your current kernel installed, but that"
		echo "have run a build in that source tree."
		;;
	esac
	echo ""
	Echo "Do you want to build and install new DRM kernel modules? (y/n) [n] "
	read response
	echo ""
	case "$response" in
	[yY]*)
		DoDrmBuild=1
		;;
	*)
		echo ""
		echo "To build the DRM modules manually, run 'make' from the"
		echo "$DRMBUILDDIR directory.  Once built, install"
		echo "them by running 'make install' from the same directory."
		;;
	esac
fi

if [ X"$DoDrmBuild" != X ]; then
	cd $DRMBUILDDIR && \
		echo "" && \
		echo "Building DRM modules" && \
		make clean > build.log 2>&1 && \
		make >> build.log 2>&1
	if [ $? != 0 ]; then
		echo ""
		echo "DRM module build failed."
		echo "See $DRMBUILDDIR/build.log for details."
	else
		echo ""
		echo "Installing DRM modules"
		make install > install.log 2>&1
		if [ $? != 0 ]; then
			echo ""
			echo "DRM module install failed."
			echo "See $DRMBUILDDIR/install.log for details."
		else
			echo ""
			echo "DRM module build and install was successful."
			echo "You may need to manually unload the old module and reload"
			echo "the new ones, or reboot to have this done automatically."
		fi
	fi
	cd $WDIR
fi

echo ""
echo "Installation complete."

exit 0
### Local Variables: 	***
### tab-width: 4 		***
### End:				***
