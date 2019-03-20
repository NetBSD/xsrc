project('xf86-video-intel', 'c',
	version : '2.99.917',
	default_options: [
	  'warning_level=2',
	  'c_std=gnu99',
	],
	license : 'MIT',
	meson_version : '>0.40.0')

config = configuration_data()

version = meson.project_version().split('.')
config.set('PACKAGE_VERSION_MAJOR', version[0])
config.set('PACKAGE_VERSION_MINOR', version[1])
config.set('PACKAGE_VERSION_PATCHLEVEL', version[2])

config.set_quoted('LIBEXEC_PATH', join_paths(get_option('prefix'),
					     get_option('libexecdir')))

cc = meson.get_compiler('c')

xorg = dependency('xorg-server', version : '>= 1.6', required : true)
pthreads = dependency('threads', required : true)
pciaccess = dependency('pciaccess', version : '>= 0.10', required : true)

x11 = dependency('x11', required : false)
xfixes = dependency('xfixes', required : false)
png = dependency('libpng', required : false)

if not cc.has_function('clock_gettime', args : '-lrt')
  error('clock_gettime() missing')
endif

if cc.has_function('getline')
  config.set('HAVE_GETLINE', 1)
endif

if cc.has_function('strndup')
  config.set('HAVE_STRNDUP', 1)
endif

if cc.has_function('strcasecmp')
  config.set('HAVE_STRCASECMP', 1)
endif

dependency('xproto', required : true)
dependency('fontsproto', required : true)
dependency('damageproto', required : true)

if cc.has_header_symbol('xorg-server.h', 'RANDR',
			dependencies : xorg)
  dependency('randrproto', required : true)
endif
if cc.has_header_symbol('xorg-server.h', 'RENDER',
			dependencies : xorg)
  dependency('renderproto', required : true)
endif
if cc.has_header_symbol('xorg-server.h', 'DPMSExtension',
			dependencies : xorg)
  dependency('xextproto', required : true)
endif

with_tools = get_option('tools')

config.set('USE_GIT_DESCRIBE', 1)
config.set('BUILDER_DESCRIPTION', 1)

atomic_primitives = 'none'

atomic_primitives_code = '''
int atomic_add(int i) {
    return __sync_fetch_and_add (&i, 1);
}
int atomic_cmpxchg(int i, int j, int k) {
    return __sync_val_compare_and_swap (&i, j, k);
}
int main(void) {
    return 0;
}'''
if cc.links(atomic_primitives_code, name : 'atomic primitives')
  atomic_primitives = 'intel'
  config.set('HAVE_ATOMIC_PRIMITIVES', 1)
endif

if atomic_primitives == 'none' and cc.has_header('atomic_ops.h')
  atomic_primitives = 'libatomic-ops'
  config.set('HAVE_LIB_ATOMIC_OPS', 1)
endif

if atomic_primitives == 'none'
  error('xf86-video-intel depends upon atomic operations, which were not found for your compiler/cpu. Try compiling with -march=native, or install the libatomics-op-dev package.')
endif

libudev = dependency('libudev', required : false)
if libudev.found()
  config.set('HAVE_UDEV', 1)
endif

cpuid_code = '''
#include <cpuid.h>
#include <stddef.h>
int main(void) {
    int eax, ebx, ecx, edx;
    if (__get_cpuid_max(0, NULL) < 4)
       return 0;
    __cpuid_count(4, 0, eax, ebx, ecx, edx);
    return 0;
}'''
if cc.links(cpuid_code, name : '__cpuid()')
  config.set('HAVE_CPUID_H', 1)
endif

has_shm = (cc.has_header('sys/ipc.h') and
	   cc.has_header('X11/extensions/XShm.h') and
	   cc.has_header('X11/extensions/shmproto.h') and
	   cc.has_header('X11/extensions/shmstr.h'))
if has_shm
  config.set('HAVE_MIT_SHM', 1)
  config.set('HAVE_X11_EXTENSIONS_SHMPROTO_H', 1)
  config.set('HAVE_X11_EXTENSIONS_SHMSTR_H', 1)
endif

if cc.has_header('X11/extensions/Xinerama.h')
  config.set('HAVE_X11_EXTENSIONS_XINERAMA_H', 1)
endif

if cc.has_header('X11/extensions/dpmsconst.h')
  config.set('HAVE_X11_EXTENSIONS_DPMSCONST_H', 1)
endif

pixman = dependency('pixman-1', version : '>= 0.16.0', required : true)

if pixman.version() >= '0.24.0'
  config.set('HAS_PIXMAN_TRIANGLES', 1)
endif
if pixman.version() >= '0.27.1'
  config.set('HAS_PIXMAN_GLYPHS', 1)
endif

with_kms = get_option('kms')
if with_kms
  config.set('KMS', 1)
endif

with_ums = get_option('ums')
if with_ums
  has_ums = cc.has_header('vgaHW.h',
			  dependencies : xorg)

  # Currently 'required' doesn't work for cc.has_header() & co.
  if not has_ums
    error('UMS dependencies not met')
  endif

  config.set('UMS', 1)
endif

with_xvmc = get_option('xvmc')
if with_xvmc
  dependency('xvmc', required : true)
  dependency('dri2proto', required : true)
  dependency('x11', required : true)
  dependency('x11-xcb', required : true)
  dependency('xcb-dri2', required : true)
  dependency('xcb-aux', required : true)
  dependency('libdrm_intel', required : true)

  config.set('ENABLE_XVMC', 1)
endif

with_valgrind = get_option('valgrind')
if with_valgrind
  message('Checking Valgrind support')
  valgrind = dependency('valgrind', required : true)
  config.set('HAVE_VALGRIND', 1)
endif

inc = include_directories([ '.', 'src', 'xvmc', 'src/render_program', ])

add_project_arguments('-include', 'config.h', language : 'c')

man_config = configuration_data()
man_config.set('appmansuffix', '1')
man_config.set('filemansuffix', '5')
man_config.set('drivermansuffix', '4')
man_config.set('miscmansuffix', '7')
man_config.set('xservername',
	       cc.get_define('__XSERVERNAME__',
			     prefix : '#include <xorg-server.h>',
			     dependencies : xorg))
man_config.set('xconfigfile',
	       cc.get_define('__XCONFIGFILE____',
			     prefix : '#include <xorg-server.h>',
			     dependencies : xorg))
man_config.set('vendorversion', '"@0@ @1@" "@2@"'.format(meson.project_name(),
							 meson.project_version(),
							 'X Version 11'))

subdir('src')
subdir('tools')

if with_xvmc
  subdir('xvmc')
endif

subdir('man')

configure_file(output: 'config.h', install: false, configuration: config)
