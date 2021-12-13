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
#include "keyboard.h"

#define VIDEO_BIT_BUFFER 1
#define VIDEO_BIT_ENABLED 2

keyboard_keycode codes[KB_KEY_LAST + 1] = {0};

void init_codes() {
    for (unsigned int i = 0; i <= KB_KEY_LAST; i++) {
        codes[i] = KEYBOARD_KEY_UNKNOWN;
    }
    codes[KB_KEY_SPACE] = KEYBOARD_KEY_SPACE;
    codes[KB_KEY_APOSTROPHE] = KEYBOARD_KEY_APOSTROPHE;
    codes[KB_KEY_COMMA] = KEYBOARD_KEY_COMMA;
    codes[KB_KEY_MINUS] = KEYBOARD_KEY_MINUS;
    codes[KB_KEY_PERIOD] = KEYBOARD_KEY_PERIOD;
    codes[KB_KEY_SLASH] = KEYBOARD_KEY_SLASH;
    codes[KB_KEY_0] = KEYBOARD_KEY_0;
    codes[KB_KEY_1] = KEYBOARD_KEY_1;
    codes[KB_KEY_2] = KEYBOARD_KEY_2;
    codes[KB_KEY_3] = KEYBOARD_KEY_3;
    codes[KB_KEY_4] = KEYBOARD_KEY_4;
    codes[KB_KEY_5] = KEYBOARD_KEY_5;
    codes[KB_KEY_6] = KEYBOARD_KEY_6;
    codes[KB_KEY_7] = KEYBOARD_KEY_7;
    codes[KB_KEY_8] = KEYBOARD_KEY_8;
    codes[KB_KEY_9] = KEYBOARD_KEY_9;
    codes[KB_KEY_SEMICOLON] = KEYBOARD_KEY_SEMICOLON;
    codes[KB_KEY_EQUAL] = KEYBOARD_KEY_EQUAL;
    codes[KB_KEY_A] = KEYBOARD_KEY_A;
    codes[KB_KEY_B] = KEYBOARD_KEY_B;
    codes[KB_KEY_C] = KEYBOARD_KEY_C;
    codes[KB_KEY_D] = KEYBOARD_KEY_D;
    codes[KB_KEY_E] = KEYBOARD_KEY_E;
    codes[KB_KEY_F] = KEYBOARD_KEY_F;
    codes[KB_KEY_G] = KEYBOARD_KEY_G;
    codes[KB_KEY_H] = KEYBOARD_KEY_H;
    codes[KB_KEY_I] = KEYBOARD_KEY_I;
    codes[KB_KEY_J] = KEYBOARD_KEY_J;
    codes[KB_KEY_K] = KEYBOARD_KEY_K;
    codes[KB_KEY_L] = KEYBOARD_KEY_L;
    codes[KB_KEY_M] = KEYBOARD_KEY_M;
    codes[KB_KEY_N] = KEYBOARD_KEY_N;
    codes[KB_KEY_O] = KEYBOARD_KEY_O;
    codes[KB_KEY_P] = KEYBOARD_KEY_P;
    codes[KB_KEY_Q] = KEYBOARD_KEY_Q;
    codes[KB_KEY_R] = KEYBOARD_KEY_R;
    codes[KB_KEY_S] = KEYBOARD_KEY_S;
    codes[KB_KEY_T] = KEYBOARD_KEY_T;
    codes[KB_KEY_U] = KEYBOARD_KEY_U;
    codes[KB_KEY_V] = KEYBOARD_KEY_V;
    codes[KB_KEY_W] = KEYBOARD_KEY_W;
    codes[KB_KEY_X] = KEYBOARD_KEY_X;
    codes[KB_KEY_Y] = KEYBOARD_KEY_Y;
    codes[KB_KEY_Z] = KEYBOARD_KEY_Z;
    codes[KB_KEY_LEFT_BRACKET] = KEYBOARD_KEY_LEFT_BRACKET;
    codes[KB_KEY_BACKSLASH] = KEYBOARD_KEY_BACKSLASH;
    codes[KB_KEY_RIGHT_BRACKET] = KEYBOARD_KEY_RIGHT_BRACKET;
    codes[KB_KEY_GRAVE_ACCENT] = KEYBOARD_KEY_GRAVE_ACCENT;
    codes[KB_KEY_WORLD_1] = KEYBOARD_KEY_WORLD_1;
    codes[KB_KEY_WORLD_2] = KEYBOARD_KEY_WORLD_2;
    codes[KB_KEY_ESCAPE] = KEYBOARD_KEY_ESCAPE;
    codes[KB_KEY_ENTER] = KEYBOARD_KEY_ENTER;
    codes[KB_KEY_TAB] = KEYBOARD_KEY_TAB;
    codes[KB_KEY_BACKSPACE] = KEYBOARD_KEY_BACKSPACE;
    codes[KB_KEY_INSERT] = KEYBOARD_KEY_INSERT;
    codes[KB_KEY_DELETE] = KEYBOARD_KEY_DELETE;
    codes[KB_KEY_RIGHT] = KEYBOARD_KEY_RIGHT;
    codes[KB_KEY_LEFT] = KEYBOARD_KEY_LEFT;
    codes[KB_KEY_DOWN] = KEYBOARD_KEY_DOWN;
    codes[KB_KEY_UP] = KEYBOARD_KEY_UP;
    codes[KB_KEY_PAGE_UP] = KEYBOARD_KEY_PAGE_UP;
    codes[KB_KEY_PAGE_DOWN] = KEYBOARD_KEY_PAGE_DOWN;
    codes[KB_KEY_HOME] = KEYBOARD_KEY_HOME;
    codes[KB_KEY_END] = KEYBOARD_KEY_END;
    codes[KB_KEY_CAPS_LOCK] = KEYBOARD_KEY_CAPS_LOCK;
    codes[KB_KEY_SCROLL_LOCK] = KEYBOARD_KEY_SCROLL_LOCK;
    codes[KB_KEY_NUM_LOCK] = KEYBOARD_KEY_NUM_LOCK;
    codes[KB_KEY_PRINT_SCREEN] = KEYBOARD_KEY_PRINT_SCREEN;
    codes[KB_KEY_PAUSE] = KEYBOARD_KEY_PAUSE;
    codes[KB_KEY_F1] = KEYBOARD_KEY_F1;
    codes[KB_KEY_F2] = KEYBOARD_KEY_F2;
    codes[KB_KEY_F3] = KEYBOARD_KEY_F3;
    codes[KB_KEY_F4] = KEYBOARD_KEY_F4;
    codes[KB_KEY_F5] = KEYBOARD_KEY_F5;
    codes[KB_KEY_F6] = KEYBOARD_KEY_F6;
    codes[KB_KEY_F7] = KEYBOARD_KEY_F7;
    codes[KB_KEY_F8] = KEYBOARD_KEY_F8;
    codes[KB_KEY_F9] = KEYBOARD_KEY_F9;
    codes[KB_KEY_F10] = KEYBOARD_KEY_F10;
    codes[KB_KEY_F11] = KEYBOARD_KEY_F11;
    codes[KB_KEY_F12] = KEYBOARD_KEY_F12;
    codes[KB_KEY_F13] = KEYBOARD_KEY_F13;
    codes[KB_KEY_F14] = KEYBOARD_KEY_F14;
    codes[KB_KEY_F15] = KEYBOARD_KEY_F15;
    codes[KB_KEY_F16] = KEYBOARD_KEY_F16;
    codes[KB_KEY_F17] = KEYBOARD_KEY_F17;
    codes[KB_KEY_F18] = KEYBOARD_KEY_F18;
    codes[KB_KEY_F19] = KEYBOARD_KEY_F19;
    codes[KB_KEY_F20] = KEYBOARD_KEY_F20;
    codes[KB_KEY_F21] = KEYBOARD_KEY_F21;
    codes[KB_KEY_F22] = KEYBOARD_KEY_F22;
    codes[KB_KEY_F23] = KEYBOARD_KEY_F23;
    codes[KB_KEY_F24] = KEYBOARD_KEY_F24;
    codes[KB_KEY_F25] = KEYBOARD_KEY_F25;
    codes[KB_KEY_KP_0] = KEYBOARD_KEY_KP_0;
    codes[KB_KEY_KP_1] = KEYBOARD_KEY_KP_1;
    codes[KB_KEY_KP_2] = KEYBOARD_KEY_KP_2;
    codes[KB_KEY_KP_3] = KEYBOARD_KEY_KP_3;
    codes[KB_KEY_KP_4] = KEYBOARD_KEY_KP_4;
    codes[KB_KEY_KP_5] = KEYBOARD_KEY_KP_5;
    codes[KB_KEY_KP_6] = KEYBOARD_KEY_KP_6;
    codes[KB_KEY_KP_7] = KEYBOARD_KEY_KP_7;
    codes[KB_KEY_KP_8] = KEYBOARD_KEY_KP_8;
    codes[KB_KEY_KP_9] = KEYBOARD_KEY_KP_9;
    codes[KB_KEY_KP_DECIMAL] = KEYBOARD_KEY_KP_DECIMAL;
    codes[KB_KEY_KP_DIVIDE] = KEYBOARD_KEY_KP_DIVIDE;
    codes[KB_KEY_KP_MULTIPLY] = KEYBOARD_KEY_KP_MULTIPLY;
    codes[KB_KEY_KP_SUBTRACT] = KEYBOARD_KEY_KP_SUBTRACT;
    codes[KB_KEY_KP_ADD] = KEYBOARD_KEY_KP_ADD;
    codes[KB_KEY_KP_ENTER] = KEYBOARD_KEY_KP_ENTER;
    codes[KB_KEY_KP_EQUAL] = KEYBOARD_KEY_KP_EQUAL;
    codes[KB_KEY_LEFT_SHIFT] = KEYBOARD_KEY_LEFT_SHIFT;
    codes[KB_KEY_LEFT_CONTROL] = KEYBOARD_KEY_LEFT_CONTROL;
    codes[KB_KEY_LEFT_ALT] = KEYBOARD_KEY_LEFT_ALT;
    codes[KB_KEY_LEFT_SUPER] = KEYBOARD_KEY_LEFT_SUPER;
    codes[KB_KEY_RIGHT_SHIFT] = KEYBOARD_KEY_RIGHT_SHIFT;
    codes[KB_KEY_RIGHT_CONTROL] = KEYBOARD_KEY_RIGHT_CONTROL;
    codes[KB_KEY_RIGHT_ALT] = KEYBOARD_KEY_RIGHT_ALT;
    codes[KB_KEY_RIGHT_SUPER] = KEYBOARD_KEY_RIGHT_SUPER;
    codes[KB_KEY_MENU] = KEYBOARD_KEY_MENU;
}

