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
#include <unistd.h>
#include <printf.h>
#include "pit.h"
#include "pic.h"
#include "vmarch.h"
#include "time.h"

// todo rework that so it can be programmed from programs
typedef struct PIT {
    PIC *pic;
    microsec_t period;
    pthread_t thread;
    volatile bool running;
    interrupt_t interrupt;
} PIT;

void *timer_loop(void *ptr);

PIT *pit_create(PIC *pic, microsec_t period, interrupt_t interrupt) {
    PIT *timer = malloc(sizeof(PIT));
    timer->pic = pic;
    timer->period = period;
    timer->interrupt = interrupt;
    timer->thread = 0;
    return timer;
}

void pit_destroy(PIT *this) {
    pit_stop(this);
    free(this);
}

void pit_start(PIT *this) {
    this->running = true;
    pthread_create(&this->thread, NULL, timer_loop, this);
}

void *timer_loop(void *ptr) {
    PIT *this = (PIT *) ptr;

    utime_t time = time_utime();

    while (this->running) {
        usleep(100);
        utime_t new_time = time_utime();
        if (new_time > time + this->period) {
            pic_interrupt_trigger(this->pic, this->interrupt);
            time = new_time;
        }
    }
    return NULL;
}

void pit_stop(PIT *this) {
    this->running = false;
    pthread_join(this->thread, NULL);
    this->thread = 0;
}