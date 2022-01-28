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

typedef struct CpoclList CpoclList;

typedef struct CpoclListOptions {
    void *(*malloc)(size_t);

    void *(*realloc)(void *, size_t);

    void (*free)(void *);
} CpoclListOptions;

#define cpocl_list_create(...) \
    cpocl_list_create_with_opts((struct CpoclListOptions) { \
        .malloc = malloc,       \
        .realloc = realloc,     \
        .free = free,           \
        __VA_ARGS__             \
    })

CpoclList *cpocl_list_create_with_opts(CpoclListOptions options);

void cpocl_list_destroy(CpoclList *self);

void cpocl_list_add(CpoclList *self, void *item);

size_t cpocl_list_size(CpoclList *self);

void *cpocl_list_get(CpoclList *self, size_t index);

void cpocl_list_foreach(CpoclList *self, fn_consumer_closure consumer_closure);

#ifdef CPOCL_SHORT_NAMES
#define List CpoclList
#define ListOptions CpoclListOptions
#define list_create cpocl_list_create
#define list_create_with_opts cpocl_list_create_with_opts
#define list_destroy cpocl_list_destroy
#define list_add cpocl_list_add
#define list_size cpocl_list_size
#define list_get cpocl_list_get
#define list_foreach cpocl_list_foreach
#endif

#endif //THM_LIST_H
