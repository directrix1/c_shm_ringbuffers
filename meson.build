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

producer_test_exe = executable('host_test', 'tests/host_test.c',
   include_directories: include_directories('src'),
   link_with : shlib)
subscriber_test_exe = executable('client_test', 'tests/client_test.c',
   include_directories: include_directories('src'),
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
