#!/usr/bin/perl
#
# ucs2any.pl -- Markus Kuhn <mkuhn@acm.org>
#
# $XFree86: xc/fonts/util/ucs2any.pl,v 1.3 2000/12/08 16:45:16 dawes Exp $
#
# This Perl script allows you to generate from an ISO10646-1 encoded
# BDF font other BDF fonts in any possible encoding. This way, you can
# derive from a single ISO10646-1 master font a whole set of 8-bit
# fonts in all ISO 8859 and various other encodings. (Note that
# a future XFree86 release (probably 4.1) will have a similar
# facility built into the server, which can reencode ISO10646-1
# on the fly, because storing the same fonts in many different
# encodings is clearly a waste of storage capacity).
#
# Id: ucs2any.pl,v 1.9 2000-06-26 14:15:13+01 mgk25 Rel mgk25 $

print <<End if $#ARGV < 0;

Usage: ucs2any.pl <source-name> { <mapping-file> <registry-encoding> }

where

   <source-name>        is the name of an ISO10646-1 encoded BDF file

   <mapping-file>       is the name of a character set table like those on
                        <ftp://ftp.unicode.org/Public/MAPPINGS/> or
                        <ftp://dkuug.dk/i18n/WG15-collection/charmaps/>

   <registry-encoding>  are the CHARSET_REGISTRY and CHARSET_ENCODING
                        field values for the font name (XLFD) of the
                        target font, separated by a hyphen

Example:

   ucs2any.pl 6x13.bdf 8859-1.TXT iso8859-1 8859-2.TXT iso8859-2

will generate the files 6x13-iso8859-1.bdf and 6x13-iso8859-2.bdf

End

exit if $#ARGV < 0;

# open and read source file
$fsource = $ARGV[0];
open(FSOURCE,  "<$fsource")  || die ("Can't read file '$fsource': $!\n");

# read header
$properties = 0;
while (<FSOURCE>) {
    last if /^CHARS\s/;
    if (/^STARTFONT/) {
	$startfont = $_;
    } elsif (/^_XMBDFED_INFO\s/ || /^_XFREE86_GLYPH_RANGES\s/) {
	$properties--;
    } elsif (/DEFAULT_CHAR\s+([0-9]+)\s*$/) {
	$default_char = $1;
	$header .= "DEFAULT_CHAR 0\n";
    } else {
	if (/^STARTPROPERTIES\s+(\d+)/) {
	    $properties = $1;
	} elsif (/^FONT\s+(.*-([^-]*-\S*))\s*$/) {
	    if ($2 ne "ISO10646-1") {
		die("FONT name in '$fsource' is '$1' and " .
		    "not '*-ISO10646-1'!\n");
	    };
	} elsif (/^CHARSET_REGISTRY\s+"(.*)"\s*$/) {
	    if ($1 ne "ISO10646") {
		die("CHARSET_REGISTRY in '$fsource' is '$1' and " .
		    "not 'ISO10646'!\n");
	    };
	} elsif (/^CHARSET_ENCODING\s+"(.*)"\s*$/) {
	    if ($1 ne "1") {
		die("CHARSET_ENCODING in '$fsource' is '$1' and " .
		    "not '1'!\n");
	    };
        } elsif (/^SLANT\s+"(.*)"\s*$/) {
	    $slant = $1;
	    $slant =~ tr/a-z/A-Z/;
	}
	s/^COMMENT\s+\"(.*)\"$/COMMENT $1/;
	s/^COMMENT\s+\$[I]d: (.*)\$\s*$/COMMENT Derived from $1\n/;
        $header .= $_;
    }
}

die ("No STARTFONT line found in '$fsource'!\n") unless $startfont;
$header =~ s/\nSTARTPROPERTIES\s+(\d+)\n/\nSTARTPROPERTIES $properties\n/;

# read characters
while (<FSOURCE>) {
    if (/^STARTCHAR/) {
	$sc = $_;
	$code = -1;
    } elsif (/^ENCODING\s+(-?\d+)/) {
        $code = $1;
	$startchar{$code} = $sc;
	$char{$code} = "";
    } elsif (/^ENDFONT$/) {
	$code = -1;
	$sc = "STARTCHAR ???\n";
    } else {
        $char{$code} .= $_;
        if (/^ENDCHAR$/) {
            $code = -1;
	    $sc = "STARTCHAR ???\n";
        }
    }
}
close FSOURCE;
delete $char{-1};


