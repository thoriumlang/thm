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

typedef void(*fn_biconsumer_t)(void *, void *);

typedef void(*fn_consumer_t)(void *);

typedef struct {
    union {
        fn_consumer_t consumer;
        fn_biconsumer_t biconsumer;
    } fn;
    void *data;
    bool has_data;
} fn_consumer_closure_t;

#define fn_consumer_closure(f, d) \
((fn_consumer_closure_t) { \
    .fn.biconsumer = (fn_biconsumer_t) (f), \
    .data = (d), \
    .has_data = true\
})
#define fn_consumer(f) \
((fn_consumer_closure_t) { \
    .fn.consumer = (fn_consumer_t) (f), \
    .data = NULL, \
    .has_data = false \
})

#endif //THM_FUNCTIONS_H
