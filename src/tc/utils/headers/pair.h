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

#ifndef THM_PAIR_H
#define THM_PAIR_H

typedef struct CpoclPair CpoclPair;

CpoclPair *cpocl_pair_create(void *a, void *b);

void cpocl_pair_destroy(CpoclPair *self);

void cpocl_pair_set_a(CpoclPair *self, void *ptr);

#define cpocl_pair_set_first cpocl_pair_set_a
#define cpocl_pair_set_left cpocl_pair_set_a
#define cpocl_pair_set_key cpocl_pair_set_a

void *cpocl_pair_get_a(CpoclPair *self);

#define cpocl_pair_get_first cpocl_pair_get_a
#define cpocl_pair_get_left cpocl_pair_get_a
#define cpocl_pair_get_key cpocl_pair_get_a

void cpocl_pair_set_b(CpoclPair *self, void *ptr);

#define cpocl_pair_set_second cpocl_pair_set_b
#define cpocl_pair_set_right cpocl_pair_set_b
#define cpocl_pair_set_value cpocl_pair_set_b

void *cpocl_pair_get_b(CpoclPair *self);

#define cpocl_pair_get_second cpocl_pair_get_b
#define cpocl_pair_get_right cpocl_pair_get_b
#define cpocl_pair_get_value cpocl_pair_get_b

#ifdef CPOCL_SHORT_NAMES
#define Pair CpoclPair
#define pair_create cpocl_pair_create
#define pair_create cpocl_pair_create
#define pair_destroy cpocl_pair_destroy

#define pair_set_a cpocl_pair_set_a
#define pair_set_first cpocl_pair_set_first
#define pair_set_left cpocl_pair_set_left
#define pair_set_key cpocl_pair_set_key

#define pair_get_a cpocl_pair_get_a
#define pair_get_first cpocl_pair_get_first
#define pair_get_left cpocl_pair_get_left
#define pair_get_key cpocl_pair_get_key

#define pair_set_b cpocl_pair_set_b
#define pair_set_second cpocl_pair_set_second
#define pair_set_right cpocl_pair_set_right
#define pair_set_value cpocl_pair_set_value

#define pair_get_b cpocl_pair_get_b
#define pair_get_second cpocl_pair_get_second
#define pair_get_right cpocl_pair_get_right
#define pair_get_value cpocl_pair_get_value
#endif

#endif //THM_PAIR_H
