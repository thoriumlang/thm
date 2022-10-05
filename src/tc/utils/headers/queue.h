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

typedef struct CpoclQueue CpoclQueue;

CpoclQueue *cpocl_queue_create(size_t size);

void cpocl_queue_destroy(CpoclQueue *self);

void cpocl_queue_enqueue(CpoclQueue *self, void *item);

void *cpocl_queue_dequeue(CpoclQueue *self);

void *cpocl_queue_peek(CpoclQueue *self, size_t n);

size_t cpocl_queue_size(CpoclQueue *self);

bool cpocl_queue_is_empty(CpoclQueue *self);

#ifdef CPOCL_SHORT_NAMES
#define Queue CpoclQueue
#define queue_create cpocl_queue_create
#define queue_create cpocl_queue_create
#define queue_destroy cpocl_queue_destroy
#define queue_enqueue cpocl_queue_enqueue
#define queue_dequeue cpocl_queue_dequeue
#define queue_peek cpocl_queue_peek
#define queue_size cpocl_queue_size
#define queue_is_empty cpocl_queue_is_empty
#endif

#endif //THM_QUEUE_H
