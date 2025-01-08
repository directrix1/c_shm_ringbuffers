env = Environment(CCFLAGS='-Isrc')

libFiles = Glob('src/*.c')

lib = env.SharedLibrary('shm_ringbuffers', libFiles)

Default(lib)
