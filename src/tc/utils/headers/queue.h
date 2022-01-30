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

#ifndef THM_QUEUE_H
#define THM_QUEUE_H

#include <stdbool.h>
#include <stddef.h>
#include <malloc.h>

typedef struct Queue Queue;

typedef struct CpoclQueueOptions {
    void *(*malloc)(size_t);

    void (*free)(void *);
} CpoclQueueOptions;

#define queue_create(size, ...) \
    queue_create_((size), (struct CpoclQueueOptions) { \
        .malloc = malloc,       \
        .free = free,           \
        __VA_ARGS__ \
    })

Queue *queue_create_(size_t size, CpoclQueueOptions options);

void queue_destroy(Queue *self);

void queue_enqueue(Queue *self, void *item);

void *queue_dequeue(Queue *self);

void *queue_peek(Queue *self, size_t n);

size_t queue_size(Queue *self);

bool queue_is_empty(Queue *self);

#endif //THM_QUEUE_H
