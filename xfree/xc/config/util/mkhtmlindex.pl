#!/usr/bin/perl
#
# $XFree86: xc/config/util/mkhtmlindex.pl,v 1.5 2004/06/01 00:16:55 dawes Exp $
#
# Copyright © 2000,2001 by VA Linux Systems, Inc.
# Copyright © 2004 by David H. Dawes.
#
# Copyright (c) 2004 by The XFree86 Project, Inc.
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
# Generate index files for HTML man pages.
#
# Author:	David Dawes <dawes@xfree86.org>
#

#
# Best viewed with tabs set to 4
#

if ($#ARGV ne 0) {
	print STDERR "Usage: mkhtmlindex.pl htmlmandir\n";
	exit 1;
}

$dir = $ARGV[0];

if (! -d $dir) {
	print STDERR "$dir is not a directory\n";
	exit 1;
}

@vollist = ("1", "2", "3", "4", "5", "6", "7", "8", "9", "o", "l", "n", "p");

$indexprefix = "manindex";

foreach $vol (@vollist) {
	$empty = "yes";
	$indexname="$dir/$indexprefix$vol.html";

	# print "Processing volume $vol\n";

	open(mindex, ">$indexname") || die "Can't create $indexname";
	opendir(dir, "$dir") || die "Can't open $dir";

	print mindex <<EOF;
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2 Final//EN">
<HTML>
<HEAD>
<META HTTP-EQUIV="Content-Type" CONTENT="text/html; charset=iso-8859-1">
<TITLE>XFree86[tm] Manual pages: Section $vol</TITLE>
</HEAD>
<BODY BGCOLOR="#efefef" TEXT="black" LINK="blue" VLINK="#551A8B" ALINK="red">

<H1>XFree86[tm] Manual pages: Section $vol</H1>
<P>
<UL>
EOF

	foreach $file (sort readdir dir) {
		if ($file =~ /\.$vol\.html/) {
			open(file, "<$dir/$file") || die "Can't open $dir/$file";
			while (<file>) {
				chop;
				if (/^<H2>/i) {
					if (! /<\/H2>$/i) {
						while (<file> && ! /<\/H2>$/i) {
							;
						}
					}
					$heading = "";
					while (<file>) {
						if (/^<H2>/i) {
							last;
						}
						$heading = "$heading" . "$_";
					}
					if ($heading) {
						undef $empty;
						$heading =~ s/--/-/;
						($name, $descr) = split(/-/, $heading, 2);
						$file =~ /(.*)\.$vol\.html/;
						$fname = $1;
						$descr =~ s/<[Pp]>//g;
						print mindex
							"<LI><A href=\"$file\">$fname</A> - $descr</LI>";
					}
					last;
				}
			}
			close file;
		}
	}

	print mindex <<EOF;
</UL>
<P>
</BODY>
</HTML>
EOF

	close mindex;
	closedir dir;
	if (defined $empty) {
		# print "Removing empty $indexname\n";
		unlink $indexname;
	}
}
