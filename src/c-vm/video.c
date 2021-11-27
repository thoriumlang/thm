/**
 * Copyright 2019 Christophe Pollet
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <stdio.h>
#include <MiniFB.h>
#include <unistd.h>
#include <time.h>
#include "vmarch.h"
#include "video.h"
#include "memory.h"

typedef struct Video {
    struct mfb_window *window;
    bool enabled;
    VideoMemory *memory;
    uint32_t *buffer;
} Video;

Video *video_create(bool enable) {
    Video *this = malloc(sizeof(Video));
    this->window = 0x0;
    this->enabled = enable;

    this->memory = malloc(sizeof(VideoMemory));
    this->memory->metadata = memory_create(VIDEO_META_SIZE, MEM_MODE_RW);

    if (this->enabled) {
        this->memory->buffer[0] = memory_create(VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_HEIGHT * 4, MEM_MODE_RW);
        this->memory->buffer[1] = memory_create(VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_HEIGHT * 4, MEM_MODE_RW);
        this->buffer = memory_raw_get(this->memory->buffer[0]);
    } else {
        this->memory->buffer[0] = NULL;
        this->memory->buffer[1] = NULL;
        this->buffer = NULL;
    }
    return this;
}

VideoMemory *video_memory_get(Video *this) {
    return this->memory;
}

void video_loop(Video *this) {
    if (this->enabled && this->buffer) {
        this->window = mfb_open("thm", VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_SCALE,
                                VIDEO_SCREEN_HEIGHT * VIDEO_SCREEN_SCALE);
    }
    if (!this->window) {
        return;
    }
    mfb_set_viewport(this->window,
                     0, 0,
                     VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_SCALE,
                     VIDEO_SCREEN_HEIGHT * VIDEO_SCREEN_SCALE
    );
    mfb_set_target_fps(VIDEO_SCREEN_FPS);

    uint32_t i, noise, carry, seed = 0xbeef;
    mfb_update_state state;

    time_t start;
    int frames = 0;
    time(&start);

    do {
        for (i = 0; i < VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_HEIGHT; ++i) {
            noise = seed;
            noise >>= 3;
            noise ^= seed;
            carry = noise & 1;
            noise >>= 1;
            seed >>= 1;
            seed |= (carry << 30);
            noise &= 0xFF;
            this->buffer[i] = MFB_RGB(noise, noise, noise);
        }
        state = mfb_update_ex(this->window, this->buffer, VIDEO_SCREEN_WIDTH, VIDEO_SCREEN_HEIGHT);
        if (state != STATE_OK) {
            this->window = 0x0;
            break;
        }
        if (!this->enabled) {
            video_stop(this);
        }

        frames = (frames + 1) % VIDEO_SCREEN_FPS;
        if (frames == 0) {
            printf("FPS: %f\n", 1 / ((double) -(start - time(&start)) / VIDEO_SCREEN_FPS));
        }
    } while (mfb_wait_sync(this->window));
    this->window = 0x0;
}

void video_stop(Video *this) {
    this->enabled = false;
    mfb_close(this->window);
}

void video_destroy(Video *this) {
    video_stop(this);
    for (int i = 0; i < 100; i++) {
        if (this->window == 0x0) {
            break;
        }
        usleep(5000);
    }
    memory_destroy(this->memory->metadata);
    if (this->memory->buffer[0]) {
        memory_destroy(this->memory->buffer[0]);
    }
    if (this->memory->buffer[1]) {
        memory_destroy(this->memory->buffer[1]);
    }
    free(this->memory);
    free(this);
}
