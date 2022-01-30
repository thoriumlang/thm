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

typedef void(*biconsumer_fn)(void *, void *);

typedef void(*consumer_fn)(void *);

typedef struct {
    union {
        consumer_fn consumer;
        biconsumer_fn biconsumer;
    } fn;
    void *data;
    bool has_data;
} fn_consumer_closure;

#define FN_CONSUMER_CLOSURE(f, d) \
((fn_consumer_closure) { \
    .fn.biconsumer = (biconsumer_fn) (f), \
    .data = (d), \
    .has_data = true\
})
#define FN_CONSUMER(f) \
((fn_consumer_closure) { \
    .fn.consumer = (consumer_fn) (f), \
    .data = NULL, \
    .has_data = false \
})

#endif //THM_FUNCTIONS_H
