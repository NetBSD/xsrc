#!/usr/bin/perl
#
# bdftruncate.pl -- Markus Kuhn <mkuhn@acm.org>
#
# This Perl script allows you to generate from an ISO10646-1 encoded
# BDF font other ISO10646-1 BDF fonts in which all characters above
# a threshold code value are stored unencoded.
#
# Id: bdftruncate.pl,v 1.4 2000-06-26 14:24:07+01 mgk25 Rel mgk25 $

print STDERR <<End if $#ARGV != 0;

Usage: bdftruncate.pl threshold <source.bdf >destination.bdf

Example:

   ucs2any.pl 0x3200 <6x13.bdf >6x13t.bdf

will generate the file 6x13t.bdf in which all glyphs with codes
>= 0x3200 will only be stored unencoded (i.e., ENCODING -1).

End

exit 1 if $#ARGV != 0;

# read threshold value from command line
$threshold = $ARGV[0];
if ($threshold =~ /^(0[xX]|U[+-]?)([0-9a-fA-F]+)$/) {
    $threshold = hex($2);
} elsif (!($threshold =~ /^[0-9]+$/)) {
    die("Illegal threshold '$threshold'!\n");
}

# filter file
while (<STDIN>) {
    if (/^ENCODING\s+(-?\d+)/ && $1 >= $threshold) {
	print "ENCODING -1\n";
    } elsif (/^STARTFONT/) {
	print;
	print "COMMENT AUTOMATICALLY GENERATED FILE. DO NOT EDIT!\n";
	printf("COMMENT In this version of the font file, all characters >= " .
	       "U+%04X are\nCOMMENT not encoded to keep XFontStruct small.\n",
	       $threshold);
    } else {
	s/^COMMENT\s+\"(.*)\"$/COMMENT $1/;
        s/^COMMENT\s+\$[I]d: (.*)\$\s*$/COMMENT Derived from $1\n/;
	print;
    }
}
