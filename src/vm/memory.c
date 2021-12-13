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
#include <printf.h>
#include "vmarch.h"
#include "memory.h"

typedef struct Memory {
    word_t *raw;
    addr_t size;
    MemMode mode;
} Memory;

#define ROUND_UP(i) ((((i) + WORD_SIZE - 1) / WORD_SIZE) * WORD_SIZE)

Memory *memory_create(addr_t bytes, MemMode mode) {
    Memory *memory = malloc(sizeof(Memory));
    memory->size = ROUND_UP(bytes);
    memory->raw = malloc(memory->size);
    memory->mode = mode;

    for (addr_t i = 0; i < bytes / sizeof(addr_t); i++) {
        memory->raw[i] = 0;
    }

    return memory;
}

void memory_destroy(Memory *memory) {
    free(memory->raw);
    free(memory);
}

addr_t memory_size_get(Memory *memory) {
    return memory->size;
}

addr_t memory_max_address_get(Memory *memory) {
    return memory->size - 1;
}

MemMode memory_mode_get(Memory *memory) {
    return memory->mode;
}

void memory_mode_set(Memory *memory, MemMode mode) {
    memory->mode = mode;
}

MemError memory_word_get(Memory *memory, addr_t address, word_t *word) {
    if (address % WORD_SIZE != 0) {
        return MEM_ERR_NOT_ALIGNED;
    }
    if (address + WORD_SIZE > memory->size) {
        return MEM_ERR_OUT_OF_BOUND;
    }
    *word = memory->raw[address / WORD_SIZE];
    return MEM_ERR_OK;
}

MemError memory_word_set(Memory *memory, addr_t address, word_t word) {
#ifdef WITH_MEMORY_LOG_MESSAGE
    printf("MEM Write to %i (size: %i)\n", address, out_memory->size);
#endif
    if (memory->mode != MEM_MODE_RW) {
        return MEM_ERR_NOT_WRITABLE;
    }
    if (address % WORD_SIZE != 0) {
        return MEM_ERR_NOT_ALIGNED;
    }
    if (address + WORD_SIZE > memory->size) {
        return MEM_ERR_OUT_OF_BOUND;
    }
    memory->raw[address / WORD_SIZE] = word;
    return MEM_ERR_OK;
}

inline void *memory_raw_get(Memory *memory) {
    return memory->raw;
}