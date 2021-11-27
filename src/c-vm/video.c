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

#define VIDEO_BIT_BUFFER 1

typedef struct Video {
    struct mfb_window *window;
    bool enabled;
    VideoMemory *memory;
    word_t flags_cache;
    uint32_t *buffer;
    struct {
        time_t time;
        int frames;
        int buffer_switches;
    } stats;
} Video;

bool select_buffer(Video *this, word_t flags);

void print_fps(Video *this);

Video *video_create(bool enable) {
    Video *this = malloc(sizeof(Video));
    this->window = 0x0;
    this->enabled = enable;
    this->memory = malloc(sizeof(VideoMemory));
    this->memory->metadata = memory_create(VIDEO_META_SIZE, MEM_MODE_RW);
    this->flags_cache = 0;

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


    mfb_update_state state;
    this->stats.time = time(NULL);
    this->stats.frames = 0;
    this->stats.buffer_switches = 0;
    do {
        word_t flags;
        if (memory_word_get(this->memory->metadata, 0, &flags) != MEM_ERR_OK) {
            continue;
        }
        flags = from_big_endian(flags);
        select_buffer(this, flags);

        state = mfb_update_ex(this->window, this->buffer, VIDEO_SCREEN_WIDTH, VIDEO_SCREEN_HEIGHT);
        if (state != STATE_OK) {
            this->window = 0x0;
            break;
        }
        if (!this->enabled) {
            video_stop(this);
        }

        print_fps(this);
    } while (mfb_wait_sync(this->window));
    this->window = 0x0;
}

void print_fps(Video *this) {
    this->stats.frames = (this->stats.frames + 1) % VIDEO_SCREEN_FPS;
    if (this->stats.frames == 0) {
        time_t seconds = -(this->stats.time - time(&(this->stats.time)));
        printf("FPS: %2.1f ; %2.1f\n",
               (double) VIDEO_SCREEN_FPS / (double) seconds,
               (double) this->stats.buffer_switches / (double) seconds);
        this->stats.buffer_switches = 0;
    }
}

/**
 * updates this->buffer if needed and returns true if an update was needed.
 */

inline bool select_buffer(Video *this, word_t flags) {
    bool update_needed = (this->flags_cache ^ flags) & VIDEO_BIT_BUFFER;

    if (update_needed) {
        this->buffer = memory_raw_get(this->memory->buffer[flags & VIDEO_BIT_BUFFER]);
        this->flags_cache ^= VIDEO_BIT_BUFFER;
        this->stats.buffer_switches++;
    }

    return update_needed;
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
