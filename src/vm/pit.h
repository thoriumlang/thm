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

#ifndef THM_PIT_H
#define THM_PIT_H

#include "pic.h"

typedef struct PIT PIT;
typedef uint32_t microsec_t;

PIT *pit_create(PIC *pic, microsec_t period, interrupt_t interrupt);

void pit_destroy(PIT *this);

void pit_start(PIT *this);

void pit_stop(PIT *this);

#endif //THM_PIT_H
