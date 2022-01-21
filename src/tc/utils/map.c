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
#include <string.h>
#include "headers/map.h"

#define MAP_BUCKETS_COUNT 255

typedef struct MapEntry {
    size_t hash;
    void *key;
    void *value;
    struct MapEntry *next;
} MapEntry;

typedef struct Map {
    hash_fn_t hash_fn;
    eq_fn_t eq_fn;
    MapEntry *buckets[MAP_BUCKETS_COUNT];
} Map;

Map *map_create(hash_fn_t hash_fn, eq_fn_t eq_fn) {
    Map *map = malloc(sizeof(Map));
    map->hash_fn = hash_fn;
    map->eq_fn = eq_fn;
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        map->buckets[i] = NULL;
    }

    return map;
}

void map_destroy(Map *this) {
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *entry = this->buckets[i];
        while (entry) {
            MapEntry *current = entry;
            entry = entry->next;
            free(current);
        }
    }
    free(this);
}

size_t map_size(Map *this) {
    size_t size = 0;
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *e = this->buckets[i];
        while (e) {
            size++;
            e = e->next;
        }
    }
    return size;
}

bool map_is_empty(Map *this) {
    return map_size(this) == 0;
}

static bool map_key_eq(Map *this, MapEntry *e, void *key, size_t hash) {
    return e->hash == hash && this->eq_fn(key, e->key);
}

static MapEntry *map_entry_create(void *key, void *value, size_t hash) {
    MapEntry *entry = malloc(sizeof(MapEntry));
    entry->hash = hash;
    entry->next = NULL;
    entry->key = key;
    entry->value = value;

    return entry;
}

static void map_entry_destroy(MapEntry *this) {
    free(this);
}

void *map_put(Map *this, void *key, void *value) {
    size_t hash = this->hash_fn(key);
    size_t bucket_idx = hash % MAP_BUCKETS_COUNT;

    if (this->buckets[bucket_idx]) {
        MapEntry *e = this->buckets[bucket_idx];
        while (true) {
            if (map_key_eq(this, e, key, hash)) {
                void *old_val = e->value;
                e->value = value;
                return old_val;
            } else if (e->next == NULL) {
                e->next = map_entry_create(key, value, hash);
                return NULL;
            }
            e = e->next;
        }
    } else {
        this->buckets[bucket_idx] = map_entry_create(key, value, hash);
        return NULL;
    }
}

void *map_get(Map *this, void *key) {
    MapEntry *entry = this->buckets[(this->hash_fn(key) % MAP_BUCKETS_COUNT)];
    while (entry) {
        if (entry->hash == this->hash_fn(key) && this->eq_fn(key, entry->key)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void map_remove(Map *this, void *key) {
    size_t hash = this->hash_fn(key);
    size_t bucket_idx = hash % MAP_BUCKETS_COUNT;

    MapEntry *previous = NULL;
    MapEntry *entry = this->buckets[bucket_idx];
    while (entry) {
        if (entry->hash == this->hash_fn(key) && this->eq_fn(key, entry->key)) {
            if (previous) {
                previous->next = entry->next;
            } else {
                this->buckets[bucket_idx] = entry->next;
            }
            map_entry_destroy(entry);
            return;
        }
        previous = entry;
        entry = entry->next;
    }
}

bool map_is_present(Map *this, void *key) {
    return map_get(this, key) != NULL;
}

List *map_get_keys(Map *this) {
    List *keys = list_create();

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *e = this->buckets[i];
        while (e) {
            list_add(keys, e->key);
            e = e->next;
        }
    }

    return keys;
}

List *map_get_values(Map *this) {
    List *keys = list_create();

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *e = this->buckets[i];
        while (e) {
            list_add(keys, e->value);
            e = e->next;
        }
    }

    return keys;
}

List *map_get_entries(Map *this) {
    List *entries = list_create();

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *e = this->buckets[i];
        while (e) {
            Pair *p = malloc(sizeof(Pair));
            p->key = e->key;
            p->value = e->value;
            list_add(entries, p);
            e = e->next;
        }
    }

    return entries;
}

size_t map_hash_fn_str(void *key) {
    char *c = (char *) key;
    size_t hash = 5381;
    while (*c) {
        hash = ((hash << 5) + hash) ^ *c++;
    }
    return hash;
}

bool map_eq_fn_str(void *a, void *b) {
    return strcmp(a, b) == 0;
}