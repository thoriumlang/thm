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

#ifndef THM_PIC_H
#define THM_PIC_H

#include <pthread.h>
#include "vmarch.h"
#include "memory.h"

extern pthread_cond_t pic_got_interrupt;
extern pthread_mutex_t pic_got_interrupt_lock;

typedef struct PIC PIC;

typedef struct PICMemory {
    Memory *interrupt_mask;
    Memory *interrupt_handlers;
} PICMemory;

typedef uint8_t interrupt_t;

PIC *pic_create();

PICMemory *pic_memory_get(PIC *this);

void pic_interrupt_mask(PIC *this, interrupt_t interrupt);

void pic_interrupt_unmask(PIC *this, interrupt_t interrupt);

bool pic_interrupt_active(PIC *this);

interrupt_t pic_interrupt_get(PIC *this);

void pic_interrupt_trigger(PIC *this, interrupt_t interrupt);

void pic_interrupt_reset(PIC *this, interrupt_t interrupt);

void pic_destroy(PIC *this);

#endif //THM_PIC_H
