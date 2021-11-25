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

#ifndef C_VM_VMARCH_H
#define C_VM_VMARCH_H

#include <stdint.h>

#define WORD_SIZE 4 // bytes
#define ADDR_SIZE 4 // bytes

#if WORD_SIZE == 4
typedef uint32_t word_t;
typedef int32_t sword_t;
#define WHEX  "%08x"
#define WXHEX "0x%08x"
#endif

#if ADDR_SIZE == 4
typedef uint32_t addr_t;
#define AHEX  "%08x"
#define AXHEX "0x%08x"
#endif

#define STACK_LENGTH ((addr_t)1024)
#define STACK_SIZE ((addr_t)(STACK_LENGTH * WORD_SIZE))

#define DEFAULT_RAM_SIZE ((addr_t)(STACK_SIZE + 1024))
#define DEFAULT_REGISTERS_COUNT 32

#define ROM_SIZE ((addr_t)(32 * 1024 * 1024))
#define ROM_ADDRESS ((addr_t)((addr_t) - ROM_SIZE))

#define VIDEO_SCREEN_WIDTH 320
#define VIDEO_SCREEN_HEIGHT 200

word_t from_big_endian(word_t *word);

#endif //C_VM_VMARCH_H
