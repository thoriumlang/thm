/**
 * Copyright 2021 Christophe Pollet
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
#include <pthread.h>
#include "bus.h"
#include "pic.h"
#include "keyboard.h"

typedef struct Keyboard {
    PIC *pic;
    Memory *out_memory; // from keyboard to ext. world
    Memory *in_memory;  // from ext. world to keyboard
    pthread_cond_t in_memory_written;
    pthread_mutex_t in_memory_written_lock;
    pthread_t thread;
    bool running;
} Keyboard;

void *keyboard_thread_run(void *ptr);

void keyboard_thread_stop(Keyboard *this);

Keyboard *keyboard_create(Bus *bus, PIC *pic) {
    Keyboard *keyboard = malloc(sizeof(Keyboard));
    keyboard->pic = pic;
    keyboard->out_memory = memory_create(KEYBOARD_OUT_MEMORY_SIZE, MEM_MODE_RW);
    keyboard->in_memory = memory_create(KEYBOARD_IN_MEMORY_SIZE, MEM_MODE_RW);
    keyboard->running = false;

    pthread_cond_init(&keyboard->in_memory_written, NULL);
    pthread_mutex_init(&keyboard->in_memory_written_lock, NULL);

    bus_memory_attach(bus, keyboard->out_memory, KEYBOARD_OUT_ADDRESS, "KBOut");
    bus_memory_attach(bus, keyboard->in_memory, KEYBOARD_IN_ADDRESS, "KBIn");
    bus_notification_register(bus, &keyboard->in_memory_written, KEYBOARD_IN_ADDRESS);

    return keyboard;
}

void keyboard_start(Keyboard *this) {
    this->running = true;
    pthread_create(&this->thread, NULL, keyboard_thread_run, this);
}

void *keyboard_thread_run(void *ptr) {
    Keyboard *kb = ((Keyboard *) ptr);

    while (kb->running) {
        if (pthread_mutex_lock(&kb->in_memory_written_lock)) {
            perror("pthread_mutex_lock");
            pthread_exit(NULL);
        }

        pthread_cond_wait(&kb->in_memory_written, &kb->in_memory_written_lock);

        // todo move next char from buffer to out_memory

        pthread_mutex_unlock(&kb->in_memory_written_lock);
    }

    return NULL;
}

void keyboard_thread_stop(Keyboard *this) {
    pthread_cond_signal(&this->in_memory_written);
    pthread_join(this->thread, NULL);
}

void keyboard_destroy(Keyboard *this) {
    this->running = false;
    keyboard_thread_stop(this);
    free(this->out_memory);
    free(this);
}

void keyboard_key_pressed(Keyboard *this, keyboard_key key) {
    word_t word = (key << 8) | 1;
    // printf("keyboard: "WXHEX"\n", word);
    memory_word_set(this->out_memory, 0, word);
    pic_interrupt_trigger(this->pic, INT_KEYBOARD);
}

void keyboard_key_released(Keyboard *this, keyboard_key key) {
    word_t word = (key << 8);
    // printf("keyboard: "WXHEX"\n", word);
    memory_word_set(this->out_memory, 0, word);
    pic_interrupt_trigger(this->pic, INT_KEYBOARD);
}