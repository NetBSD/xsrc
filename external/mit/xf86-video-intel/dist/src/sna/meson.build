config.set('USE_SNA', 1)

if cc.has_member('struct sysinfo', 'totalram',
		 prefix : '#include <sys/sysinfo.h>')
  config.set('HAVE_STRUCT_SYSINFO_TOTALRAM', 1)
endif

git_version_h = vcs_tag(input : 'git_version.h.in', output : 'git_version.h',
			fallback : 'not compiled from git',
			command : [ 'git', 'describe' ] )

config.set('HAVE_DOT_GIT', 1)

if cc.has_header('alloca.h')
  config.set('HAVE_ALLOCA_H', 1)
endif

sna_deps = [
  cc.find_library('m', required : true),
  dependency('threads', required : true),
  dependency('xorg-server', required : true),
  dependency('libdrm', required : true),
]

sna_sources = [
  'blt.c',
  'kgem.c',
  'sna_accel.c',
  'sna_acpi.c',
  'sna_blt.c',
  'sna_composite.c',
  'sna_cpu.c',
  'sna_damage.c',
  'sna_display.c',
  'sna_display_fake.c',
  'sna_driver.c',
  'sna_glyphs.c',
  'sna_gradient.c',
  'sna_io.c',
  'sna_render.c',
  'sna_stream.c',
  'sna_trapezoids.c',
  'sna_trapezoids_boxes.c',
  'sna_trapezoids_imprecise.c',
  'sna_trapezoids_mono.c',
  'sna_trapezoids_precise.c',
  'sna_tiling.c',
  'sna_transform.c',
  'sna_threads.c',
  'sna_vertex.c',
  'sna_video.c',
  'sna_video_overlay.c',
  'sna_video_sprite.c',
  'sna_video_textured.c',
  'gen2_render.c',
  'gen3_render.c',
  'gen4_common.c',
  'gen4_render.c',
  'gen4_source.c',
  'gen4_vertex.c',
  'gen5_render.c',
  'gen6_common.c',
  'gen6_render.c',
  'gen7_render.c',
  'gen8_eu.c',
  'gen8_render.c',
  'gen8_vertex.c',
  'gen9_render.c',
]

if libudev.found()
  sna_deps += libudev
endif

if with_valgrind
  sna_deps += valgrind
endif

if with_dri2
  sna_sources += 'sna_dri2.c'
  sna_deps += [
    dependency('dri2proto', required : true),
    cc.find_library('rt', required : true),
  ]
endif

if with_dri3
  sna_sources += 'sna_dri3.c'
  sna_deps += dri3
endif

if has_present
  sna_sources += 'sna_present.c'
  sna_deps += present
endif

if with_xvmc
  sna_sources += 'sna_video_hwmc.c'
endif

if debug == 'full'
  sna_sources += [
    'kgem_debug.c',
    'kgem_debug_gen2.c',
    'kgem_debug_gen3.c',
    'kgem_debug_gen4.c',
    'kgem_debug_gen5.c',
    'kgem_debug_gen6.c',
    'kgem_debug_gen7.c',
  ]
endif

if get_option('tearfree')
  config.set('TEARFREE', 1)
endif
if get_option('use-create2')
  config.set('USE_CREATE2', 1)
endif
if get_option('async-swap')
  config.set('USE_ASYNC_SWAP', 1)
endif

subdir('brw')
subdir('fb')

sna = static_library('sna',
		     [ git_version_h, sna_sources ],
		     dependencies : sna_deps,
		     link_with : [ brw, fb, ],
		     include_directories : inc,
		     c_args : [
		       '-Wno-unused-but-set-variable',
		       '-Wno-expansion-to-defined',
		       '-Wno-maybe-uninitialized',
		       '-Wno-unused-parameter',
		       '-Wno-unused-function',
		       '-Wno-unused-variable',
		       '-Wno-sign-compare',
		       '-Wno-type-limits',
		       '-Wno-clobbered',
		     ],
		     install : false)