typedef struct Video {
    PIC *pic;
    VideoMemory *memory;
    Keyboard *keyboard;
    struct mfb_window *window;
    bool enabled;
    uint32_t *buffer;
    struct {
        utime_t utime;
        int frames;
        int buffer_switches;
        double fps;
    } stats;
    pthread_cond_t meta_memory_written;
    pthread_mutex_t meta_memory_written_lock;
    pthread_t thread;
} Video;

void video_kb_callback(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed);

void *video_thread_run(void *ptr);

void video_thread_stop(Video *this);

void print_fps(Video *this);

Video *video_create(Bus *bus, PIC *pic, Keyboard *keyboard, bool enable) {
    init_codes();
    Video *this = malloc(sizeof(Video));
    this->pic = pic;
    this->memory = malloc(sizeof(VideoMemory));
    this->keyboard = keyboard;
    this->window = 0x0;
    this->enabled = enable;
    this->memory->metadata = memory_create(VIDEO_META_SIZE, MEM_MODE_RW);
    this->stats.frames = 0;
    this->stats.buffer_switches = 0;
    this->stats.fps = 0;
    this->stats.utime = 0;
    pthread_cond_init(&this->meta_memory_written, NULL);
    pthread_mutex_init(&this->meta_memory_written_lock, NULL);

    bus_memory_attach(bus, this->memory->metadata, VIDEO_META_ADDRESS, "VMeta");
    memory_word_set(this->memory->metadata, 0, VIDEO_BIT_ENABLED);

    if (this->enabled) {
        this->memory->buffer[0] = memory_create(VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_HEIGHT * 4, MEM_MODE_RW);
        this->memory->buffer[1] = memory_create(VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_HEIGHT * 4, MEM_MODE_RW);
        this->buffer = memory_raw_get(this->memory->buffer[0]);

        bus_memory_attach(bus, this->memory->buffer[0], VIDEO_BUFFER_0_ADDRESS, "VBuf0");
        bus_memory_attach(bus, this->memory->buffer[1], VIDEO_BUFFER_1_ADDRESS, "VBuf1");
    } else {
        this->memory->buffer[0] = NULL;
        this->memory->buffer[1] = NULL;
        this->buffer = NULL;
    }

    bus_notification_register(bus, &this->meta_memory_written, VIDEO_META_ADDRESS);
    return this;
}

