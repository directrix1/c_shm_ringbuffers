c_shm_ringbuffers
=================

This library is intended to provide shared memory ring buffers which are used to efficiently convey state information via rings of buffers (which have per ring fixed buffer size and count) from producer processes to subscriber processes in a relatively simple and efficient way. Think of it like a local high performance, simple to use, interprocess FIFO, with the downside being a subscriber may potentially drop some buffers if it can't keep up.

This software is currently only compatible with systems that have a **POSIX interface**. So pretty much **anything but Windows** (aka **Linux, Unix, BSD, MacOS, etc.**)  ***Patches to enable Windows support are welcome.***

There are 3 roles that software using this library may take on:

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

Building
========

This software currently requires a POSIX based shared memory implementation to function (so probably not Windows at the moment?). It is written in C and uses [Meson Build system](https://mesonbuild.com/).

To build run (from within the source tree):

   meson setup builddir
   cd builddir
   ninja
   ninja install

Utilities
=========

There are 2 simple (but hopefully useful) utility programs included with this library. Just run either one without parameters for help with using them.

srbhost
-------

Hosts at the shared memory location you specify, as many rings of any variety that you specify on the commandline. This allows the ring buffers to stay online and accessible, regardless of if the producer or subscriber are connected.

srbinfo
-------

This describes all the ring buffers at the commandline-specified shared memory location.

License and Attributions
========================

See the files [LICENSE.txt](LICENSE.txt) and [AUTHORS.md](AUTHORS.md) for further information. The just of it is the initial version of this was written by Edward Andrew Flick and released under the MIT License in January 2025.
