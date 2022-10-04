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

#ifndef THM_MEMORY_H
#define THM_MEMORY_H

#include <stddef.h>

typedef struct CpoclMemory CpoclMemory;

#ifndef CPOCL_GLOBAL
#define CPOCL_GLOBAL cpocl_memory
#endif

#ifdef CPOCL_MEMORY_DEBUG
#pragma region Debug enabled
#define CPOCL_MEMORY_GLOBAL() CpoclMemory *CPOCL_GLOBAL;
#define CPOCL_MEMORY_INIT() CPOCL_GLOBAL = cpocl_memory_create();
#define CPOCL_MEMORY_STATS() cpocl_memory_print_stats(CPOCL_GLOBAL);

#define cpocl_memory_alloc(size) cpocl_memory_alloc_debug((size), __FILE__, __LINE__)
#define cpocl_memory_free(ptr) cpocl_memory_free_debug((ptr), __FILE__, __LINE__)
#define cpocl_memory_realloc(ptr, size) cpocl_memory_realloc_debug((ptr), (size), __FILE__, __LINE__)

#ifdef CPOCL_SHORT_NAMES
#define memory_create() cpocl_memory_create()
#define memory_destroy(self) cpocl_memory_destroy((self))
#define memory_print_stats(self) cpocl_memory_print_stats((self))
#define memory_alloc(size) cpocl_memory_alloc_debug((size), __FILE__, __LINE__)
#define memory_free(ptr) cpocl_memory_free_debug((ptr), __FILE__, __LINE__)
#define memory_realloc(ptr, size) cpocl_memory_realloc_debug((ptr), (size), __FILE__, __LINE__)
#endif // CPOCL_SHORT_NAMES

CpoclMemory *cpocl_memory_create(void);

void cpocl_memory_destroy(CpoclMemory *self);

void cpocl_memory_print_stats(CpoclMemory *self);

void *cpocl_memory_alloc_debug(size_t size, char *file, int line);

void cpocl_memory_free_debug(void *ptr, char *file, int line);

void *cpocl_memory_realloc_debug(void *ptr, size_t new_size, char *file, int line);

//void *cpocl_memory_alloc_nodebug(size_t size);
//
//void cpocl_memory_free_nodebug(void *ptr);
//
//void *cpocl_memory_realloc_nodebug(void *ptr, size_t new_size);

#pragma endregion
#else // CPOCL_MEMORY_DEBUG
#pragma region Debug disabled
#define CPOCL_MEMORY_GLOBAL() /*noop*/
#define CPOCL_MEMORY_INIT() /*noop*/
#define CPOCL_MEMORY_STATS() /*noop*/

#ifdef CPOCL_SHORT_NAMES
#define memory_alloc(size) cpocl_memory_alloc((size))
#define memory_free(ptr) cpocl_memory_free((ptr))
#define memory_realloc(ptr, size) cpocl_memory_realloc((ptr), (size))
#endif // CPOCL_SHORT_NAMES

void *cpocl_memory_alloc(size_t size);

void cpocl_memory_free(void *ptr);

void *cpocl_memory_realloc(void *ptr, size_t new_size);

#pragma endregion
#endif // CPOCL_MEMORY_DEBUG

#ifdef CPOCL_SHORT_NAMES
#define MEMORY_GLOBAL() CPOCL_MEMORY_GLOBAL()
#define MEMORY_INIT() CPOCL_MEMORY_INIT()
#define MEMORY_STATS() CPOCL_MEMORY_STATS()
#endif // CPOCL_SHORT_NAMES

#endif //THM_MEMORY_H
