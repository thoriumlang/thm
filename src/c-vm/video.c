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
#include <MiniFB.h>
#include <unistd.h>
#include "vmarch.h"
#include "video.h"


typedef struct Video {
    struct mfb_window *window;
    bool enabled;
} Video;

Video *video_create(bool enable) {
    Video *this = malloc(sizeof(Video));
    this->window = 0x0;
    this->enabled = enable;
    return this;
}

void video_loop(Video *this) {
    if (this->enabled) {
        this->window = mfb_open("thm", VIDEO_SCREEN_WIDTH, VIDEO_SCREEN_HEIGHT);
    }
    if (!this->window) {
        return;
    }
    mfb_set_viewport(this->window, 0, 0, VIDEO_SCREEN_WIDTH, VIDEO_SCREEN_HEIGHT);

    uint32_t i, noise, carry, seed = 0xbeef;
    uint32_t *g_buffer = (uint32_t *) malloc(VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_HEIGHT * 4);

    mfb_update_state state;
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
            g_buffer[i] = MFB_RGB(noise, noise, noise);
        }

        state = mfb_update_ex(this->window, g_buffer, VIDEO_SCREEN_WIDTH, VIDEO_SCREEN_HEIGHT);
        if (state != STATE_OK) {
            this->window = 0x0;
            break;
        }
        if (!this->enabled) {
            video_stop(this);
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
    free(this);
}
