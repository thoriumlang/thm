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
#include <arpa/inet.h>
#include "vmarch.h"
#include "json.h"

void vmarch_print() {
    printf("Architecture\n");
    printf("  addr_size:             %i\n", ADDR_SIZE);
    printf("  word_size:             %i\n", WORD_SIZE);
    printf("  stack_dept:            %i\n", STACK_LENGTH);
    printf("  stack_size:            %i\n", STACK_SIZE);
    printf("  stack_start:           "AXHEX"\n", 0);
    printf("  stack_end:             "AXHEX"\n", STACK_SIZE - 1);
    printf("  vmeta_start:           "AXHEX"\n", VIDEO_META_ADDRESS);
    printf("  vmeta_end:             "AXHEX"\n", VIDEO_META_ADDRESS + VIDEO_META_SIZE - 1);
    printf("  vbuffer_size:          %i\n", VIDEO_BUFFER_SIZE);
    printf("  vbuf0_start:           "AXHEX"\n", VIDEO_BUFFER_0_ADDRESS);
    printf("  vbuf0_end:             "AXHEX"\n", VIDEO_BUFFER_0_ADDRESS + VIDEO_BUFFER_SIZE - 1);
    printf("  vbuf1_start:           "AXHEX"\n", VIDEO_BUFFER_1_ADDRESS);
    printf("  vbuf1_end:             "AXHEX"\n", VIDEO_BUFFER_1_ADDRESS + VIDEO_BUFFER_SIZE - 1);
    printf("  rom_size:              %i\n", ROM_SIZE);
    printf("  rom_start:             "AXHEX"\n", ROM_ADDRESS);
    printf("  rom_end:               "AXHEX"\n", ROM_ADDRESS + ROM_SIZE - 1);
    printf("  interrupts_count:      %i\n", INTERRUPTS_COUNT);
    printf("  idt start:             "AXHEX"\n", INTERRUPT_DESCRIPTOR_TABLE_ADDRESS);
    printf("  idt end:               "AXHEX"\n", INTERRUPT_MASK_ADDRESS - 1);
    printf("  interrupts mask start: "AXHEX"\n", INTERRUPT_MASK_ADDRESS);
    printf("  interrupts mask end:   "AXHEX"\n", INTERRUPT_MASK_ADDRESS + INTERRUPTS_WORDS_COUNT * WORD_SIZE - 1);
    printf("  keyboard in start:     "AXHEX"\n", KEYBOARD_IN_ADDRESS);
    printf("  keyboard in end:       "AXHEX"\n", KEYBOARD_IN_ADDRESS + KEYBOARD_IN_MEMORY_SIZE - 1);
    printf("  keyboard out start:    "AXHEX"\n", KEYBOARD_OUT_ADDRESS);
    printf("  keyboard out end:      "AXHEX"\n", KEYBOARD_OUT_ADDRESS + KEYBOARD_OUT_MEMORY_SIZE - 1);
}

void vmarch_header_print() {
    printf("// addresses\n");
    printf("$__rom_start = "AXHEX"\n", ROM_ADDRESS);
    printf("$__video_meta = "AXHEX"\n", VIDEO_META_ADDRESS);
    printf("$__video_buffer0 = "AXHEX"\n", VIDEO_BUFFER_0_ADDRESS);
    printf("$__video_buffer1 = "AXHEX"\n", VIDEO_BUFFER_1_ADDRESS);
    printf("$__video_buffer_size = %i\n", VIDEO_BUFFER_SIZE);
    printf("$__idt_start = "AXHEX"\n", INTERRUPT_DESCRIPTOR_TABLE_ADDRESS);
    printf("$__imask_start = "AXHEX"\n", INTERRUPT_MASK_ADDRESS);
    printf("$__keyboard_out = "AXHEX"\n\n", KEYBOARD_OUT_ADDRESS);
    printf("$__keyboard_in = "AXHEX"\n\n", KEYBOARD_IN_ADDRESS);
    printf("// interrupts\n");
    printf("$__int_timer = 0x%02x\n", INT_TIMER);
    printf("$__int_vsync = 0x%02x\n", INT_VSYNC);
    printf("$__int_keyboard = 0x%02x\n", INT_KEYBOARD);
}

JsonElement *vmarch_json_get() {
    char hex[32];
    JsonElement *arch = json_object();
    json_object_put(arch, "addr_size", json_number(ADDR_SIZE));
    json_object_put(arch, "word_size", json_number(WORD_SIZE));
    json_object_put(arch, "stack_depth", json_number(STACK_LENGTH));
    json_object_put(arch, "stack_size", json_number(STACK_SIZE));
    sprintf(hex, AXHEX, 0);
    json_object_put(arch, "stack_start", json_string(hex));
    sprintf(hex, AXHEX, STACK_SIZE - 1);
    json_object_put(arch, "stack_end", json_string(hex));
    sprintf(hex, AXHEX, STACK_SIZE);
    json_object_put(arch, "code_start", json_string(hex));
    json_object_put(arch, "rom_size", json_number(ROM_SIZE));
    sprintf(hex, AXHEX, ROM_ADDRESS);
    json_object_put(arch, "rom_start", json_string(hex));
    sprintf(hex, AXHEX, ROM_ADDRESS + ROM_SIZE - 1);
    json_object_put(arch, "rom_end", json_string(hex));
    json_object_put(arch, "interrupts_count", json_number(INTERRUPTS_COUNT));
    sprintf(hex, AXHEX, INTERRUPT_DESCRIPTOR_TABLE_ADDRESS);
    json_object_put(arch, "idt_start", json_string(hex));
    sprintf(hex, AXHEX, INTERRUPT_MASK_ADDRESS - 1);
    json_object_put(arch, "idt_end", json_string(hex));
    sprintf(hex, AXHEX, INTERRUPT_MASK_ADDRESS);
    json_object_put(arch, "interrupt_mask_start", json_string(hex));
    sprintf(hex, AXHEX, INTERRUPT_MASK_ADDRESS + INTERRUPTS_WORDS_COUNT * WORD_SIZE - 1);
    json_object_put(arch, "interrupt_mask_end", json_string(hex));
    return arch;
}

word_t vtoh(word_t word) {
#if WORD_SIZE == 4
    return ntohl(word);
#endif
}

word_t htov(word_t word) {
#if WORD_SIZE == 4
    return htonl(word);
#endif
}