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

#ifndef CPOCL_SHORT_NAMES
#define CPOCL_SHORT_NAMES
#endif

#include "headers/pair.h"

typedef struct CpoclPair {
    CpoclPairOptions opts;
    void *a;
    void *b;
} CpoclPair;

CpoclPair *cpocl_pair_create_with_opts(void *a, void *b, CpoclPairOptions options) {
    CpoclPair *pair = options.malloc(sizeof(CpoclPair));
    pair->opts = options;
    pair->a = a;
    pair->b = b;
    return pair;
}

void cpocl_pair_destroy(CpoclPair *self) {
    self->opts.free(self);
}

void cpocl_pair_set_a(CpoclPair *self, void *ptr) {
    self->a = ptr;
}

void *cpocl_pair_get_a(CpoclPair *self) {
    return self->a;
}

void cpocl_pair_set_b(CpoclPair *self, void *ptr) {
    self->b = ptr;
}

void *cpocl_pair_get_b(CpoclPair *self) {
    return self->b;
}
