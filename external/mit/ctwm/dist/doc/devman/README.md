# Developer's Manual

You'll need [asciidoctor](http://asciidoctor.org/) to build the manual.
With a little manual hackery of the generated Makefile, you can try using
the old python asciidoc instead; it will probably _mostly_ work, but
you'll get some odd formatting in at least some cases.  No attempt is
made to remain compatible (unlike the user manual, where compatibility is
still currently intended).

    % ./mkmk.sh
    % make
    # view HTML in build/

If you add new `*.adoc` files, you'll need to rerun `mkmk.sh`.
