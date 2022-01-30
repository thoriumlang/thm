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
    CpoclMapOptions opts;
    hash_fn hash;
    eq_fn eq;
    MapEntry *buckets[MAP_BUCKETS_COUNT];
} Map;

Map *map_create_(hash_fn hash, eq_fn eq, CpoclMapOptions options) {
    Map *map = options.malloc(sizeof(Map));
    map->opts = options;
    map->hash = hash;
    map->eq = eq;
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        map->buckets[i] = NULL;
    }

    return map;
}

void map_destroy(Map *self) {
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *entry = self->buckets[i];
        while (entry) {
            MapEntry *current = entry;
            entry = entry->next;
            self->opts.free(current);
        }
    }
    self->opts.free(self);
}

size_t map_size(Map *self) {
    size_t size = 0;
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *e = self->buckets[i];
        while (e) {
            size++;
            e = e->next;
        }
    }
    return size;
}

bool map_is_empty(Map *self) {
    return map_size(self) == 0;
}

static bool map_key_eq(Map *self, MapEntry *e, void *key, size_t hash) {
    return e->hash == hash && self->eq(key, e->key);
}

static MapEntry *map_entry_create(Map *self, void *key, void *value, size_t hash) {
    MapEntry *entry = self->opts.malloc(sizeof(MapEntry));
    entry->hash = hash;
    entry->next = NULL;
    entry->key = key;
    entry->value = value;

    return entry;
}

static void map_entry_destroy(Map *self, MapEntry *entry) {
    self->opts.free(entry);
}

void *map_put(Map *self, void *key, void *value) {
    size_t hash = self->hash(key);
    size_t bucket_idx = hash % MAP_BUCKETS_COUNT;

    if (self->buckets[bucket_idx]) {
        MapEntry *e = self->buckets[bucket_idx];
        while (true) {
            if (map_key_eq(self, e, key, hash)) {
                void *old_val = e->value;
                e->value = value;
                return old_val;
            } else if (e->next == NULL) {
                e->next = map_entry_create(self, key, value, hash);
                return NULL;
            }
            e = e->next;
        }
    } else {
        self->buckets[bucket_idx] = map_entry_create(self, key, value, hash);
        return NULL;
    }
}

void *map_get(Map *self, void *key) {
    MapEntry *entry = self->buckets[(self->hash(key) % MAP_BUCKETS_COUNT)];
    while (entry) {
        if (entry->hash == self->hash(key) && self->eq(key, entry->key)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void map_remove(Map *self, void *key) {
    size_t hash = self->hash(key);
    size_t bucket_idx = hash % MAP_BUCKETS_COUNT;

    MapEntry *previous = NULL;
    MapEntry *entry = self->buckets[bucket_idx];
    while (entry) {
        if (entry->hash == self->hash(key) && self->eq(key, entry->key)) {
            if (previous) {
                previous->next = entry->next;
            } else {
                self->buckets[bucket_idx] = entry->next;
            }
            map_entry_destroy(self, entry);
            return;
        }
        previous = entry;
        entry = entry->next;
    }
}

bool map_is_present(Map *self, void *key) {
    return map_get(self, key) != NULL;
}

List *map_get_keys(Map *self) {
    List *keys = list_create();

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *e = self->buckets[i];
        while (e) {
            list_add(keys, e->key);
            e = e->next;
        }
    }

    return keys;
}

List *map_get_values(Map *self) {
    List *keys = list_create();

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *e = self->buckets[i];
        while (e) {
            list_add(keys, e->value);
            e = e->next;
        }
    }

    return keys;
}

List *map_get_entries(Map *self) {
    List *entries = list_create();

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        MapEntry *e = self->buckets[i];
        while (e) {
            Pair *p = self->opts.malloc(sizeof(Pair));
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