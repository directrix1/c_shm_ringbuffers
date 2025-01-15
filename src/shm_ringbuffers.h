#ifndef SHM_RINGBUFFERS_H
#define SHM_RINGBUFFERS_H

#include <stdint.h>

#if defined _WIN32 || defined __CYGWIN__
#ifdef BUILDING_SHM_RINGBUFFERS
#define SHM_RINGBUFFERS_PUBLIC __declspec(dllexport)
#else
#define SHM_RINGBUFFERS_PUBLIC __declspec(dllimport)
#endif
#else
#ifdef BUILDING_SHM_RINGBUFFERS
#define SHM_RINGBUFFERS_PUBLIC __attribute__((visibility("default")))
#else
#define SHM_RINGBUFFERS_PUBLIC
#endif
#endif

enum EShmRingBuffersState {
    SRB_STOPPED = 0,
    SRB_RUNNING = 1,
    SRB_STOPPING = 2,
};

struct ShmRingBufferDef {
    unsigned int buffer_size;
    unsigned int num_buffers;
    char* description;
};

struct ShmRingBufferShared {
    unsigned int buffer_size;
    unsigned int num_buffers;
    unsigned int write_ring_pos;
};

struct ShmRingBuffer {
    char* description;
    uint8_t* buffers;
    struct ShmRingBufferShared* shared;
};

struct ShmRingBuffersHead {
    enum EShmRingBuffersState state;
    unsigned int num_ringbuffers;
};

struct ShmRingBuffersLocal {
    struct ShmRingBuffersHead* ring_buffers_head;
    struct ShmRingBuffer* ringbuffers;
    int is_host;
    int shm_fd;
    uint8_t* mem_map;
    const char* shm_path;
    unsigned int shm_size;
};

typedef struct ShmRingBuffersLocal* SRBHandle;

// ====================
// Subscriber functions
// ====================

/*
 * srb_subscriber_get_most_recent_buffer_id
 *
 * params:
 *   ring_buffer - the ring buffer to get the most recent buffer id
 *
 * returns:
 *   the most recent buffer id, that is not currently set as the write_ring_pos, or 0 if no valid buffers exist
 */
unsigned int SHM_RINGBUFFERS_PUBLIC srb_subscriber_get_most_recent_buffer_id(struct ShmRingBuffer* ring_buffer);

/*
 * srb_subscriber_get_most_recent_buffer
 *
 * params:
 *   ring_buffer - the ring buffer to get the most recent buffer
 *
 * returns:
 *   the most recent buffer that is not currently set as the write_ring_pos, or NULL if no valid buffers exist
 */
uint8_t SHM_RINGBUFFERS_PUBLIC* srb_subscriber_get_most_recent_buffer(struct ShmRingBuffer* ring_buffer);

/*
 * srb_subscriber_get_state
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory
 *
 * returns:
 *   the run state of the producer
 */
enum EShmRingBuffersState SHM_RINGBUFFERS_PUBLIC srb_subscriber_get_state(SRBHandle ring_buffers_handle);

// ==================
// Producer functions
// ==================

/*
 * srb_producer_first_write_buffer
 *
 * params:
 *   ring_buffer - the ring buffer to get the first shared buffer from
 *
 * return:
 *   pointer to the first shared buffer
 */
uint8_t SHM_RINGBUFFERS_PUBLIC* srb_producer_first_write_buffer(struct ShmRingBuffer* ring_buffer);

/*
 * srb_producer_next_write_buffer
 *   this function returns the next shared write buffer.
 *
 * params:
 *   ring_buffer - the ring buffer to get the next shared buffer from
 *
 * return:
 *   pointer to the next shared buffer
 */
uint8_t SHM_RINGBUFFERS_PUBLIC* srb_producer_next_write_buffer(struct ShmRingBuffer* ring_buffer);

/*
 * srb_producer_signal_stopping
 *   sets shared state to SRB_STOPPING
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory'
 *
 */
void SHM_RINGBUFFERS_PUBLIC srb_producer_signal_stopping(SRBHandle ring_buffers_handle);

// =================================================
// Common functions to producer and subscriber sides
// =================================================

/*
 * srb_host_new
 *
 * params:
 *   shm_path - shared memory path
 *   num_defs - the number of ringbuffers you are defining
 *   ring_buffer_defs - array of {struct ShmRingBuffer}s which will be created in the mmap based on the supplied members:
 *        buffer_size, num_buffers, and description. (these can be freed if wanted after this call)
 *
 * returns:
 *   the SRBHandle that references the shared memory ring buffers. This is usually followed up with srb_get_rings call.
 */
SRBHandle SHM_RINGBUFFERS_PUBLIC srb_host_new(const char* shm_path, unsigned int num_defs, struct ShmRingBufferDef* ring_buffer_defs);

/*
 * srb_client_new
 *
 * params:
 *   shm_path - shared memory path
 *
 * returns:
 *   the SRBHandle that references the shared memory ring buffers. This is usually followed up with srb_get_rings call.
 */
SRBHandle SHM_RINGBUFFERS_PUBLIC srb_client_new(const char* shm_path);

/*
 * srb_get_rings
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory
 *   ring_buffers - will be set to the memory mapped ring_buffers
 *
 * returns:
 *   the number of ring_buffers
 */
unsigned int SHM_RINGBUFFERS_PUBLIC srb_get_rings(SRBHandle ring_buffers_handle, struct ShmRingBuffer** ring_buffers);

/*
 * srb_close
 *   unmaps all ring buffers and closes the shared memory, if producer first signals SRB_STOPPED
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory
 */
void SHM_RINGBUFFERS_PUBLIC srb_close(SRBHandle ring_buffers_handle);

#endif
