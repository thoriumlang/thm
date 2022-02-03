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
#include <stdlib.h>

typedef struct CpoclQueue CpoclQueue;

typedef struct CpoclQueueOptions {
    void *(*malloc)(size_t);

    void (*free)(void *);
} CpoclQueueOptions;

#define cpocl_queue_create(size, ...) \
    cpocl_queue_create_with_opts((size), (struct CpoclQueueOptions) { \
        .malloc = malloc,       \
        .free = free,           \
        __VA_ARGS__             \
    })

CpoclQueue *cpocl_queue_create_with_opts(size_t size, CpoclQueueOptions options);

void cpocl_queue_destroy(CpoclQueue *self);

void cpocl_queue_enqueue(CpoclQueue *self, void *item);

void *cpocl_queue_dequeue(CpoclQueue *self);

void *cpocl_queue_peek(CpoclQueue *self, size_t n);

size_t cpocl_queue_size(CpoclQueue *self);

bool cpocl_queue_is_empty(CpoclQueue *self);

#ifdef CPOCL_SHORT_NAMES
#define Queue CpoclQueue
#define QueueOptions CpoclQueueOptions
#define queue_create cpocl_queue_create
#define queue_create_with_opts cpocl_queue_create_with_opts
#define queue_destroy cpocl_queue_destroy
#define queue_enqueue cpocl_queue_enqueue
#define queue_dequeue cpocl_queue_dequeue
#define queue_peek cpocl_queue_peek
#define queue_size cpocl_queue_size
#define queue_is_empty cpocl_queue_is_empty
#endif

#endif //THM_QUEUE_H
