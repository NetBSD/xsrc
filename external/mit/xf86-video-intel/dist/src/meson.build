xorg = dependency('xorg-server', required : true)
libdrm = dependency('libdrm', required : true)
pixman = dependency('pixman-1', required : true)

with_dri1 = get_option('dri1')
if with_dri1
  dri1 = dependency('xf86driproto', required : true)
  has_dri1 = (cc.has_header('dri.h', dependencies : xorg) and
	      cc.has_header('sarea.h', dependencies : xorg) and
	      cc.has_header('dristruct.h', dependencies : xorg))

  # Currently 'required' doesn't work for cc.has_header() & co.
  if not has_dri1
    error('DRI1 dependencies not met')
  endif

  config.set('HAVE_DRI1', 1)
endif

with_dri2 = get_option('dri2')
if with_dri2
  dri2 = dependency('dri2proto', version : '>= 2.6', required : true)

  dri = dependency('dri', required : false)
  if dri.found()
    dridriverdir = dri.get_pkgconfig_variable('dridriverdir')
  else
    dridriverdir = join_paths(get_option('libdir'), 'dri')
  endif

  config.set('HAVE_DRI2', 1)
  config.set_quoted('DRI_DRIVER_PATH', dridriverdir)
endif

with_dri3 = get_option('dri3')
if with_dri3
  dri3 = dependency('dri3proto', required : true)
  has_dri3 = (cc.has_header_symbol('xorg-server.h', 'DRI3',
				   dependencies : xorg) and
	      cc.has_header('misyncstr.h',
			    dependencies : xorg) and
	      cc.has_header('misyncshm.h',
			    dependencies : xorg))

  # Currently 'required' doesn't work for cc.has_header() & co.
  if not has_dri3
    error('DRI3 dependencies not met')
  endif

  config.set('HAVE_DRI3', 1)
endif

default_dri = get_option('default-dri')
config.set('DEFAULT_DRI_LEVEL', default_dri)

present = dependency('presentproto', required : false)
has_present = (present.found() and
	       cc.has_header('present.h', dependencies : xorg))
if has_present
  config.set('HAVE_PRESENT', 1)
endif

if get_option('backlight')
  config.set('USE_BACKLIGHT', 1)
endif
with_backlight_helper = get_option('backlight-helper')
if with_backlight_helper
  config.set('USE_BACKLIGHT_HELPER', 1)
endif

debug = get_option('internal-debug')
if debug == 'sync'
  config.set('DEBUG_SYNC', 1)
endif
if debug == 'memory' or debug == 'full'
  config.set('DEBUG_MEMORY', 1)
endif
if debug == 'pixmap' or debug == 'full'
  config.set('DEBUG_PIXMAP', 1)
endif
if debug == 'full'
  config.set('HAS_DEBUG_FULL', 1)
endif

intel_drv_sources = [
  'backlight.c',
  'fd.c',
  'intel_device.c',
  'intel_options.c',
  'intel_module.c',
]

intel_drv_deps = [
  dependency('pciaccess', version : '>= 0.10', required : true),
  libdrm,
  xorg,
]

intel_drv_libs = []

if with_ums
  subdir('legacy/i810')
  intel_drv_libs += i810
endif

default_accel = get_option('default-accel')

with_sna = get_option('sna')
if with_sna
  subdir('sna')
  intel_drv_libs += sna
elif default_accel == 'sna'
  error('SNA not available, so can\'t selected as the default acceleration method')
endif

with_uxa = get_option('uxa')
if with_uxa
  subdir('uxa')
  intel_drv_libs += uxa
elif default_accel == 'uxa'
  error('UXA not available, so can\'t selected as the default acceleration method')
endif

if default_accel == 'sna'
  config.set('DEFAULT_ACCEL_METHOD', 'SNA')
elif default_accel == 'uxa'
  config.set('DEFAULT_ACCEL_METHOD', 'UXA')
else
  config.set('DEFAULT_ACCEL_METHOD', 'NOACCEL')
endif

if with_valgrind
  intel_drv_deps += valgrind
endif

xorg_moduledir = get_option('xorg-module-dir')
moduledir = ''
foreach dir : xorg_moduledir.split('/')
  if dir == '@libdir@'
    dir = get_option('libdir')
  endif
  moduledir = join_paths(moduledir, dir)
endforeach

shared_module('intel_drv',
	      sources : intel_drv_sources,
	      dependencies : intel_drv_deps,
	      link_with : intel_drv_libs,
	      c_args : [
		'-DMAJOR_IN_SYSMACROS',
		'-Wno-unused-parameter',
		'-Wno-sign-compare',
	      ],
	      name_prefix : '',
	      install_dir : join_paths(moduledir, 'drivers'),
	      install : true)
