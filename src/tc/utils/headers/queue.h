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

typedef struct Queue Queue;

Queue *queue_create(size_t size);

void queue_destroy(Queue *this);

void queue_enqueue(Queue *this, void *item);

void *queue_dequeue(Queue *this);

void *queue_peek(Queue *this, size_t n);

size_t queue_size(Queue *this);

bool queue_is_empty(Queue *this);

#endif //THM_QUEUE_H
