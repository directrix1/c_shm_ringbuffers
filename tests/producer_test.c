#include "test_structs.h"
#include <shm_ringbuffers.h>
#include <stdio.h>
#include <string.h>

int main()
{
    char line[100] = "";

    printf("Producer side test\n");

    struct ShmRingBufferDef srbd = {
        .buffer_size = sizeof(struct test_struct1), // Gonna be passing test_struct1s
        .num_buffers = 3, // Gonna make a ring buffer of size 3
        .description = "Test 1", // What to call this ringbuffer
    };

    struct ShmRingBuffer* srb;
    SRBHandle h = srb_producer_new("/srb_test1", 1, &srbd);
    srb_get_rings(h, &srb); // We know there's only 1 so ignore return
    printf("Created ring description: %s\n", srb->description);

    int64_t all_words_size = 0;
    struct test_struct1* cur = (struct test_struct1*)srb_producer_first_write_buffer(srb);
    do {
        printf("Enter some words (q to quit): ");
        fgets(line, 99, stdin);
        if (strcmp(line, "q\n") == 0) {
            continue;
        }

        all_words_size += strlen(line);
        printf("Sending: %ld %s\n", all_words_size, line);

        cur->anum = all_words_size;
        strcpy(cur->aword, line);

        cur = (struct test_struct1*)srb_producer_next_write_buffer(srb);

    } while (strcmp(line, "q\n") != 0);

    srb_close(h);

    return 0;
}
