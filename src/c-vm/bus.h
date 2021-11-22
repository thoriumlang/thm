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

#ifndef C_VM_BUS_H
#define C_VM_BUS_H

#include <stdio.h>
#include "vmarch.h"
#include "memory.h"
#include "json.h"

typedef struct Bus Bus;

typedef enum {
    BUS_ERR_OK = 1,
    BUS_ERR_ZONE_OUT_OF_ORDER,
    BUS_ERR_INVALID_ADDRESS,
    BUS_ERR_ILLEGAL_ACCESS
} BusError;

Bus *bus_create();

void bus_destroy(Bus *bus);

BusError bus_memory_attach(Bus *bus, Memory *memory, addr_t from, char *name);

BusError bus_word_read(Bus *bus, addr_t address, word_t *word);

BusError bus_word_write(Bus *bus, addr_t address, word_t word);

void bus_state_print(Bus *bus, FILE *file);

JsonElement *bus_state_to_json(Bus *bus);

void bus_dump(Bus *bus, addr_t from, addr_t count, FILE *file);

#endif //C_VM_BUS_H
