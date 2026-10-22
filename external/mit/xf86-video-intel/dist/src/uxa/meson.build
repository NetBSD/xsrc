config.set('USE_UXA', 1)

uxa_sources = [
  'intel_batchbuffer.c',
  'intel_display.c',
  'intel_driver.c',
  'intel_memory.c',
  'intel_uxa.c',
  'intel_video.c',
  'intel_video_overlay.c',
  'intel_uxa_video.c',
  'i830_3d.c',
  'i830_render.c',
  'i915_3d.c',
  'i915_render.c',
  'i915_video.c',
  'i965_3d.c',
  'i965_video.c',
  'i965_render.c',
  'uxa.c',
  'uxa-accel.c',
  'uxa-glyphs.c',
  'uxa-render.c',
  'uxa-unaccel.c',
]

uxa_deps = [
  dependency('xorg-server', version : '>= 1.6', required : true),
  dependency('pixman-1', version : '>= 0.24.0', required : true),
  dependency('libdrm', required : true),
  dependency('libdrm_intel', version : '>= 2.4.52', required : true),
  dependency('libudev', required : false),
]

if with_dri2
  uxa_sources += 'intel_dri.c'
  uxa_deps += dependency('dri2proto', version : '>= 2.6', required : true)
endif

if with_dri3
  uxa_sources += [
    'intel_dri3.c',
    'intel_sync.c',
  ]
endif

if has_present
  uxa_sources += 'intel_present.c'
endif

if with_xvmc
  uxa_sources += 'intel_hwmc.c'
endif

uxa = static_library('uxa',
		     sources : uxa_sources,
		     dependencies : uxa_deps,
		     include_directories : inc,
		     c_args : [
		       '-Wno-deprecated-declarations',
		       '-Wno-unused-parameter',
		       '-Wno-sign-compare',
		     ],
		     install : false)
