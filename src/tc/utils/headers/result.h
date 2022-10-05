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

#ifndef THM_RESULT_H
#define THM_RESULT_H

#include <stdbool.h>

typedef struct CpoclResult CpoclResult;

CpoclResult *cpocl_result_create_success(void *value);

CpoclResult *cpocl_result_create_error(void *value);

void cpocl_result_destroy(CpoclResult *self);

bool cpocl_result_is_success(CpoclResult *self);

void *cpocl_result_get_value(CpoclResult *self);

void *cpocl_result_unwrap(CpoclResult *self);

void *cpocl_result_get_error(CpoclResult *self);

#ifdef CPOCL_SHORT_NAMES
#define Result CpoclResult
#define result_create_success cpocl_result_create_success
#define result_create_error cpocl_result_create_error
#define result_destroy cpocl_result_destroy
#define result_is_success cpocl_result_is_success
#define result_get_value cpocl_result_get_value
#define result_unwrap cpocl_result_unwrap
#define result_get_error cpocl_result_get_error
#endif

#endif //THM_RESULT_H
