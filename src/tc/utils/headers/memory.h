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

#ifndef CPOCL_MEMORY_H
#define CPOCL_MEMORY_H

#include <stddef.h>
#include <stdlib.h>

typedef struct CpoclMemory CpoclMemory;

#ifndef CPOCL_GLOBAL
#define CPOCL_GLOBAL cpocl_memory
#endif

// defaults
#define CPOCL_MEMORY_GLOBAL() /*noop*/
#define CPOCL_MEMORY_INIT() /*noop*/
#define CPOCL_MEMORY_STATS() /*noop*/

#ifdef CPOCL_MEMORY_DEBUG
#undef CPOCL_MEMORY_GLOBAL
#define CPOCL_MEMORY_GLOBAL() CpoclMemory *CPOCL_GLOBAL;

#undef CPOCL_MEMORY_INIT
#define CPOCL_MEMORY_INIT() CPOCL_GLOBAL = cpocl_memory_create();

#undef CPOCL_MEMORY_STATS
#define CPOCL_MEMORY_STATS() cpocl_memory_print_stats(CPOCL_GLOBAL);

#define malloc(size) cpocl_memory_malloc_int((size), __FILE__, __LINE__)

#define free(ptr) cpocl_memory_free_int((ptr), __FILE__, __LINE__)

#define realloc(ptr, size) cpocl_memory_realloc_int((ptr), (size), __FILE__, __LINE__)
#endif

CpoclMemory *cpocl_memory_create(void);

void cpocl_memory_destroy(CpoclMemory *self);

void cpocl_memory_print_stats(CpoclMemory *self);

void *cpocl_memory_malloc_int(size_t size, const char *file, int line);

void cpocl_memory_free_int(void *ptr, const char *file, int line);

void *cpocl_memory_realloc_int(void *ptr, size_t new_size, const char *file, int line);

#define cpocl_memory_malloc(size) cpocl_memory_malloc_int((size), __FILE__, __LINE__)
#define cpocl_memory_free(ptr) cpocl_memory_free_int((ptr), __FILE__, __LINE__)
#define cpocl_memory_realloc(ptr, size) cpocl_memory_realloc_int((ptr), (size), __FILE__, __LINE__)

#ifdef CPOCL_SHORT_NAMES
#define MEMORY_GLOBAL() CPOCL_MEMORY_GLOBAL()
#define MEMORY_INIT() CPOCL_MEMORY_INIT()
#define MEMORY_STATS() CPOCL_MEMORY_STATS()
#define memory_malloc(size) cpocl_memory_malloc_int((size), __FILE__, __LINE__)
#define memory_free(ptr) cpocl_memory_free_int((ptr), __FILE__, __LINE__)
#define memory_realloc(ptr, size) cpocl_memory_realloc_int((ptr), (size), __FILE__, __LINE__)
#endif // CPOCL_SHORT_NAMES

#endif //CPOCL_MEMORY_H
