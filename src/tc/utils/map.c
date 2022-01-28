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

#ifndef CPOCL_SHORT_NAMES
#define CPOCL_SHORT_NAMES
#endif

#include <string.h>
#include "headers/map.h"
#include "headers/pair.h"

#define MAP_BUCKETS_COUNT 255

typedef struct CpoclMapEntry {
    size_t hash;
    void *key;
    void *value;
    struct CpoclMapEntry *next;
} CpoclMapEntry;

typedef struct CpoclMap {
    CpoclMapOptions opts;
    cpocl_hash_fn hash;
    cpocl_eq_fn eq;
    CpoclMapEntry *buckets[MAP_BUCKETS_COUNT];
} CpoclMap;

CpoclMap *cpocl_map_create_with_opts(cpocl_hash_fn hash, cpocl_eq_fn eq, CpoclMapOptions options) {
    CpoclMap *map = options.malloc(sizeof(CpoclMap));
    map->opts = options;
    map->hash = hash;
    map->eq = eq;
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        map->buckets[i] = NULL;
    }

    return map;
}

void cpocl_map_destroy(CpoclMap *self) {
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        CpoclMapEntry *entry = self->buckets[i];
        while (entry) {
            CpoclMapEntry *current = entry;
            entry = entry->next;
            self->opts.free(current);
        }
    }
    self->opts.free(self);
}

size_t cpocl_map_size(CpoclMap *self) {
    size_t size = 0;
    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        CpoclMapEntry *e = self->buckets[i];
        while (e) {
            size++;
            e = e->next;
        }
    }
    return size;
}

bool cpocl_map_is_empty(CpoclMap *self) {
    return cpocl_map_size(self) == 0;
}

static bool cpocl_map_key_eq(CpoclMap *self, CpoclMapEntry *e, void *key, size_t hash) {
    return e->hash == hash && self->eq(key, e->key);
}

static CpoclMapEntry *cpocl_map_entry_create(CpoclMap *self, void *key, void *value, size_t hash) {
    CpoclMapEntry *entry = self->opts.malloc(sizeof(CpoclMapEntry));
    entry->hash = hash;
    entry->next = NULL;
    entry->key = key;
    entry->value = value;

    return entry;
}

static void cpocl_map_entry_destroy(CpoclMap *self, CpoclMapEntry *entry) {
    self->opts.free(entry);
}

void *cpocl_map_put(CpoclMap *self, void *key, void *value) {
    size_t hash = self->hash(key);
    size_t bucket_idx = hash % MAP_BUCKETS_COUNT;

    if (self->buckets[bucket_idx]) {
        CpoclMapEntry *e = self->buckets[bucket_idx];
        while (true) {
            if (cpocl_map_key_eq(self, e, key, hash)) {
                void *old_val = e->value;
                e->value = value;
                return old_val;
            } else if (e->next == NULL) {
                e->next = cpocl_map_entry_create(self, key, value, hash);
                return NULL;
            }
            e = e->next;
        }
    } else {
        self->buckets[bucket_idx] = cpocl_map_entry_create(self, key, value, hash);
        return NULL;
    }
}

void *cpocl_map_get(CpoclMap *self, void *key) {
    CpoclMapEntry *entry = self->buckets[(self->hash(key) % MAP_BUCKETS_COUNT)];
    while (entry) {
        if (entry->hash == self->hash(key) && self->eq(key, entry->key)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void cpocl_map_remove(CpoclMap *self, void *key) {
    size_t hash = self->hash(key);
    size_t bucket_idx = hash % MAP_BUCKETS_COUNT;

    CpoclMapEntry *previous = NULL;
    CpoclMapEntry *entry = self->buckets[bucket_idx];
    while (entry) {
        if (entry->hash == self->hash(key) && self->eq(key, entry->key)) {
            if (previous) {
                previous->next = entry->next;
            } else {
                self->buckets[bucket_idx] = entry->next;
            }
            cpocl_map_entry_destroy(self, entry);
            return;
        }
        previous = entry;
        entry = entry->next;
    }
}

bool cpocl_map_is_present(CpoclMap *self, void *key) {
    return cpocl_map_get(self, key) != NULL;
}

static inline CpoclList *make_list(CpoclMap *self) {
    return cpocl_list_create_with_opts((CpoclListOptions) {
            .malloc = self->opts.malloc,
            .realloc = self->opts.realloc,
            .free = self->opts.free
    });
}

CpoclList *cpocl_map_get_keys(CpoclMap *self) {
    CpoclList *keys = make_list(self);

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        CpoclMapEntry *e = self->buckets[i];
        while (e) {
            cpocl_list_add(keys, e->key);
            e = e->next;
        }
    }

    return keys;
}

CpoclList *cpocl_map_get_values(CpoclMap *self) {
    CpoclList *keys = make_list(self);

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        CpoclMapEntry *e = self->buckets[i];
        while (e) {
            cpocl_list_add(keys, e->value);
            e = e->next;
        }
    }

    return keys;
}

CpoclList *cpocl_map_get_entries(CpoclMap *self) {
    CpoclList *entries = make_list(self);

    for (size_t i = 0; i < MAP_BUCKETS_COUNT; i++) {
        CpoclMapEntry *e = self->buckets[i];
        while (e) {
            Pair *p = pair_create_with_opts(e->key, e->value, (CpoclPairOptions) {
                    .malloc = self->opts.malloc,
                    .free = self->opts.free
            });
            cpocl_list_add(entries, p);
            e = e->next;
        }
    }

    return entries;
}

size_t cpocl_map_hash_fn_str(void *key) {
    char *c = (char *) key;
    size_t hash = 5381;
    while (*c) {
        hash = ((hash << 5) + hash) ^ *c++;
    }
    return hash;
}

bool cpocl_map_eq_fn_str(void *a, void *b) {
    return strcmp(a, b) == 0;
}
