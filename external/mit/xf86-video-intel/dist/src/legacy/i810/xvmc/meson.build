shared_library('I810XvMC',
	       soversion : '1',
	       version : '1.0.0',
	       sources : 'I810XvMC.c',
	       dependencies : [
		 dependency('x11', required : true),
		 dependency('xvmc', required : true),
		 dependency('xorg-server', required : true),
		 dependency('libdrm', required : true),
	       ],
	       c_args : [
		 '-Wno-unused-parameter',
		 '-Wno-sign-compare',
	       ],
	       install : true)
