# Client

This dir contains various "client"-type programs that aren't really part
of ctwm, but may be useful adjuncts to it, or possible skeletons for
future useful adjuncts.  These programs are generally not maintained or
vetted to the same degree as ctwm itself, and are provided mostly for
special-case uses or as examples.

Use at your own risk.

## Building

This directory is not built by default.  To enable it, set the
`DO_CLIENT` flag in your `cmake` invocation.  e.g.,

    % make CMAKE_EXTRAS="-DDO_CLIENT=ON"

## Roadmap

A quick summary of the things in here

libctwmc
:       A library for some routines potentially useful for querying info
        about a running `ctwm` instance.

demolib
:       A small program that links against `libctwmc` and demonstrates a
        few of its functions.  It probably won't work at all, and won't
        work right unless you're Claude with Claude's config and windows.

gtw
:       A small program to switch to a workspace or change a window's
        occupation from the command line.  Doesn't use `libctwmc`.

forward
:       A standalone utility that attempts to forward events from
        "desktop environment" style desktop windows to the real root
        window, so that a ctwm running in those sorts of environments can
        get access to the keystrokes/mouseclicks.  Appears to generally
        not _quite_ work right in most cases.  May be a simple fix, but
        nobody has spent enough time to find it.


{>>
 vim:expandtab:ft=markdown:
<<}
