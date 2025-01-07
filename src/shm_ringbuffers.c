#include "shm_ringbuffers.h"
#include <fcntl.h> /* For O_* constants */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <unistd.h>

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
SRBHandle srb_subscriber_new(const char* shm_path)
{
    return NULL;
}

/*
 * srb_subscriber_get_most_recent_buffer
 *
 * params:
 *   ring_buffer - the ring buffer to get the most recent buffer
 *
 * returns:
 *   the most recent buffer that is not currently set as the write_ring_pos, or NULL if no valid buffers exist
 */
void* srb_subscriber_get_most_recent_buffer(struct ShmRingBuffer* ring_buffer)
{
    return NULL;
}

/*
 * srb_subscriber_get_state
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory
 *
 * returns:
 *   the run state of the producer
 */
enum EShmRingBuffersState srb_subscriber_get_state(SRBHandle ring_buffers_handle)
{
    return ring_buffers_handle->ring_buffers_head->state;
}

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
SRBHandle srb_producer_new(const char* shm_path, uint64_t num_defs, struct ShmRingBuffer* ring_buffer_defs)
{
    int head_size = sizeof(struct ShmRingBuffersHead);
    int rb_size = sizeof(struct ShmRingBuffer);
    int descriptions_offset = head_size + rb_size * num_defs;
    int descriptions_size = 0;
    int buffers_size = 0;
    for (int i = 0; i < num_defs; i++) {
        // TODO: handle null description
        descriptions_size += strlen(ring_buffer_defs[i].description) + 1;
        buffers_size += ring_buffer_defs[i].num_buffers * ring_buffer_defs[i].buffer_size;
    }
    int buffers_offset = descriptions_offset + descriptions_size;
    buffers_offset /= 4096;
    buffers_offset = 4096 * (1 + buffers_offset); // 4k align buffers area past end of header info
    int total_size = buffers_offset + buffers_size;

    int shmfd = shm_open(shm_path, O_CREAT | O_RDWR, S_IRWXU);
    if (shmfd <= 0) {
        fprintf(stderr, "Error creating shm object (%s): %d\n", shm_path, shmfd);
        return NULL;
    }
    if (ftruncate(shmfd, total_size) < 0) {
        fprintf(stderr, "Error truncating shm object (%s) at size: %d\n", shm_path, total_size);
    }
    uint8_t* m = (uint8_t*)mmap(NULL, total_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);

    // Create the memory mapped structure
    SRBHandle handle = malloc(sizeof(struct ShmRingBuffersLocal));
    handle->is_producer = 1;
    handle->shm_fd = shmfd;
    struct ShmRingBuffersHead* head = handle->ring_buffers_head = (struct ShmRingBuffersHead*)m;
    head->state = SRB_STOPPED;
    head->num_ringbuffers = num_defs;
    head->ringbuffers = (struct ShmRingBuffer*)(m + head_size);
    char* description = (char*)(m + descriptions_offset);
    uint8_t* buffer = m + buffers_offset;
    for (int i = 0; i < num_defs; i++) {
        struct ShmRingBuffer* dest = head->ringbuffers + i;
        struct ShmRingBuffer* src = ring_buffer_defs + i;
        dest->num_buffers = src->num_buffers;
        dest->buffer_size = src->buffer_size;
        dest->description = description;
        // TODO: handle null description
        strcpy(src->description, description);
        dest->buffers = (void*)buffer;

        if (i < (num_defs - 1)) {
            // TODO: handle null description
            description += strlen(description) + 1;
            buffer += dest->num_buffers * dest->buffer_size;
        }
    }

    return handle;
}

/*
 * srb_producer_first_write_buffer
 *
 * params:
 *   ring_buffer - the ring buffer to get the first shared buffer from
 *
 * return:
 *   pointer to the first shared buffer
 */
void* srb_producer_first_write_buffer(struct ShmRingBuffer* ring_buffer)
{
    return NULL;
}

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
void* srb_producer_next_write_buffer(struct ShmRingBuffer* ring_buffer)
{
    return NULL;
}

/*
 * srb_producer_signal_running
 *   sets shared state to SRB_RUNNING
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory'
 *
 */
void srb_producer_signal_running(SRBHandle ring_buffers_handle)
{
}

/*
 * srb_producer_signal_stopping
 *   sets shared state to SRB_STOPPING
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory'
 *
 */
void srb_producer_signal_stopping(SRBHandle ring_buffers_handle)
{
}

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
uint64_t srb_get_rings(SRBHandle ring_buffers_handle, struct ShmRingBuffer** ring_buffers)
{
    return 0;
}

/*
 * srb_close
 *   unmaps all ring buffers and closes the shared memory, if producer first signals SRB_STOPPED
 *
 * params:
 *   ring_buffers_handle - the handle to the ring buffer's shared memory
 */
void srb_close(SRBHandle ring_buffers_handle)
{
}
