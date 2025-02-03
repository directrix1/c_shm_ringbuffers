/******************************************************************************
 *
 * Copyright (c) 2025-present Edward Andrew Flick.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "test_structs.h"
#include <shm_ringbuffers.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

SRBHandle h = NULL;

void printUsage(char* progName)
{
    printf("Usage:\n %s (host|producer|subscriber) [OptionalChannelName]\n", progName);
}

void hostCloseSRB(int signum)
{
    printf("Signalling (%d) that host is shutting down...\n", signum);
    srb_host_signal_stopping(h);
    sleep(5);
    printf("Closing shared buffers.\n");
    srb_close(h);
    exit(0);
}

void closeSRB(int signum)
{
    printf("Closing shared buffers (%d).\n", signum);
    srb_close(h);
    exit(0);
}

int main(int argc, char** argv)
{
    char* channelName = "Test Channel 1";
    struct ShmRingBuffer* srb;
    char line[100] = "";

    if (argc == 3) {
        channelName = argv[2];
    } else if (argc != 2) {
        printUsage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "host") == 0) {
        // Host needs to be run before and while all clients are run.
        printf("Hosting Channel: %s\n", channelName);

        struct ShmRingBufferDef srbd = {
            .buffer_size = sizeof(struct test_struct1), // Gonna be passing test_struct1s
            .num_buffers = 3, // Gonna make a ring buffer of size 3, this is the minimum size
            .description = channelName, // What to call this ringbuffer
        };

        h = srb_host_new("/srb_test1", 1, &srbd);
        if (h == NULL) {
            return 1;
        }
        signal(SIGINT, hostCloseSRB);

        srb_get_rings(h, &srb); // We know there's only 1 so ignore return
        printf("Created ring description: %s\n", srb->description);
        while (1) {
            // This is also a client and can produce/subscribe to buffers here
            sleep(1);
        };

    } else if (strcmp(argv[1], "producer") == 0) {
        // Producer updates the buffers with its current state as it runs
        printf("Producing Channel: %s\n", channelName);

        h = srb_client_new("/srb_test1");
        if (h == NULL) {
            return 1;
        }
        signal(SIGINT, closeSRB);

        srb = srb_get_ring_by_description(h, channelName);
        if (srb) {
            printf("Found ring: %s\n", srb->description);
        } else {
            fprintf(stderr, "Could not find ring: %s\n", channelName);
            closeSRB(2);
        }

        int64_t all_words_size = 0;
        struct test_struct1* cur;
        do {
            cur = (struct test_struct1*)srb_producer_next_write_buffer(srb);
            printf("Enter some words (q to quit): ");
            fgets(line, 99, stdin);
            line[99] = 0;
            if ((strcmp(line, "q\n") == 0) || (srb_client_get_state(h) != SRB_RUNNING)) {
                continue;
            }

            all_words_size += strlen(line);
            printf("Sending: %ld %s\n", all_words_size, line);

            cur->anum = all_words_size;
            strcpy(cur->aword, line);

        } while ((strcmp(line, "q\n") != 0) && (srb_client_get_state(h) == SRB_RUNNING));
        closeSRB(0);

    } else if (strcmp(argv[1], "subscriber") == 0) {
        // Subscribers read the most recent buffers produced and try to catch up to its current state
        printf("Subscribing Channel: %s\n", channelName);

        h = srb_client_new("/srb_test1");
        if (h == NULL) {
            return 1;
        }
        signal(SIGINT, closeSRB);

        srb = srb_get_ring_by_description(h, channelName);
        if (srb) {
            printf("Found ring: %s\n", srb->description);
        } else {
            fprintf(stderr, "Could not find ring: %s\n", channelName);
            closeSRB(2);
        }

        struct test_struct1* cur;
        while (srb_client_get_state(h) == SRB_RUNNING) {
            if ((cur = (struct test_struct1*)srb_subscriber_get_next_unread_buffer(srb))) {
                printf("Received: %ld %s\n", cur->anum, cur->aword);
            } else {
                sleep(1);
            }
        };

    } else {
        printf("Invalid role supplied: %s\n", argv[1]);
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
