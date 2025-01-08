VariantDir('build', 'src')

env = Environment(
    CCFLAGS='-Ibuild'
    )

libFiles = Glob('build/*.c')

lib = env.SharedLibrary('build/shm_ringbuffers', libFiles)

Default(lib)