shift(@ARGV);
while ($#ARGV > 0) {
    $fmap = $ARGV[0];
    if ($ARGV[1] =~ /^([^-]+)-([^-]+)$/) {
	$registry = $1;
	$encoding = $2;
    } else {
	die("Argument registry-encoding '$ARGV[1]' not in expected format!\n");
    }

    shift(@ARGV);
    shift(@ARGV);

    # open and read source file
    open(FMAP,  "<$fmap")
	|| die ("Can't read mapping file '$fmap': $!\n");
    %map = ();
    while (<FMAP>) {
        next if /^\s*\#/;
        if (/^\s*(0[xX])?([0-9A-Fa-f]{2})\s+(0[xX]|U\+|U-)?([0-9A-Fa-f]{4})\s*/ ||
	    /^<(.*)>\s+\/x([0-9A-Fa-f]{2})\s+<U()([0-9A-Fa-f]{4})>/) {
	    $target = hex($2);
	    $ucs = hex($4);
	    if ($startchar{$ucs}) {
		$map{$target} = $ucs;
	    } else {
		printf STDERR "No glyph for character U+%04X " .
		    "(0x%02x) available.\n", $ucs, $target
			unless $ucs < 32 || ($ucs >= 127 && $ucs < 160) ||
			   ($ucs >= 0x2500 && $ucs <= 0x25FF && $slant ne "R");
	    }
	}
    }
    close FMAP;

    # add default character
    if (!(defined($map{0}) && $startchar{$map{0}})) {
	if (defined($default_char) && $startchar{$default_char}) {
	    $map{0} = $default_char;
	    $startchar{$default_char} = "STARTCHAR defaultchar\n";
	} else {
	    printf STDERR "No default character defined.\n";
	}
    }
    # pass through C0 DEC VT100 characters if they happen to be present
    # in the source font
    for ($i = 0; $i < 32; $i++) {
	if ($startchar{$i} && !$map{$i}) {
	    $map{$i} = $i;
	    $startchar{$i} = "STARTCHAR char$i\n";
	}
    }

    @chars = sort {$a <=> $b;} keys(%map);
    if ($fsource =~ /^(.*).bdf$/i) {
	$fout = $1 . "-$registry-$encoding.bdf";
    } else {
	$fout = $fsource . "-$registry-$encoding";
    }
    $fout =~ s/^(.*\/)?([^\/]+)$/$2/;  # remove path prefix

    # write new BDF file
    printf STDERR "Writing %d characters into file '$fout'.\n", $#chars + 1;
    open(FOUT,  ">$fout")
	|| die ("Can't write file '$fout': $!\n");
    
    print FOUT $startfont;
    print FOUT "COMMENT AUTOMATICALLY GENERATED FILE. DO NOT EDIT!\n";
    print FOUT "COMMENT Generated with 'ucs2any.pl $fsource $fmap " .
	"$registry-$encoding'\n";
    print FOUT "COMMENT from an ISO10646-1 encoded source BDF font.\n";
    print FOUT "COMMENT ucs2any.pl by Markus Kuhn <mkuhn\@acm.org>, 2000.\n";
    $newheader = $header;
    $newheader =~
	s/\nFONT\s+(.*-)[^-\s]+-\S*\n/\nFONT $1$registry-$encoding\n/;
    $newheader =~
	s/\nCHARSET_REGISTRY\s+.*\n/\nCHARSET_REGISTRY "$registry"\n/;
    $newheader =~
	s/\nCHARSET_ENCODING\s+.*\n/\nCHARSET_ENCODING "$encoding"\n/;
    print FOUT $newheader;
    printf FOUT "CHARS %d\n", $#chars + 1;

    # Write characters
    for $target (@chars) {
	$ucs = $map{$target};
	print FOUT $startchar{$ucs};
	print FOUT "ENCODING $target\n";
	print FOUT $char{$ucs};
    }

    print FOUT "ENDFONT\n";

    close(FOUT);
}
