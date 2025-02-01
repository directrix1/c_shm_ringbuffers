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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

SRBHandle h = NULL;

void printUsage(char* progName)
{
    printf("Usage:\n %s SHMNAME\n\nShows info about SRBs at shared memory location SHMNAME.\n\n", progName);
}

int main(int argc, char** argv)
{
    char* shmName;

    struct ShmRingBuffer* srb;

    if (argc < 2) {
        printUsage(argv[0]);
        return 1;
    }

    shmName = argv[1];

    h = srb_client_new(shmName);
    if (h == NULL) {
        return 1;
    }
    printf("SRB buffers at \"%s\":\n", shmName);

    int numRings = srb_get_rings(h, &srb);
    while (numRings--) {
        printf("\t%s (%d bytes x %d buffers)\n", srb->description, srb->shared->buffer_size, srb->shared->num_buffers);
    }

    srb_close(h);

    return 0;
}
