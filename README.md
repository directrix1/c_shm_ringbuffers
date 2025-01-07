c_shm_ringbuffers
=================

This library is intended to provide shared memory ring buffers which convey information one way, from producer to subscriber. This is done with minimal locking as subscribers only read the most recent memory mapped buffer which has been fully filled.
