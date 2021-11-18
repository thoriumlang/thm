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

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "vmarch.h"
#include "memory.h"
#include "bus.h"

typedef struct {
    Memory *memory;
    addr_sz from;
    char *name;
} Zone;

typedef struct Bus {
    uint8_t zones_count;
    Zone *zones;
} Bus;

inline char *memory_mode_to_char(MemMode mode);

inline bool in_zone(Zone *zone, addr_sz address);

inline addr_sz memory_max_address(Memory *memory, addr_sz base);

addr_sz translate(Zone *zone, addr_sz address);

Zone *find_zone(Bus *bus, addr_sz address);

Bus *bus_create() {
    Bus *bus = malloc(sizeof(Bus));
    bus->zones_count = 0;
    bus->zones = NULL;
    return bus;
}

void bus_destroy(Bus *bus) {
    free(bus);
}

BusError bus_memory_attach(Bus *bus, Memory *memory, addr_sz from, char *name) {
    for (uint8_t i = 0; i < bus->zones_count; i++) {
        if (in_zone(&bus->zones[i], from)) {
            return BUS_ERR_ZONE_OUT_OF_ORDER;
        }
    }

    bus->zones = realloc(bus->zones, sizeof(*bus->zones) * (bus->zones_count + 1));
    Zone *new_zone = &bus->zones[bus->zones_count];
    new_zone->memory = memory;
    new_zone->from = from;
    new_zone->name = name;
    bus->zones_count++;
    return BUS_ERR_OK;
}

bool in_zone(Zone *zone, addr_sz address) {
    return zone->from <= address && memory_max_address(zone->memory, zone->from) >= address;
}

addr_sz memory_max_address(Memory *memory, addr_sz base) {
    return memory_size_get(memory) + base - 1;
}

BusError bus_word_read(Bus *bus, addr_sz address, word_sz *word) {
    Zone *zone = find_zone(bus, address);
    if (zone != NULL) {
        switch (memory_word_get(zone->memory, translate(zone, address), word)) {
            case MEM_ERR_OK:
                return BUS_ERR_OK;
            case MEM_ERR_NOT_ALIGNED:
                return BUS_ERR_INVALID_ADDRESS;
            case MEM_ERR_NOT_WRITABLE:
                return BUS_ERR_ILLEGAL_ACCESS;
            case MEM_ERR_OUT_OF_BOUND:
                assert(false);
        }
    }
    return BUS_ERR_INVALID_ADDRESS;
}

Zone *find_zone(Bus *bus, addr_sz address) {
    for (uint8_t i = 0; i < bus->zones_count; i++) {
        Zone *zone = &bus->zones[i];
        if (in_zone(zone, address)) {
            return zone;
        }
    }
    return NULL;
}

BusError bus_word_write(Bus *bus, addr_sz address, word_sz word) {
    Zone *zone = find_zone(bus, address);
    if (zone != NULL) {
        switch (memory_word_set(zone->memory, translate(zone, address), word)) {
            case MEM_ERR_OK:
                return BUS_ERR_OK;
            case MEM_ERR_NOT_ALIGNED:
                return BUS_ERR_INVALID_ADDRESS;
            case MEM_ERR_NOT_WRITABLE:
                return BUS_ERR_ILLEGAL_ACCESS;
            case MEM_ERR_OUT_OF_BOUND:
                assert(false);
        }
    }
    return BUS_ERR_INVALID_ADDRESS;
}

addr_sz translate(Zone *zone, addr_sz address) {
    return address - zone->from;
}

void bus_print_state(FILE *file, Bus *bus) {
    for (int z = 0; z < bus->zones_count; z++) {
        Memory *memory = bus->zones[z].memory;
        fprintf(file, "%s zone %02u (%s): 0x%08x - 0x%08x (%u bytes)\n",
                memory_mode_to_char(memory_mode_get(memory)),
                z,
                bus->zones[z].name,
                bus->zones[z].from,
                bus->zones[z].from + memory_size_get(memory) - 1,
                memory_size_get(bus->zones[z].memory)
        );
    }
}

char *memory_mode_to_char(MemMode mode) {
    switch (mode) {
        case MEM_MODE_R:
            return "RO";
        case MEM_MODE_RW:
            return "RW";
        default:
            fprintf(stderr, "Unsupported memory mode: %u", mode);
            exit(1);
    }
}