void video_loop(Video *this) {
    if (this->enabled && this->buffer) {
        this->window = mfb_open("thm", VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_SCALE,
                                VIDEO_SCREEN_HEIGHT * VIDEO_SCREEN_SCALE);
    }
    if (!this->window) {
        return;
    }
    mfb_set_user_data(this->window, this);
    mfb_set_viewport(this->window,
                     0, 0,
                     VIDEO_SCREEN_WIDTH * VIDEO_SCREEN_SCALE,
                     VIDEO_SCREEN_HEIGHT * VIDEO_SCREEN_SCALE
    );
    mfb_set_target_fps(VIDEO_SCREEN_FPS);

    mfb_set_keyboard_callback(this->window, video_kb_callback);

    pthread_create(&this->thread, NULL, video_thread_run, this);

    mfb_update_state state;
    this->stats.utime = time_utime();
    this->stats.frames = 0;
    this->stats.buffer_switches = 0;
    do {
        word_t flags;
        if (memory_word_get(this->memory->metadata, 0, &flags) != MEM_ERR_OK) {
            continue;
        }

        state = mfb_update_ex(this->window, this->buffer, VIDEO_SCREEN_WIDTH, VIDEO_SCREEN_HEIGHT);
        if (state != STATE_OK) {
            this->window = 0x0;
            break;
        }

        if (!this->enabled) {
            video_stop(this);
        }

        pic_interrupt_trigger(this->pic, INT_VSYNC);
        print_fps(this); // todo create an cli option for that
    } while (mfb_wait_sync(this->window));
    this->window = 0x0;
}

