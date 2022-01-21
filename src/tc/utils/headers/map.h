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
#include "list.h"

typedef struct Map Map;

typedef struct Pair {
    void *key;
    void *value;
} Pair;

typedef size_t(*hash_fn_t)(void *);

typedef bool(*eq_fn_t)(void *, void *);

Map *map_create(hash_fn_t hash_fn, eq_fn_t eq_fn);

void map_destroy(Map *map);

size_t map_size(Map *this);

bool map_is_empty(Map *this);

void *map_put(Map *this, void *key, void *value);

void *map_get(Map *this, void *key);

void map_remove(Map *this, void *key);

bool map_is_present(Map *this, void *key);

List *map_get_keys(Map *this);

List *map_get_values(Map *this);

List *map_get_entries(Map *this);

size_t map_hash_fn_str(void *key);

bool map_eq_fn_str(void *a, void *b);

#endif //THM_MAP_H
