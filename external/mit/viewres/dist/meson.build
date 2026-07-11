# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025, Oracle and/or its affiliates.
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

project(
  'viewres',
  'c',
  version: '1.0.8',
  license: 'MIT-Open-Group',
  license_files: 'COPYING',
  meson_version: '>= 1.1.0',
)

cc = meson.get_compiler('c')

conf = configuration_data()
conf.set_quoted('PACKAGE_STRING',
                ' '.join(meson.project_name(), meson.project_version()))
config_h = configure_file(output: 'config.h', configuration: conf)
add_project_arguments('-DHAVE_CONFIG_H', language: ['c'])

# Replacement for XORG_DEFAULT_OPTIONS
if cc.has_argument('-fno-strict-aliasing')
  add_project_arguments('-fno-strict-aliasing', language: 'c')
endif

# Checks for pkg-config packages
dep_libxaw = dependency('xaw7', required: true)
dep_libxmu = dependency('xmu', required: true)
dep_libxt  = dependency('xt', required: true)
dep_xproto = dependency('xproto', required: true, version: '>= 7.0.22')

add_project_arguments('-D_CONST_X_STRING', language: ['c'])

executable(
  'viewres',
  [config_h, 'viewres.c'],
  dependencies: [dep_libxaw, dep_libxmu, dep_libxt, dep_xproto],
  install: true
)

# Find directory for installing app-defaults files
appdefaultdir = get_option('appdefaultdir')
if appdefaultdir == ''
  appdefaultdir = dep_libxt.get_variable('appdefaultdir')
endif
summary('appdefaultdir', appdefaultdir)

# App default files
appdefaults = [
  'app-defaults/Viewres',
  'app-defaults/Viewres-color'
]
install_data(appdefaults, install_dir: appdefaultdir)

prog_sed = find_program('sed')

custom_target(
  'viewres.man',
  input: 'man/viewres.man',
  output: 'viewres.1',
  command: [
    prog_sed,
    '-e', 's/__xorgversion__/"viewres @0@" "X Version 11"/'.format(meson.project_version()),
    '-e', 's/__appmansuffix__/1/g',
    '-e', 's/__miscmansuffix__/7/g',
    '@INPUT@',
  ],
  capture: true,
  install: true,
  install_dir: get_option('mandir') / 'man1',
)
