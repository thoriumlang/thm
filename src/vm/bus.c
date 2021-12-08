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
    addr_t from;
    char *name;
} Zone;

typedef struct Bus {
    uint8_t zones_count;
    Zone *zones;
} Bus;

inline char *memory_mode_to_char(MemMode mode);

inline bool in_zone(Zone *zone, addr_t address);

inline addr_t memory_max_address(Memory *memory, addr_t base);

addr_t translate(Zone *zone, addr_t address);

Zone *find_zone(Bus *bus, addr_t address);

Bus *bus_create() {
    Bus *bus = malloc(sizeof(Bus));
    bus->zones_count = 0;
    bus->zones = NULL;
    return bus;
}

void bus_destroy(Bus *bus) {
    free(bus);
}

BusError bus_memory_attach(Bus *bus, Memory *memory, addr_t from, char *name) {
    for (uint8_t i = 0; i < bus->zones_count; i++) {
        if (in_zone(&bus->zones[i], from)) {
            fprintf(stderr, "Cannot attach %s\n", name);
            return BUS_ERR_ZONE_OUT_OF_ORDER; // todo force error handling / stop vm
        }
    }

    Zone *old_zones = bus->zones;
    bus->zones = malloc(sizeof(*bus->zones) * (bus->zones_count + 1));

    uint8_t i;
    for (i = 0; i < bus->zones_count; i++) {
        if (from < memory_max_address(old_zones[i].memory, old_zones[i].from)) {
            break;
        }
        bus->zones[i] = old_zones[i];
    }

    bus->zones[i].memory = memory;
    bus->zones[i].from = from;
    bus->zones[i].name = name;

    bus->zones_count++;
    for (i = i + 1; i < bus->zones_count; i++) {
        bus->zones[i] = old_zones[i - 1];
    }

    if (old_zones) {
        free(old_zones);
    }

    return BUS_ERR_OK;
}

bool in_zone(Zone *zone, addr_t address) {
    return zone->from <= address && memory_max_address(zone->memory, zone->from) >= address;
}

addr_t memory_max_address(Memory *memory, addr_t base) {
    return memory_size_get(memory) + base - 1;
}

BusError bus_word_read(Bus *bus, addr_t address, word_t *word) {
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
                abort();
        }
    }
    return BUS_ERR_INVALID_ADDRESS;
}

Zone *find_zone(Bus *bus, addr_t address) {
    for (uint8_t i = 0; i < bus->zones_count; i++) {
        Zone *zone = &bus->zones[i];
        if (in_zone(zone, address)) {
            return zone;
        }
    }
    return NULL;
}

BusError bus_word_write(Bus *bus, addr_t address, word_t word) {
#ifdef WITH_MEMORY_LOG_MESSAGE
    printf("BUS Write to "AXHEX"\n", address);
#endif
    Zone *zone = find_zone(bus, address);
    if (zone != NULL) {
#ifdef WITH_MEMORY_LOG_MESSAGE
        printf("  Zone: %s\n", zone->name);
#endif
        switch (memory_word_set(zone->memory, translate(zone, address), word)) {
            case MEM_ERR_OK:
                return BUS_ERR_OK;
            case MEM_ERR_NOT_ALIGNED:
                return BUS_ERR_INVALID_ADDRESS;
            case MEM_ERR_NOT_WRITABLE:
                return BUS_ERR_ILLEGAL_ACCESS;
            case MEM_ERR_OUT_OF_BOUND:
                abort();
        }
    }
    return BUS_ERR_INVALID_ADDRESS;
}

addr_t translate(Zone *zone, addr_t address) {
    return address - zone->from;
}

void bus_state_print(Bus *bus, FILE *file) {
    fprintf(file, "\nBus state\n");
    for (int z = 0; z < bus->zones_count; z++) {
        Memory *memory = bus->zones[z].memory;
        fprintf(file, "  %s zone %02u (%s): "AXHEX" - "AXHEX" (%u bytes)\n",
                memory_mode_to_char(memory_mode_get(memory)),
                z,
                bus->zones[z].name,
                bus->zones[z].from,
                bus->zones[z].from + memory_size_get(memory) - 1,
                memory_size_get(bus->zones[z].memory)
        );
    }
}

JsonElement *bus_json_get(Bus *bus) {
    JsonElement *root = json_array();
    for (int z = 0; z < bus->zones_count; z++) {
        char hex[32];
        Memory *memory = bus->zones[z].memory;
        JsonElement *zone = json_object();
        json_object_put(zone, "name", json_string(bus->zones[z].name));
        json_object_put(zone, "mode", json_string(memory_mode_to_char(memory_mode_get(memory))));
        json_object_put(zone, "from", json_number(bus->zones[z].from));
        sprintf(hex, AXHEX, bus->zones[z].from);
        json_object_put(zone, "from_hex", json_string(hex));
        json_object_put(zone, "to", json_number(bus->zones[z].from + memory_size_get(memory) - 1));
        sprintf(hex, AXHEX, bus->zones[z].from + memory_size_get(memory) - 1);
        json_object_put(zone, "to_hex", json_string(hex));
        json_object_put(zone, "size", json_number(memory_size_get(bus->zones[z].memory)));
        json_array_append(root, zone);
    }
    return root;
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

void bus_dump(Bus *bus, addr_t from, addr_t count, FILE *file) {
    fprintf(file, "\nDump of "AXHEX" - "AXHEX"\n", from, from + count - 1);
    int col = 0;
    for (addr_t address = from; address < from + count - 1; address += WORD_SIZE) {
        word_t word;
        if (bus_word_read(bus, address, &word) == BUS_ERR_OK) {
            if (col % 4 == 0) {

            }
            switch (col % 4) {
                // todo make it dynamic regarding to the WORD_SIZE
                case 0:
                    fprintf(file, "  "AHEX"  %02x %02x %02x %02x", address,
                            (word >> 24) & 0x000000ff,
                            (word >> 16) & 0x000000ff,
                            (word >> 8) & 0x000000ff,
                            word & 0x000000ff
                    );
                    break;
                case 1:
                    fprintf(file, " %02x %02x %02x %02x",
                            (word >> 24) & 0x000000ff,
                            (word >> 16) & 0x000000ff,
                            (word >> 8) & 0x000000ff,
                            word & 0x000000ff
                    );
                    break;
                case 2:
                    fprintf(file, "  %02x %02x %02x %02x",
                            (word >> 24) & 0x000000ff,
                            (word >> 16) & 0x000000ff,
                            (word >> 8) & 0x000000ff,
                            word & 0x000000ff
                    );
                    break;
                case 3:
                    fprintf(file, " %02x %02x %02x %02x\n",
                            (word >> 24) & 0x000000ff,
                            (word >> 16) & 0x000000ff,
                            (word >> 8) & 0x000000ff,
                            word & 0x000000ff
                    );
                    break;
            }
            col++;
        }
    }
    if (col % 4 != 0) {
        printf("\n");
    }
}
