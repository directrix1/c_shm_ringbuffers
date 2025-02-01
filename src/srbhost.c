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

#include <shm_ringbuffers.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

SRBHandle h = NULL;

void printUsage(char* progName)
{
    printf("Usage:\n %s SHMNAME (RINGNAME BUFFERSIZE NUMBUFFERS)+\n\nAttaches to shared memory SHMNAME, and creates a ring for each RINGNAME BUFFERSIZE and NUMBUFFERS set provided. example:\n\n %s /srb_video_test video_frames 8294400 10\n\n ... will attach to /srb_video_test and create one ring named video_frames with 10 buffers of size 8294400 bytes.\n", progName, progName);
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

int main(int argc, char** argv)
{
    char* shmName;

    if (argc < 5) {
        printUsage(argv[0]);
        return 1;
    }

    shmName = argv[1];

    if ((argc - 2) % 3) {
        // Wrong number of args supplied
        printUsage(argv[0]);
        return 1;
    }

    int numChannels = (argc - 2) / 3;
    struct ShmRingBufferDef* srbd = malloc(sizeof(struct ShmRingBuffer) * numChannels);

    char** rings = argv + 2;
    for (int channelNum = 0; channelNum < numChannels; channelNum++) {
        char* channelName = *(rings++);
        int bufferSize = atoi(*(rings++));
        int numBuffers = atoi(*(rings++));

        if ((bufferSize < 1) || (numBuffers < 3)) {
            free(srbd);
            printUsage(argv[0]);
            return 2;
        }

        srbd[channelNum].buffer_size = bufferSize;
        srbd[channelNum].num_buffers = numBuffers;
        srbd[channelNum].description = channelName;
    }

    h = srb_host_new(shmName, numChannels, srbd);
    if (h == NULL) {
        return 3;
    }
    signal(SIGINT, hostCloseSRB);

    // Host needs to be run before and while all clients are run.
    printf("Hosting (at \"%s\") buffers:\n", shmName);
    for (int channelNum = 0; channelNum < numChannels; channelNum++) {
        printf("\t%s (%d bytes x %d buffers)\n", srbd[channelNum].description, srbd[channelNum].buffer_size, srbd[channelNum].num_buffers);
    }
    printf("\nPress Ctrl+C to stop.\n");

    while (1) {
        sleep(1);
    };

    return 0;
}
