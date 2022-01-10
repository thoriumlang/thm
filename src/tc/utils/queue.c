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
#include <printf.h>
#include <stdio.h>
#include "headers/queue.h"

typedef struct Queue {
    void **items;
    size_t items_count;
    size_t size;
    size_t head;
    size_t tail;
} Queue;

Queue *queue_create(size_t size) {
    Queue *queue = malloc(sizeof(Queue));
    queue->items = malloc(size * sizeof(void *));
    queue->items_count = 0;
    queue->size = size;
    queue->head = 0;
    queue->tail = 0;
    return queue;
}

void queue_destroy(Queue *this) {
    void *item;
    while ((item = queue_dequeue(this)) != NULL) {
        free(item);
    }
    free(this->items);
    free(this);
}

void queue_enqueue(Queue *this, void *item) {
    if (this->items_count == this->size) {
        this->size = this->size * 2;

        void **new_items = malloc(this->size * sizeof(void*));
        size_t new_head = 0;

        void *old_item;
        while ((old_item = queue_dequeue(this)) != NULL) {
            new_items[new_head++] = old_item;
        }

        free(this->items);
        this->items = new_items;
        this->items_count = new_head;
        this->head = this->items_count;
        this->tail = 0;
    }

    this->items[this->head] = item;
    this->head = (this->head + 1) % this->size;
    this->items_count++;
}

void *queue_dequeue(Queue *this) {
    if (this->items_count == 0) {
        return NULL;
    }
    void *item = this->items[this->tail];

    this->items[this->tail] = NULL;
    this->tail = (this->tail + 1) % this->size;
    this->items_count--;

    return item;
}

void *queue_peek(Queue *this, size_t n) {
    if (this->items_count < n + 1) {
        return NULL;
    }
    return this->items[(this->tail + n) % this->size];
}

size_t queue_size(Queue *this) {
    return this->items_count;
}

bool queue_is_empty(Queue *this) {
    return this->items_count == 0;
}