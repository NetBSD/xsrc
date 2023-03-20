#
# Copyright (c) 2023, Oracle and/or its affiliates.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#

project('xorg-cf-files', version: '1.0.8', meson_version: '>=0.60.0')
conf = configuration_data()
conf.set('prefix', get_option('prefix'))

# Replaces XORG_FONTROOTDIR from xorg/font/util/fontutil.m4
fontrootdir = get_option('with-fontrootdir')
if fontrootdir == ''
    fontutil_dep = dependency('fontutil', required: false)
    if fontutil_dep.found() and fontutil_dep.type_name() == 'pkgconfig'
        fontrootdir = fontutil_dep.get_variable(pkgconfig: 'fontrootdir')
    endif
    if fontrootdir == ''
        fontrootdir = get_option('prefix') / get_option('datadir') / '/fonts/X11'
    endif
endif
conf.set('FONTROOTDIR', fontrootdir)

# Replaces XORG_FONTSUBDIR([ENCODINGSDIR],[encodingsdir],[encodings])
# with the substitution to use the FontDir defined in the Imake configs
encodingsdir = get_option('with-fontrootdir')
if encodingsdir == ''
    encodingsdir = 'FontDir' / 'encodings'
endif
conf.set('ENCODINGSDIR', encodingsdir)

xconfdir = get_option('with-config-dir')
if xconfdir == ''
    xconfdir = get_option('prefix') / get_option('libdir') / 'X11/config'
endif

site_def = configure_file(input: 'site.def.in', output: 'site.def',
               configuration: conf)

def_files = [
    'cross.def',
    site_def,
    'xf86site.def',
    'xorgsite.def',
    'xorgversion.def'
]

rule_files = [
    'Imake.rules',
    'Motif.rules',
    'OpenBSDLib.rules',
    'Win32.rules',
    'X11.rules',
    'bsdiLib.rules',
    'bsdLib.rules',
    'cde.rules',
    'cross.rules',
    'cygwin.rules',
    'mingw.rules',
    'darwinLib.rules',
    'gnuLib.rules',
    'hpLib.rules',
    'ibmLib.rules',
    'lnxLib.rules',
    'lnxdoc.rules',
    'necLib.rules',
    'noop.rules',
    'nto.rules',
    'oldlib.rules',
    'os2.rules',
    'os2Lib.rules',
    'osfLib.rules',
    'scoLib.rules',
    'sequentLib.rules',
    'sgiLib.rules',
    'sunLib.rules',
    'sv3Lib.rules',
    'sv4Lib.rules',
    'QNX4.rules',
    'xf86.rules'
]

tmpl_files = [
    'Imake.tmpl',
    'Library.tmpl',
    'Motif.tmpl',
    'OpenBSDLib.tmpl',
    'Server.tmpl',
    'ServerLib.tmpl',
    'Threads.tmpl',
    'WinLib.tmpl',
    'X11.tmpl',
    'bsdiLib.tmpl',
    'bsdLib.tmpl',
    'cde.tmpl',
    'cygwin.tmpl',
    'mingw.tmpl',
    'darwinLib.tmpl',
    'hpLib.tmpl',
    'gnuLib.tmpl',
    'ibmLib.tmpl',
    'lnxLib.tmpl',
    'lnxdoc.tmpl',
    'necLib.tmpl',
    'os2Lib.tmpl',
    'osfLib.tmpl',
    'sgiLib.tmpl',
    'sunLib.tmpl',
    'sv3Lib.tmpl',
    'sv4Lib.tmpl',
    'xf86.tmpl',
    'xorg.tmpl'
]

cf_files = [
    'Imake.cf',
    'Amoeba.cf',
    'DGUX.cf',
    'dmx.cf',
    'DragonFly.cf',
    'FreeBSD.cf',
    'Mips.cf',
    'NetBSD.cf',
    'OpenBSD.cf',
    'Oki.cf',
    'cygwin.cf',
    'mingw.cf',
    'Win32.cf',
    'apollo.cf',
    'bsd.cf',
    'bsdi.cf',
    'convex.cf',
    'cray.cf',
    'fujitsu.cf',
    'generic.cf',
    'gnu.cf',
    'hp.cf',
    'ibm.cf',
    'isc.cf',
    'linux.cf',
    'luna.cf',
    'macII.cf',
    'mach.cf',
    'minix.cf',
    'moto.cf',
    'ncr.cf',
    'nec.cf',
    'nto.cf',
    'os2.cf',
    'osf1.cf',
    'pegasus.cf',
    'sco.cf',
    'sco5.cf',
    'sequent.cf',
    'sgi.cf',
    'sony.cf',
    'sun.cf',
    'svr3.cf',
    'svr4.cf',
    'ultrix.cf',
    'usl.cf',
    'lynx.cf',
    'x386.cf',
    'QNX4.cf',
    'darwin.cf',
    'xfree86.cf',
    'xorg.cf'
]

data_files = [ rule_files, tmpl_files, def_files, cf_files ]

install_data(data_files, install_dir: xconfdir)

foreach target_def: ['date.def', 'host.def', 'version.def']
  install_data('empty.def', rename: target_def, install_dir: xconfdir)
endforeach