void video_kb_callback(struct mfb_window *window, mfb_key key, mfb_key_mod mod, bool isPressed) {
    if (key == KB_KEY_ESCAPE) {
        mfb_close(window);
    }

    Video *video = (Video *) mfb_get_user_data(window);

    if (isPressed) {
        keyboard_key_pressed(video->keyboard, codes[key] << 8 | mod);
    } else {
        keyboard_key_released(video->keyboard, codes[key] << 8 | mod);
    }
}

/**
 * Waits for out_memory update and update video buffer accordingly.
 * @param ptr pointer to this
 * @return NULL
 */
void *video_thread_run(void *ptr) {
    Video *this = (Video *) ptr;

    word_t flags = 0;
    memory_word_get(this->memory->metadata, 0, &flags);

    while (this->window) {
        if (pthread_mutex_lock(&this->meta_memory_written_lock)) {
            perror("pthread_mutex_lock");
            pthread_exit(NULL);
        }
        pthread_cond_wait(&this->meta_memory_written, &this->meta_memory_written_lock);

        word_t new_flags = 0;
        if (memory_word_get(this->memory->metadata, 0, &new_flags) != MEM_ERR_OK) {
            continue;
        }
        if ((flags ^ new_flags) & VIDEO_BIT_BUFFER) {
            this->buffer = memory_raw_get(this->memory->buffer[new_flags & VIDEO_BIT_BUFFER]);
            this->stats.buffer_switches++;
        }
        flags = new_flags;

        pthread_mutex_unlock(&this->meta_memory_written_lock);
    }
    return NULL;
}

void video_thread_stop(Video *this) {
    pthread_cond_signal(&this->meta_memory_written);
    pthread_join(this->thread, NULL);
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

void video_stop(Video *this) {
    this->enabled = false;
    mfb_close(this->window);
}

void video_destroy(Video *this) {
    video_stop(this);
    video_thread_stop(this);
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
    fprintf(file, "  enable:%i\n", this->enabled);
    fprintf(file, "  fps: %2.0f\n", this->stats.fps);
}