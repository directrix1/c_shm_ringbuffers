c_shm_ringbuffers
=================

This library is intended to provide shared memory ring buffers which are used to efficiently convey state information from producer processes to subscriber processes in a relatively simple, efficient, and variably sized per ringbuffer way. There are 3 roles:

Host
----
The host process define the buffer parameters (name, size, and number of buffers in the ring) for each named ring (there can be numerous in one shared memory space). It then controls the lifecycle of the shared memory space, allocating it at the start and unlinking it at stop.

The host is also a client and can work in the following roles as well, if you so wish it to.

Clients
-------
The clients are connected to the buffers setup by the host, they have a couple sub-roles:

 * Producer - the process with information which subscribers wish to receive in an on-going basis. This information is periodically / continuously pushed to any of the named ring buffers. (i.e. frame information, sensor data, etc.)
 
 * Subscriber - a process interested in receiving the continuously updated information pushed by the producer over the ring buffers. The goal is not to receive all data, but to receive it on an ongoing regular basis, and depending on application requirements, to possibly consume the information at a rate as fast as the producer can make it.

Each named ring buffer can have any number of subscribers but currently only one producer per ring buffer.

License and Attributions
========================

See the files [LICENSE.txt](LICENSE.txt) and [AUTHORS.md](AUTHORS.md) for further information. The just of it is the initial version of this was written by Edward Andrew Flick and released under the MIT License in January 2025.
