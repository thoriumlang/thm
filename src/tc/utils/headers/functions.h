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

#ifndef THM_FUNCTIONS_H
#define THM_FUNCTIONS_H

#include <stdbool.h>

typedef size_t(*cpocl_hash_fn)(void *);

typedef bool(*cpocl_eq_fn)(void *, void *);

typedef void(*cpocl_biconsumer_fn)(void *, void *);

typedef void(*cpocl_consumer_fn)(void *);

typedef struct {
    union {
        cpocl_consumer_fn consumer;
        cpocl_biconsumer_fn biconsumer;
    } fn;
    void *data;
    bool has_data;
} cpocl_fn_consumer_closure;

#define CPOCL_FN_CONSUMER_CLOSURE(f, d) \
((fn_consumer_closure) { \
    .fn.biconsumer = (cpocl_biconsumer_fn) (f), \
    .data = (d), \
    .has_data = true\
})
#define CPOCL_FN_CONSUMER(f) \
((fn_consumer_closure) { \
    .fn.consumer = (cpocl_consumer_fn) (f), \
    .data = NULL, \
    .has_data = false \
})

#ifdef CPOCL_SHORT_NAMES
#define FN_CONSUMER_CLOSURE CPOCL_FN_CONSUMER_CLOSURE
#define FN_CONSUMER CPOCL_FN_CONSUMER
#define hash_fn cpocl_hash_fn
#define eq_fn cpocl_eq_fn
#define fn_consumer_closure cpocl_fn_consumer_closure
#define biconsumer_fn cpocl_biconsumer_fn
#define consumer_fn cpocl_consumer_fn
#endif

#endif //THM_FUNCTIONS_H
