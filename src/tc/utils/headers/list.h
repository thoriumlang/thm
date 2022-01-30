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

#ifndef THM_LIST_H
#define THM_LIST_H

#include <malloc.h>
#include "functions.h"

typedef struct List List;

typedef struct CpoclListOptions {
    void *(*malloc)(size_t);

    void *(*realloc)(void *, size_t);

    void (*free)(void *);
} CpoclListOptions;

#define list_create(...) \
    list_create_((struct CpoclListOptions) { \
        .malloc = malloc,       \
        .realloc = realloc,     \
        .free = free,           \
        __VA_ARGS__ \
    })

List *list_create_(CpoclListOptions options);

void list_destroy(List *self);

void list_add(List *self, void *item);

size_t list_size(List *self);

void *list_get(List *self, size_t index);

void list_foreach(List *self, fn_consumer_closure consumer_closure);

#endif //THM_LIST_H
