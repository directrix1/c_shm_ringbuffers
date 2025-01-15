#include "test_structs.h"
#include <shm_ringbuffers.h>
#include <stdio.h>
#include <string.h>

int main()
{
    printf("Subscriber side test\n");

    struct ShmRingBuffer* srb;
    SRBHandle h = srb_client_new("/srb_test1");
    if (h == NULL) {
        return 1;
    }
    int num_rings = srb_get_rings(h, &srb);
    if (num_rings != 1) {
        fprintf(stderr, "Returned too many rings: %u.\n", num_rings);
        return 2;
    } else {
        printf("Ring description (length: %ld): %s\n", strlen(srb->description), srb->description);
    }

    unsigned int cur_frame_id = 0;
    unsigned int next_frame_id = 0;
    struct test_struct1* cur;
    while (srb_subscriber_get_state(h) == SRB_RUNNING) {
        next_frame_id = srb_subscriber_get_most_recent_buffer_id(srb);
        if (cur_frame_id != next_frame_id) {
            cur_frame_id = next_frame_id;
            cur = (struct test_struct1*)srb_subscriber_get_most_recent_buffer(srb);
            printf("Received: %ld %s\n", cur->anum, cur->aword);
        }
    };

    srb_close(h);

    return 0;
}
