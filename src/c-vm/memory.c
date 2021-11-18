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
#include "memory.h"

typedef struct Memory {
    word_sz *raw;
    addr_sz size;
    MemMode mode;
} Memory;

inline addr_sz round_up(addr_sz bytes);

Memory *memory_create(addr_sz bytes, MemMode mode) {
    Memory *memory = malloc(sizeof(Memory));
    memory->size = round_up(bytes);
    memory->raw = malloc(memory->size);
    memory->mode = mode;

    for (uint32_t i = 0; i < bytes / sizeof(addr_sz); i++) {
        memory->raw[i] = 0;
    }

    return memory;
}

addr_sz round_up(addr_sz bytes) {
    return ((bytes + sizeof(word_sz) - 1) / sizeof(word_sz)) * sizeof(word_sz);;
}

void memory_destroy(Memory *memory) {
    free(memory->raw);
    free(memory);
}

addr_sz memory_size_get(Memory *memory) {
    return memory->size;
}

addr_sz memory_max_address_get(Memory *memory) {
    return memory->size - 1;
}

MemMode memory_mode_get(Memory *memory) {
    return memory->mode;
}

MemError memory_word_get(Memory *memory, addr_sz address, word_sz *word) {
    if (address % sizeof(word_sz) != 0) {
        return MEM_ERR_NOT_ALIGNED;
    }
    if (address + sizeof(word_sz) > memory->size) {
        return MEM_ERR_OUT_OF_BOUND;
    }
    *word = memory->raw[address];
    return MEM_ERR_OK;
}

MemError memory_word_set(Memory *memory, addr_sz address, word_sz word) {
    if (memory->mode != MEM_MODE_RW) {
        return MEM_ERR_NOT_WRITABLE;
    }
    if (address % sizeof(word_sz) != 0) {
        return MEM_ERR_NOT_ALIGNED;
    }
    if (address + sizeof(word_sz) > memory->size) {
        return MEM_ERR_OUT_OF_BOUND;
    }
    memory->raw[address] = word;
    return MEM_ERR_OK;
}