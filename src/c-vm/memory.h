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

#ifndef C_VM_MEMORY_H
#define C_VM_MEMORY_H

#include <stdint.h>
#include "vmarch.h"

typedef struct Memory Memory;

typedef enum {
    MEM_MODE_R, MEM_MODE_RW
} MemMode;
typedef enum {
    MEM_ERR_OK = 1,
    MEM_ERR_OUT_OF_BOUND,
    MEM_ERR_NOT_ALIGNED,
    MEM_ERR_NOT_WRITABLE
} MemError;

Memory *memory_create(addr_sz bytes, MemMode mode);

void memory_destroy(Memory *memory);

addr_sz memory_size_get(Memory *memory);

addr_sz memory_max_address_get(Memory *memory);

MemMode memory_mode_get(Memory *memory);

void memory_mode_set(Memory *memory, MemMode mode);

MemError memory_word_get(Memory *memory, addr_sz address, word_sz *word);

MemError memory_word_set(Memory *memory, addr_sz address, word_sz word);

#endif //C_VM_MEMORY_H
