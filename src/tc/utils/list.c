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

#include <stdlib.h>
#include "headers/list.h"

typedef struct List {
    void **items;
    size_t items_count;
} List;

List *list_create() {
    List *list = malloc(sizeof(List));
    list->items_count = 0;
    list->items = NULL;
    return list;
}

void list_destroy(List *this) {
    free(this->items);
    free(this);
}

void list_add(List *this, void *item) {
    this->items = realloc(this->items, sizeof(void *) * ++this->items_count);
    this->items[this->items_count - 1] = item;
}

size_t list_size(List *this) {
    return this->items_count;
}

void *list_get(List *this, size_t index) {
    if (index >= this->items_count) {
        return NULL;
    }
    return this->items[index];
}

void list_foreach(List *this, fn_consumer_closure_t consumer_closure) {
    for (size_t i = 0; i < this->items_count; i++) {
        if (consumer_closure.has_data) {
            consumer_closure.fn.biconsumer(this->items[i], consumer_closure.data);
        } else {
            consumer_closure.fn.consumer(this->items[i]);
        }
    }
}