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
#include "list.h"

typedef struct Map Map;

typedef struct Pair {
    void *key;
    void *value;
} Pair;

typedef struct CpoclMapOptions {
    void *(*malloc)(size_t);

    void (*free)(void *);
} CpoclMapOptions;

typedef size_t(*hash_fn)(void *);

typedef bool(*eq_fn)(void *, void *);

#define map_create(hash_fn, eq_fn, ...) \
    map_create_((hash_fn), (eq_fn), (struct CpoclMapOptions) { \
        .malloc = malloc,       \
        .free = free,           \
        __VA_ARGS__ \
    })

Map *map_create_(hash_fn hash, eq_fn eq, CpoclMapOptions options);

void map_destroy(Map *self);

size_t map_size(Map *self);

bool map_is_empty(Map *self);

void *map_put(Map *self, void *key, void *value);

void *map_get(Map *self, void *key);

void map_remove(Map *self, void *key);

bool map_is_present(Map *self, void *key);

List *map_get_keys(Map *self);

List *map_get_values(Map *self);

List *map_get_entries(Map *self);

size_t map_hash_fn_str(void *key);

bool map_eq_fn_str(void *a, void *b);

#endif //THM_MAP_H
