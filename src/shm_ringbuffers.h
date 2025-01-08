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

struct ShmRingBuffer {
    // Only need to def these members when becoming producer, subscriber fills all fields.
    uint64_t buffer_size;
    uint64_t num_buffers;
    char* description;

    // These are always filled in by the 'new' functions.
    uint64_t read_ring_pos;
    void* buffers;
};

struct ShmRingBuffersHead {
    enum EShmRingBuffersState state;
    uint64_t num_ringbuffers;
    struct ShmRingBuffer* ringbuffers;
};

struct ShmRingBuffersLocal {
    struct ShmRingBuffersHead* ring_buffers_head;
    int is_producer;
    int shm_fd;
};

typedef struct ShmRingBuffersLocal* SRBHandle;

// ====================
// Subscriber functions
// ====================

/*
 * srb_subscriber_new
 *
 * params:
 *   shm_path - shared memory path
 *
 * returns:
 *   the SRBHandle that references the shared memory ring buffers. This is usually followed up with srb_get_rings call.
 */
SRBHandle SHM_RINGBUFFERS_PUBLIC srb_subscriber_new(const char* shm_path);

/*
 * srb_subscriber_get_most_recent_buffer
 *
 * params:
 *   ring_buffer - the ring buffer to get the most recent buffer
 *
 * returns:
 *   the most recent buffer that is not currently set as the write_ring_pos, or NULL if no valid buffers exist
 */
void* SHM_RINGBUFFERS_PUBLIC srb_subscriber_get_most_recent_buffer(struct ShmRingBuffer* ring_buffer);

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
 * srb_producer_new
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
SRBHandle SHM_RINGBUFFERS_PUBLIC srb_producer_new(const char* shm_path, uint64_t num_defs, struct ShmRingBuffer* ring_buffer_defs);

/*
 * srb_producer_first_write_buffer
 *
 * params:
 *   ring_buffer - the ring buffer to get the first shared buffer from
 *
 * return:
 *   pointer to the first shared buffer
 */
void* SHM_RINGBUFFERS_PUBLIC srb_producer_first_write_buffer(struct ShmRingBuffer* ring_buffer);

/*
 * srb_producer_next_write_buffer
 *   this function returns the next shared write buffer and also calls srb_signal_running.
 *
 * params:
 *   ring_buffer - the ring buffer to get the next shared buffer from
 *
 * return:
 *   pointer to the next shared buffer
 */
void* SHM_RINGBUFFERS_PUBLIC srb_producer_next_write_buffer(struct ShmRingBuffer* ring_buffer);

/*
 * srb_producer_signal_running
 *   sets shared state to SRB_RUNNING
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory'
 *
 */
void SHM_RINGBUFFERS_PUBLIC srb_producer_signal_running(SRBHandle ring_buffers_handle);

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
 * srb_get_rings
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory
 *   ring_buffers - will be set to the memory mapped ring_buffers
 *
 * returns:
 *   the number of ring_buffers
 */
uint64_t SHM_RINGBUFFERS_PUBLIC srb_get_rings(SRBHandle ring_buffers_handle, struct ShmRingBuffer** ring_buffers);

/*
 * srb_close
 *   unmaps all ring buffers and closes the shared memory, if producer first signals SRB_STOPPED
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory
 */
void SHM_RINGBUFFERS_PUBLIC srb_close(SRBHandle ring_buffers_handle);

#endif
