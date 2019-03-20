brw_deps = [
  xorg,
  libudev,
  libdrm,
]

if with_valgrind
  brw_deps += valgrind
endif

brw = static_library('brw',
		     sources : [
		       'brw_disasm.c',
		       'brw_eu.c',
		       'brw_eu_emit.c',
		       'brw_sf.c',
		       'brw_wm.c',
		     ],
		     dependencies : brw_deps,
		     include_directories : inc,
		     c_args : [
		       '-Wno-unused-parameter',
		       '-Wno-sign-compare',
		     ],
		     install : false)

executable('brw_test',
	   sources : [
	     'brw_test.c',
	     'brw_test_gen4.c',
	     'brw_test_gen5.c',
	     'brw_test_gen6.c',
	     'brw_test_gen7.c',
	   ],
	   link_with : brw,
	   include_directories : inc,
	   c_args : [
	     '-Wno-unused-const-variable',
	     '-Wno-unused-parameter',
	   ],
	   install : false)
