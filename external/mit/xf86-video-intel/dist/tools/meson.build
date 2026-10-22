if with_tools
  executable('intel-virtual-output',
	     sources : 'virtual.c',
	     dependencies : [
	       dependency('x11', required : true),
	       dependency('xext', required : true),
	       dependency('xfixes', required : true),
	       dependency('xrender', required : true),
	       dependency('xdamage', required : true),
	       dependency('xrandr', required : true),
	       dependency('xrender', required : true),
	       dependency('xcursor', required : true),
	       dependency('xscrnsaver', required : true),
	       dependency('xinerama', required : true),
	       dependency('xtst', required : true),
	       dependency('pixman-1', required : true),
	     ],
	     c_args : [
	       '-Wno-unused-parameter',
	     ],
	     install : true)

  configure_file(input : 'intel-virtual-output.man',
		 output : 'intel-virtual-output.4',
		 command : [
		 'sed',
		 '-e',
		 's/__appmansuffix__/@0@/g'.format(man_config.get('appmansuffix')),
		 '-e',
		 's/__filemansuffix__/@0@/g'.format(man_config.get('filemansuffix')),
		 '-e',
		 's/__drivermansuffix__/@0@/g'.format(man_config.get('drivermansuffix')),
		 '-e',
		 's/__miscmansuffix__/@0@/g'.format(man_config.get('miscmansuffix')),
		 '-e',
		 's/__xservername__/@0@/g'.format(man_config.get('xservername')),
		 '-e',
		 's/__xconfigfile__/@0@/g'.format(man_config.get('xconfigfile')),
		 '-e',
		 's/__vendorversion__/@0@/g'.format(man_config.get('vendorversion')),
		 '@INPUT@'
	       ],
	       capture : true,
	       install_dir: join_paths(get_option('mandir'), 'man4'),
	       install : true)

  executable('cursor',
	     sources : 'cursor.c',
	     dependencies : [
	       dependency('x11', required : true),
	       dependency('xfixes', required : true),
	       dependency('libpng', required : true),
	     ],
	     c_args : [
	       '-Wno-unused-parameter',
	     ],
	     install : false)
endif

if with_tools and with_dri3
  executable('dri3info',
	     sources : 'dri3info.c',
	     dependencies : [
	       dependency('x11-xcb', required : true),
	       dependency('xcb-dri3', required : true),
	       dependency('x11', required : true),
	       dependency('xrandr', required : true),
	       dependency('xxf86vm', required : true),
	       dependency('dri3proto', required : true),
	       dependency('dri', required : true),
	       dependency('libdrm', required : true),
	     ],
	     install : false)
endif

if with_backlight_helper
  executable('xf86-video-intel-backlight-helper',
	     sources : 'backlight_helper.c',
	     install_dir : get_option('libexecdir'),
	     install_mode : [ 'rws--x--x', 'root', 'root' ],
	     c_args : [
	       '-DMAJOR_IN_SYSMACROS',
	     ],
	     install : true)

  polkit_config = configuration_data()
  polkit_config.set('LIBEXEC_PATH',
		    join_paths(get_option('prefix'),
			       get_option('libexecdir')))

  configure_file(input : 'org.x.xf86-video-intel.backlight-helper.policy.in',
		 output : 'org.x.xf86-video-intel.backlight-helper.policy',
		 configuration : polkit_config,
		 install_dir : join_paths(get_option('datadir'), 'polkit-1/actions'),
		 install : true)
endif
