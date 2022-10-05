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

#include <printf.h>
#include <stdbool.h>
#include "headers/memory.h"
#include "headers/queue.h"

typedef struct CpoclQueue {
    void **items;
    size_t items_count;
    size_t size;
    size_t head;
    size_t tail;
} CpoclQueue;

CpoclQueue *cpocl_queue_create(size_t size) {
    CpoclQueue *queue = malloc(sizeof(CpoclQueue));
    queue->items = malloc(size * sizeof(void *));
    queue->items_count = 0;
    queue->size = size;
    queue->head = 0;
    queue->tail = 0;
    return queue;
}

void cpocl_queue_destroy(CpoclQueue *self) {
    free(self->items);
    free(self);
}

void cpocl_queue_enqueue(CpoclQueue *self, void *item) {
    if (self->items_count == self->size) {
        self->size = self->size * 2;

        void **new_items = malloc(self->size * sizeof(void *));
        size_t new_head = 0;

        void *old_item;
        while ((old_item = cpocl_queue_dequeue(self)) != NULL) {
            new_items[new_head++] = old_item;
        }

        free(self->items);
        self->items = new_items;
        self->items_count = new_head;
        self->head = self->items_count;
        self->tail = 0;
    }

    self->items[self->head] = item;
    self->head = (self->head + 1) % self->size;
    self->items_count++;
}

void *cpocl_queue_dequeue(CpoclQueue *self) {
    if (self->items_count == 0) {
        return NULL;
    }
    void *item = self->items[self->tail];

    self->items[self->tail] = NULL;
    self->tail = (self->tail + 1) % self->size;
    self->items_count--;

    return item;
}

void *cpocl_queue_peek(CpoclQueue *self, size_t n) {
    if (self->items_count < n + 1) {
        return NULL;
    }
    return self->items[(self->tail + n) % self->size];
}

size_t cpocl_queue_size(CpoclQueue *self) {
    return self->items_count;
}

bool cpocl_queue_is_empty(CpoclQueue *self) {
    return self->items_count == 0;
}
