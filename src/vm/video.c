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
#include "time.h"
#include "vmarch.h"
#include "video.h"
#include "memory.h"

#define VIDEO_BIT_BUFFER 1
#define VIDEO_BIT_ENABLED 2
#define VIDEO_BIT_SYNC 3

typedef struct Video {
    PIC *pic;
    struct mfb_window *window;
    bool enabled;
    VideoMemory *memory;
    word_t flags_cache;
    uint32_t *buffer;
    struct {
        utime_t utime;
        int frames;
        int buffer_switches;
        double fps;
    } stats;
} Video;

bool select_buffer(Video *this, word_t flags);

void print_fps(Video *this);

Video *video_create(PIC *pic, bool enable) {
    Video *this = malloc(sizeof(Video));
    this->pic = pic;
    this->window = 0x0;
    this->enabled = enable;
    this->memory = malloc(sizeof(VideoMemory));
    this->memory->metadata = memory_create(VIDEO_META_SIZE, MEM_MODE_RW);
    this->flags_cache = 0;
    this->stats.frames = 0;
    this->stats.buffer_switches = 0;
    this->stats.fps = 0;
    this->stats.utime = 0;

    if (enable) {
        this->flags_cache |= VIDEO_BIT_ENABLED;
    }
    memory_word_set(this->memory->metadata, 0, this->flags_cache);

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
    this->stats.utime = time_utime();
    this->stats.frames = 0;
    this->stats.buffer_switches = 0;
    do {
        word_t flags;
        if (memory_word_get(this->memory->metadata, 0, &flags) != MEM_ERR_OK) {
            continue;
        }

        select_buffer(this, flags);
        state = mfb_update_ex(this->window, this->buffer, VIDEO_SCREEN_WIDTH, VIDEO_SCREEN_HEIGHT);
        if (state != STATE_OK) {
            this->window = 0x0;
            break;
        }

        if (!this->enabled) {
            video_stop(this);
        }

        pic_interrupt_trigger(this->pic, INT_VSYNC);
        print_fps(this);
    } while (mfb_wait_sync(this->window));
    this->window = 0x0;
}

void print_fps(Video *this) {
    this->stats.frames++;

    utime_t current_time = time_utime();
    utime_t elapsed_microsec = current_time - this->stats.utime;
    if (current_time - this->stats.utime >= ONE_SEC_IN_USECS) {
        this->stats.fps = (double) this->stats.frames * (double) ONE_SEC_IN_USECS / (double) elapsed_microsec;
        this->stats.utime = current_time;

        printf("FPS: %2.1f ; %2.1f\n",
               this->stats.fps,
               (double) this->stats.buffer_switches * (double) ONE_SEC_IN_USECS / (double) elapsed_microsec);
        this->stats.buffer_switches = 0;
        this->stats.frames = 0;
    }
}

/**
 * updates this->buffer if needed and returns true if it was updated
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

void video_state_print(Video *this, FILE *file) {
    fprintf(file, "\nVideo state\n");
    fprintf(file, "  enable:%i\n", (this->flags_cache & VIDEO_BIT_ENABLED) != 0);
    fprintf(file, "  buffer:%i\n", (this->flags_cache & VIDEO_BIT_BUFFER) != 0);
    fprintf(file, "  sync:%i\n", (this->flags_cache & VIDEO_BIT_SYNC) != 0);
    fprintf(file, "  fps: %2.0f\n", this->stats.fps);
}