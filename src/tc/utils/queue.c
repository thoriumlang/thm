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

#include <printf.h>
#include "headers/queue.h"

typedef struct Queue {
    CpoclQueueOptions opts;

    void **items;
    size_t items_count;
    size_t size;
    size_t head;
    size_t tail;
} Queue;

Queue *queue_create_(size_t size, CpoclQueueOptions options) {
    Queue *queue = options.malloc(sizeof(Queue));
    queue->opts = options;
    queue->items = options.malloc(size * sizeof(void *));
    queue->items_count = 0;
    queue->size = size;
    queue->head = 0;
    queue->tail = 0;
    return queue;
}

void queue_destroy(Queue *self) {
    self->opts.free(self->items);
    self->opts.free(self);
}

void queue_enqueue(Queue *self, void *item) {
    if (self->items_count == self->size) {
        self->size = self->size * 2;

        void **new_items = malloc(self->size * sizeof(void *));
        size_t new_head = 0;

        void *old_item;
        while ((old_item = queue_dequeue(self)) != NULL) {
            new_items[new_head++] = old_item;
        }

        self->opts.free(self->items);
        self->items = new_items;
        self->items_count = new_head;
        self->head = self->items_count;
        self->tail = 0;
    }

    self->items[self->head] = item;
    self->head = (self->head + 1) % self->size;
    self->items_count++;
}

void *queue_dequeue(Queue *self) {
    if (self->items_count == 0) {
        return NULL;
    }
    void *item = self->items[self->tail];

    self->items[self->tail] = NULL;
    self->tail = (self->tail + 1) % self->size;
    self->items_count--;

    return item;
}

void *queue_peek(Queue *self, size_t n) {
    if (self->items_count < n + 1) {
        return NULL;
    }
    return self->items[(self->tail + n) % self->size];
}

size_t queue_size(Queue *self) {
    return self->items_count;
}

bool queue_is_empty(Queue *self) {
    return self->items_count == 0;
}