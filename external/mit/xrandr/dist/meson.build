# SPDX-License-Identifier: HPND-sell-variant
#
# Copyright (c) 2026, Oracle and/or its affiliates.
#
# Permission to use, copy, modify, distribute, and sell this software and its
# documentation for any purpose is hereby granted without fee, provided that
# the above copyright notice appear in all copies and that both that copyright
# notice and this permission notice appear in supporting documentation, and
# that the name of the copyright holders not be used in advertising or
# publicity pertaining to distribution of the software without specific,
# written prior permission.  The copyright holders make no representations
# about the suitability of this software for any purpose.  It is provided "as
# is" without express or implied warranty.
#
# THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
# INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
# EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
# CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
# DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
# TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
# OF THIS SOFTWARE.
#

project(
  'xrandr',
  'c',
  version: '1.5.4',
  license: 'HPND-sell-variant',
  license_files: 'COPYING',
  meson_version: '>= 1.3.0',
)

conf = configuration_data()
package_string = ' '.join(meson.project_name(), meson.project_version())
conf.set_quoted('VERSION', meson.project_version())

cc = meson.get_compiler('c')

# Replacement for XORG_DEFAULT_OPTIONS
if cc.has_argument('-fno-strict-aliasing')
  add_project_arguments('-fno-strict-aliasing', language: 'c')
endif

# Checks for pkg-config packages
dep_libxrandr  = dependency('xrandr', required: true, version: '>= 1.5')
dep_libxrender = dependency('xrender', required: true)
dep_libx11     = dependency('x11', required: true)
dep_xproto     = dependency('xproto', required: true, version: '>= 7.0.17')
deps = [dep_libxrandr, dep_libxrender, dep_libx11, dep_xproto]

# Math libraries & functions
dep_libm = cc.find_library('m', required: false)
math_prefix = '#include <math.h>'

# - some compilers use builtin inlines for floor when optimizing
if not cc.has_function('floor', required: false, prefix: math_prefix)
  if cc.has_function('floor', dependencies: [dep_libm], prefix: math_prefix,
                     required: true)
    deps += [dep_libm]
  endif
endif

config_h = configure_file(output: 'config.h', configuration: conf)
add_project_arguments('-DHAVE_CONFIG_H', language: 'c')

executable(
  'xrandr',
  [config_h, 'xrandr.c'],
  dependencies: deps,
  install: true
)

custom_target(
  'xkeystone',
  input: ['nickle.sh', 'keystone.5c'],
  output: 'xkeystone',
  command: ['cat', '@INPUT@'],
  capture: true,
  install: true,
  install_dir: get_option('bindir'),
  install_mode: 'rwxr-xr-x',
)

# Man page
prog_sed = find_program('sed')

custom_target(
  'xrandr.man',
  input: 'man/xrandr.man',
  output: 'xrandr.1',
  command: [
    prog_sed,
    '-e', f's/__vendorversion__/"@package_string@" "X Version 11"/',
    '-e', 's/__appmansuffix__/1/g',
    '-e', 's/__libmansuffix__/3/g',
    '@INPUT@',
  ],
  capture: true,
  install: true,
  install_dir: get_option('mandir') / 'man1',
  install_tag: 'man',
)
