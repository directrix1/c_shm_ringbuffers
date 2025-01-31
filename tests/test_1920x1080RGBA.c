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

#include <bits/time.h>
#include <math.h>
#include <shm_ringbuffers.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

SRBHandle h = NULL;

double get_cur_time(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec + (double)ts.tv_nsec / 1000000000.0);
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

struct RGBA {
    unsigned char r, g, b, a;
};

void make_frame(struct RGBA* pixels, double t)
{
    for (int y = 0; y < 1080; y++) {
        for (int x = 0; x < 1920; x++) {
            pixels->r = (unsigned char)(128.0 * (sin((x + y) * 0.01 + t) + 1.0));
            pixels->g = (unsigned char)(128.0 * (sin((x + y) * 0.02 + t) + 1.0));
            pixels->b = (unsigned char)(128.0 * (sin((x + y) * 0.04 + t) + 1.0));
            pixels->a = 255;
            pixels++;
        }
    }
}

#define FPS (10)

int main(__attribute__((unused)) int argc, __attribute__((unused)) char** argv)
{
    char* channelName = "video_frames";
    struct ShmRingBuffer* srb;

    // Host needs to be run before and while all clients are run.
    printf("Hosting Channel: %s\n", channelName);

    struct ShmRingBufferDef srbd = {
        .buffer_size = 1920 * 1080 * sizeof(struct RGBA), // Gonna be passing 1920x1080 RGBA data
        .num_buffers = 3, // Gonna make a ring buffer of size 3, this is the minimum size
        .description = channelName, // What to call this ringbuffer
    };

    h = srb_host_new("/srb_video_test", 1, &srbd);
    if (h == NULL) {
        return 1;
    }
    signal(SIGINT, hostCloseSRB);

    srb_get_rings(h, &srb); // We know there's only 1 so ignore return
    printf("Created ring description: %s\n", srb->description);

    double curTime = get_cur_time();
    double frameJump = 1.0 / FPS;
    struct RGBA* pixels = (struct RGBA*)srb_producer_first_write_buffer(srb);
    make_frame(pixels, curTime);
    useconds_t frame_time = 1000000 / FPS;
    curTime = get_cur_time();
    while (1) {
        pixels = (struct RGBA*)srb_producer_next_write_buffer(srb);
        useconds_t dt = (useconds_t)((get_cur_time() - curTime) * 1000000.0);
        if (dt < frame_time) {
            usleep(frame_time - dt);
            curTime += frameJump;
        } else {
            printf("Not hitting %d fps (%d > %d).\n", (FPS), dt, frame_time);
            curTime = get_cur_time();
        }
        make_frame(pixels, curTime);
    };

    return 0;
}
