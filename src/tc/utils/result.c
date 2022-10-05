/**
 * Copyright 2022 Christophe Pollet
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

#include <assert.h>
#include <stdlib.h>
#include "headers/result.h"

#ifdef ASSERT_MOCK
extern void mock_assert(const int result, const char* const expression,
                        const char * const file, const int line);
#undef assert
#define assert(expression) \
    mock_assert((int)(expression), #expression, __FILE__, __LINE__);
#endif

enum CpoclResultKind {
    RK_SUCCESS, RK_ERROR
};

typedef struct CpoclResult {
    enum CpoclResultKind kind;
    void *value;
    void *error;
} CpoclResult;

CpoclResult *cpocl_result_create_success(void *value) {
    Result *result = malloc(sizeof(CpoclResult));
    result->kind = RK_SUCCESS;
    result->value = value;
    result->error = NULL;
    return result;
}

CpoclResult *cpocl_result_create_error(void *error) {
    Result *result = malloc(sizeof(CpoclResult));
    result->kind = RK_ERROR;
    result->value = NULL;
    result->error = error;
    return result;
}

void cpocl_result_destroy(CpoclResult *self) {
    free(self);
}

bool cpocl_result_is_success(CpoclResult *self) {
    return self->kind == RK_SUCCESS;
}

void *cpocl_result_get_value(CpoclResult *self) {
    assert(self->kind == RK_SUCCESS);
    return self->value;
}

void *cpocl_result_unwrap(CpoclResult *self) {
    assert(self->kind == RK_SUCCESS);
    void *value = self->value;
    free(self);
    return value;
}

void *cpocl_result_get_error(CpoclResult *self) {
    assert(self->kind == RK_ERROR);
    return self->error;
}