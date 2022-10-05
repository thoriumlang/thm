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

#include <stddef.h>
#include "headers/list.h"
#include "headers/memory.h"

typedef struct CpoclList {
    void **items;
    size_t items_count;
} CpoclList;

CpoclList *cpocl_list_create(void) {
    CpoclList *list = malloc(sizeof(CpoclList));
    list->items_count = 0;
    list->items = NULL;
    return list;
}

void cpocl_list_destroy(CpoclList *self) {
    if (self->items) {
        free(self->items);
    }
    free(self);
}

void cpocl_list_add(CpoclList *self, void *item) {
    self->items = realloc(self->items, sizeof(void *) * ++self->items_count);
    self->items[self->items_count - 1] = item;
}

size_t cpocl_list_size(CpoclList *self) {
    return self->items_count;
}

void *cpocl_list_get(CpoclList *self, size_t index) {
    if (index >= self->items_count) {
        return NULL;
    }
    return self->items[index];
}

void cpocl_list_foreach(CpoclList *self, fn_consumer_closure consumer_closure) {
    for (size_t i = 0; i < self->items_count; i++) {
        if (consumer_closure.has_data) {
            consumer_closure.fn.biconsumer(self->items[i], consumer_closure.data);
        } else {
            consumer_closure.fn.consumer(self->items[i]);
        }
    }
}
