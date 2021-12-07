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
#include <stdio.h>
#include "pic.h"
#include "keyboard.h"

typedef struct Keyboard {
    PIC *pic;
    Memory *memory;
} Keyboard;

Keyboard *keyboard_create(PIC *pic) {
    Keyboard *keyboard = malloc(sizeof(Keyboard));
    keyboard->pic = pic;
    keyboard->memory = memory_create(KEYBOARD_MEMORY_SIZE, MEM_MODE_RW);
    return keyboard;
}

void keyboard_destroy(Keyboard *this) {
    free(this->memory);
    free(this);
}

Memory *keyboard_memory_get(Keyboard *this) {
    return this->memory;
}

void keyboard_key_pressed(Keyboard *this, keyboard_key key) {
    word_t word = (key << 8) | 1;
    // printf("keyboard: "WXHEX"\n", word);
    memory_word_set(this->memory, 0, word);
    pic_interrupt_trigger(this->pic, INT_KEYBOARD);
}

void keyboard_key_released(Keyboard *this, keyboard_key key) {
    word_t word = (key << 8);
    // printf("keyboard: "WXHEX"\n", word);
    memory_word_set(this->memory, 0, word);
    pic_interrupt_trigger(this->pic, INT_KEYBOARD);
}