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
#include "vmarch.h"
#include "pic.h"
#include "memory.h"

typedef struct PIC {
    PICMemory *memory;
    word_t *active_interrupts;
} PIC;

typedef struct int_loc_t {
    int word_index;
    word_t bit;
} int_loc_t;

int_loc_t find_interrupt_location(interrupt_t interrupt);

bool is_active(const PIC *this, int_loc_t *loc);

bool is_masked(const PIC *this, int_loc_t *loc);

PIC *pic_create() {
    PIC *this = malloc(sizeof(PIC));

    this->memory = malloc(sizeof(PICMemory));
    this->memory->interrupt_handlers = memory_create(INTERRUPTS_COUNT * ADDR_SIZE, MEM_MODE_RW);
    this->memory->interrupt_mask = memory_create(INTERRUPTS_WORDS_COUNT * WORD_SIZE, MEM_MODE_RW);
    this->active_interrupts = malloc(INTERRUPTS_WORDS_COUNT * WORD_SIZE);

    for (int i = 0; i < INTERRUPTS_WORDS_COUNT; i++) {
        this->active_interrupts[i] = 0;
    }

    return this;
}

PICMemory *pic_memory_get(PIC *this) {
    return this->memory;
}

inline int_loc_t find_interrupt_location(interrupt_t interrupt) {
    return (int_loc_t) {
            .word_index = interrupt / (int) INTERRUPTS_PER_WORD,
            .bit = (word_t) 1 << (interrupt % INTERRUPTS_PER_WORD)
    };
}

void pic_interrupt_trigger(PIC *this, interrupt_t interrupt) {
    int_loc_t loc = find_interrupt_location(interrupt);
    this->active_interrupts[loc.word_index] |= loc.bit;
}

void pic_interrupt_reset(PIC *this, interrupt_t interrupt) {
    int_loc_t loc = find_interrupt_location(interrupt);
    this->active_interrupts[loc.word_index] &= ~loc.bit;
}

bool pic_interrupt_active(PIC *this) {
    for (int i = 0; i < INTERRUPTS_WORDS_COUNT; i++) {
        word_t masked;
        memory_word_get(this->memory->interrupt_mask, i * WORD_SIZE, &masked);
        if ((this->active_interrupts[i] & ~masked) > 0) {
            return true;
        }
    }
    return false;
}

interrupt_t pic_interrupt_get(PIC *this) {
    int_loc_t loc = {
            .word_index = 0,
            .bit = 1
    };
    for (int i = 0; i < INTERRUPTS_COUNT; i++) {
        if (is_active(this, &loc) && !is_masked(this, &loc)) {
            return i;
        }
        loc.bit = loc.bit << 1;
        loc.word_index += (loc.bit == 0); // increment if bit == 0
        loc.bit += loc.bit == 0;          // reset to 1 if bit == 0
    }
    return 0;
}

inline bool is_active(const PIC *this, int_loc_t *loc) {
    return (this->active_interrupts[loc->word_index] & loc->bit) > 0;
}

inline bool is_masked(const PIC *this, int_loc_t *loc) {
    word_t masked;
    memory_word_get(this->memory->interrupt_mask, loc->word_index * WORD_SIZE, &masked);
    return (masked & loc->bit) > 0;
}

void pic_destroy(PIC *this) {
    free(this->memory->interrupt_handlers);
    free(this->memory->interrupt_mask);
    free(this->memory);
    free(this->active_interrupts);
    free(this);
}
