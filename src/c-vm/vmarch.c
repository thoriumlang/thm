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

#include <stdio.h>
#include "vmarch.h"
#include "json.h"

void arch_print() {
    printf("Architecture\n");
    printf("  addr_size:      %i\n", ADDR_SIZE);
    printf("  word_size:      %i\n", WORD_SIZE);
    printf("  stack_dept:     %i\n", STACK_LENGTH);
    printf("  stack_size:     %i\n", STACK_SIZE);
    printf("  stack_start:    "AXHEX"\n", 0);
    printf("  stack_end:      "AXHEX"\n", STACK_SIZE - 1);
    printf("  vmeta_start:    "AXHEX"\n", VIDEO_META_ADDRESS);
    printf("  vmeta_end:      "AXHEX"\n", VIDEO_META_ADDRESS + VIDEO_META_SIZE - 1);
    printf("  vbuffer_size:   %i\n", VIDEO_BUFFER_SIZE);
    printf("  vbuf1_start:    "AXHEX"\n", VIDEO_BUFFER_1_ADDRESS);
    printf("  vbuf1_end:      "AXHEX"\n", VIDEO_BUFFER_1_ADDRESS + VIDEO_BUFFER_SIZE - 1);
    printf("  vbuf2_start:    "AXHEX"\n", VIDEO_BUFFER_2_ADDRESS);
    printf("  vbuf2_end:      "AXHEX"\n", VIDEO_BUFFER_2_ADDRESS + VIDEO_BUFFER_SIZE - 1);
    printf("  rom_size:       %i\n", ROM_SIZE);
    printf("  rom_start:      "AXHEX"\n", ROM_ADDRESS);
    printf("  rom_end:        "AXHEX"\n", ROM_ADDRESS + ROM_SIZE - 1);
}

JsonElement * arch_json_get(){
    char hex[32];
    JsonElement *arch = json_object();
    json_object_put(arch, "addr_size", json_number(ADDR_SIZE));
    json_object_put(arch, "word_size", json_number(WORD_SIZE));
    json_object_put(arch, "stack_depth", json_number(STACK_LENGTH));
    json_object_put(arch, "stack_size", json_number(STACK_SIZE));
    json_object_put(arch, "stack_start", json_number(0));
    sprintf(hex, AXHEX, 0);
    json_object_put(arch, "stack_start_hex", json_string(hex));
    json_object_put(arch, "stack_end", json_number(STACK_SIZE - 1));
    sprintf(hex, AXHEX, STACK_SIZE - 1);
    json_object_put(arch, "stack_end_hex", json_string(hex));
    json_object_put(arch, "code_start", json_number(STACK_SIZE));
    sprintf(hex, AXHEX, STACK_SIZE);
    json_object_put(arch, "code_start_hex", json_string(hex));
    json_object_put(arch, "rom_size", json_number(ROM_SIZE));
    json_object_put(arch, "rom_start", json_number(ROM_ADDRESS));
    sprintf(hex, AXHEX, ROM_ADDRESS);
    json_object_put(arch, "rom_start", json_string(hex));
    json_object_put(arch, "rom_end", json_number(ROM_ADDRESS + ROM_SIZE - 1));
    sprintf(hex, AXHEX, ROM_ADDRESS + ROM_SIZE - 1);
    json_object_put(arch, "rom_end", json_string(hex));
    return arch;
}

word_t from_big_endian(word_t *word) {
    uint8_t *bytes = (uint8_t *) word;
    return bytes[3] | bytes[2] << 8 | bytes[1] << 16 | bytes[0] << 24;
}