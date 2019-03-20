xvmc_sources = [
  'intel_xvmc.c',
  'intel_xvmc_dump.c',
  'i915_xvmc.c',
  'i965_xvmc.c',
  'xvmc_vld.c',
  'intel_batchbuffer.c',
]

shared_library('IntelXvMC',
	       soversion : '1',
	       version : '1.0.0',
	       sources : xvmc_sources,
	       dependencies : [
		 dependency('threads', required : true),
		 dependency('x11', required : true),
		 dependency('xvmc', required : true),
		 dependency('xorg-server', required : true),
		 dependency('x11-xcb', required : true),
		 dependency('xcb-aux', required : true),
		 dependency('xcb-dri2', required : true),
		 dependency('libdrm_intel', required : true),
	       ],
	       c_args : [
		 '-DFALSE=0', '-DTRUE=1',
		 '-Wno-unused-but-set-variable',
		 '-Wno-unused-parameter',
		 '-Wno-sign-compare',
		 '-Wno-pointer-sign',
	       ],
	       install : true)
