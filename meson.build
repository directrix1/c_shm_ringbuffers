project('shm_ringbuffers', 'c',
  version : '1.0',
  default_options : ['warning_level=3'])

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_SHM_RINGBUFFERS','-Isrc']

shlib = shared_library('shm_ringbuffers', 'src/shm_ringbuffers.c',
  install : true,
  c_args : lib_args,
  gnu_symbol_visibility : 'hidden',
)

srbhost_exe = executable('srbhost', 'src/srbhost.c',
   install : true,
   include_directories: include_directories('src'),
   link_with : shlib)

srbinfo_exe = executable('srbinfo', 'src/srbinfo.c',
   install : true,
   include_directories: include_directories('src'),
   link_with : shlib)

test_exe = executable('test1', 'tests/test1.c',
   include_directories: include_directories('src'),
   link_with : shlib)
test_1920x1080RGBA_exe = executable('test_1920x1080RGBA', 'tests/test_1920x1080RGBA.c',
   include_directories: include_directories('src'),
   link_args : ['-lm'],
   link_with : shlib)
# test('shm_ringbuffers', test_exe)

# Make this library usable as a Meson subproject.
shm_ringbuffers_dep = declare_dependency(
  include_directories: include_directories('.'),
  link_with : shlib)

# Make this library usable from the system's
# package manager.
install_headers('src/shm_ringbuffers.h', subdir : '.')

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'shm_ringbuffers',
  filebase : 'shm_ringbuffers',
  description : 'Meson sample project.',
  subdirs : 'shm_ringbuffers',
  libraries : shlib,
  version : '1.0',
)
