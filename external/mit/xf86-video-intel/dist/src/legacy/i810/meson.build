i810_sources = [
  'i810_accel.c',
  'i810_cursor.c',
  'i810_driver.c',
  'i810_memory.c',
  'i810_video.c',
  'i810_wmark.c',
]

xorg = dependency('xorg-server', required : true)

i810_deps = [
  dependency('libdrm', required : true),
  dependency('pciaccess', required : true),
  xorg,
]

if cc.has_header('xaa.h', dependencies : xorg)
  config.set('HAVE_XAA_H', 1)
  i810_sources += 'i810_xaa.c'
endif

if cc.has_header('dgaproc.h', dependencies : xorg)
  config.set('HAVE_DGAPROC_H', 1)
  i810_sources += 'i810_dga.c'
endif

if with_dri1
  i810_sources += 'i810_dri.c'
  i810_deps += dependency('xf86driproto', required : true)

  if with_xvmc
    i810_sources += 'i810_hwmc.c'
  endif
endif

i810 = static_library('legacy_i810',
		      sources : i810_sources,
		      dependencies : i810_deps,
		      include_directories : inc,
		      c_args : [
			'-Wno-unused-parameter',
			'-Wno-sign-compare',
		      ],
		      install : false)

if with_xvmc
  subdir('xvmc')
endif
