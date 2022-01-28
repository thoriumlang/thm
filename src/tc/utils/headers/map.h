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

#ifndef THM_MAP_H
#define THM_MAP_H

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>
#include "functions.h"
#include "list.h"

typedef struct CpoclMap CpoclMap;

typedef struct CpoclMapOptions {
    void *(*malloc)(size_t);

    void *(*realloc)(void *, size_t);

    void (*free)(void *);
} CpoclMapOptions;

#define cpocl_map_create(hash_fn, eq_fn, ...) \
    cpocl_map_create_with_opts((hash_fn), (eq_fn), (struct CpoclMapOptions) { \
        .malloc = malloc,       \
        .realloc = realloc,     \
        .free = free,           \
        __VA_ARGS__             \
    })

CpoclMap *cpocl_map_create_with_opts(cpocl_hash_fn hash, cpocl_eq_fn eq, CpoclMapOptions options);

void cpocl_map_destroy(CpoclMap *self);

size_t cpocl_map_size(CpoclMap *self);

bool cpocl_map_is_empty(CpoclMap *self);

void *cpocl_map_put(CpoclMap *self, void *key, void *value);

void *cpocl_map_get(CpoclMap *self, void *key);

void cpocl_map_remove(CpoclMap *self, void *key);

bool cpocl_map_is_present(CpoclMap *self, void *key);

CpoclList *cpocl_map_get_keys(CpoclMap *self);

CpoclList *cpocl_map_get_values(CpoclMap *self);

CpoclList *cpocl_map_get_entries(CpoclMap *self);

size_t cpocl_map_hash_fn_str(void *key);

bool cpocl_map_eq_fn_str(void *a, void *b);

#ifdef CPOCL_SHORT_NAMES
#define Map CpoclMap
#define MapOptions CpoclMapOptions
#define map_create cpocl_map_create
#define map_create_with_opts cpocl_map_create_with_opts
#define map_destroy cpocl_map_destroy
#define map_size cpocl_map_size
#define map_is_empty cpocl_map_is_empty
#define map_put cpocl_map_put
#define map_get cpocl_map_get
#define map_remove cpocl_map_remove
#define map_is_present cpocl_map_is_present
#define map_get_keys cpocl_map_get_keys
#define map_get_values cpocl_map_get_values
#define map_get_entries cpocl_map_get_entries
#define map_hash_fn_str cpocl_map_hash_fn_str
#define map_eq_fn_str cpocl_map_eq_fn_str
#endif

#endif //THM_MAP_H
