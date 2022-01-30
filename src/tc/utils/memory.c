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

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include "headers/memory.h"

#ifdef CPOCL_MEMORY_DEBUG
#pragma region Debug enabled

extern CpoclMemory *CPOCL_GLOBAL;

enum MEMORY_BLOCK_FLAGS {
    MEMORY_BLOCK_FLAGS_freed = 1,
};

typedef struct MemoryBlock {
    size_t size;
    int flags;
    struct MemoryBlock *next;
    char *alloc_file;
    int alloc_line;
    char *freed_file;
    int freed_line;
} MemoryBlock;

typedef struct CpoclMemory {
    size_t total_allocated;
    size_t allocated;
    MemoryBlock *blocks;
    MemoryBlock *last_block;
} CpoclMemory;

CpoclMemory *cpocl_memory_create(void) {
    CpoclMemory *mem = malloc(sizeof(CpoclMemory));
    mem->allocated = 0;
    mem->blocks = NULL;
    mem->last_block = NULL;
    return mem;
}

void cpocl_memory_destroy(CpoclMemory *self) {
    free(self);
}

void cpocl_memory_print_stats(CpoclMemory *self) {
    fprintf(stderr, "Total allocated: %lu bytes\n", self->total_allocated);
    fprintf(stderr, "Still allocated: %lu bytes\n", self->allocated);
    MemoryBlock *block = CPOCL_GLOBAL->blocks;
    while (block != NULL) {
        if ((block->flags & MEMORY_BLOCK_FLAGS_freed) != MEMORY_BLOCK_FLAGS_freed) {
            fprintf(stderr, " - block allocated at %s:%d not freed (%lu bytes)\n",
                    block->alloc_file, block->alloc_line, block->size);
        }
        block = block->next;
    }
}

void *cpocl_memory_alloc_debug(size_t size, char *file, int line) {
    fprintf(stderr, "%s:%d: allocating %lu bytes\n",
            file, line, size);

    MemoryBlock *block = malloc(size + sizeof(MemoryBlock));
    block->size = size;
    block->flags = 0;
    block->next = NULL;
    block->alloc_file = malloc(strlen(file) + 1);
    memcpy(block->alloc_file, file, strlen(file) + 1);
    block->alloc_line = line;

    CPOCL_GLOBAL->total_allocated += size;
    CPOCL_GLOBAL->allocated += size;
    if (CPOCL_GLOBAL->blocks == NULL) {
        CPOCL_GLOBAL->blocks = block;
    }
    if (CPOCL_GLOBAL->last_block == NULL) {
        CPOCL_GLOBAL->last_block = block;
    } else {
        CPOCL_GLOBAL->last_block->next = block;
        CPOCL_GLOBAL->last_block = block;
    }

    return (void *) (((int8_t *) block) + sizeof(MemoryBlock));
}

void cpocl_memory_free_debug(void *ptr, char *file, int line) {
    MemoryBlock *block = (void *) ((int8_t *) ptr - sizeof(MemoryBlock));

    if ((block->flags & MEMORY_BLOCK_FLAGS_freed) == MEMORY_BLOCK_FLAGS_freed) {
        fprintf(stderr, "%s:%d: cannot free block allocated at %s:%d: already freed at %s:%d\n",
                file, line, block->alloc_file, block->alloc_line, block->freed_file, block->freed_line);
        exit(1);
    }
    fprintf(stderr, "%s:%d: freeing block allocated at %s:%d (%lu bytes)\n",
            file, line, block->alloc_file, block->alloc_line, block->size);
    block->flags |= MEMORY_BLOCK_FLAGS_freed;
    block->freed_file = malloc(strlen(file) + 1);
    memcpy(block->freed_file, file, strlen(file) + 1);
    block->freed_line = line;
    CPOCL_GLOBAL->allocated -= block->size;
}

void *cpocl_memory_realloc_debug(void *ptr, size_t new_size, char *file, int line) {
    MemoryBlock *block = (void *) ((int8_t *) ptr - sizeof(MemoryBlock));

    if ((block->flags & MEMORY_BLOCK_FLAGS_freed) == MEMORY_BLOCK_FLAGS_freed) {
        fprintf(stderr, "%s:%d: cannot realloc: block allocated at %s:%d already freed at %s:%d\n",
                file, line, block->alloc_file, block->alloc_line, block->freed_file, block->freed_line);
        exit(1);
    }

    fprintf(stderr, "%s:%d: reallocating block allocated at %s:%d: %lu bytes -> %lu bytes\n",
            file, line, block->alloc_file, block->alloc_line, block->size, new_size);

    block = realloc(block, new_size + sizeof(MemoryBlock));

    CPOCL_GLOBAL->allocated += new_size - block->size;
    if (new_size > block->size) {
        CPOCL_GLOBAL->total_allocated += new_size - block->size;
    }
    block->size = new_size;

    return (void *) (((int8_t *) block) + sizeof(MemoryBlock));
}

void *cpocl_memory_alloc_nodebug(size_t size) {
    return cpocl_memory_alloc_debug(size, (char *) &"?", 0);
}

void cpocl_memory_free_nodebug(void *ptr) {
    cpocl_memory_free_debug(ptr, (char *) &"?", 0);
}

void *cpocl_memory_realloc_nodebug(void *ptr, size_t new_size) {
    return cpocl_memory_realloc_debug(ptr, new_size, (char *) &"?", 0);
}

#pragma endregion
#else
#pragma region Debug disabled

inline void *cpocl_memory_alloc(size_t size) {
    return malloc(size);
}

inline void cpocl_memory_free(void *ptr) {
    free(ptr);
}

inline void *cpocl_memory_realloc(void *ptr, size_t new_size) {
    return realloc(ptr, new_size);
}

#pragma endregion
#endif
