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

typedef uint32_t addr_sz;
typedef uint32_t word_sz;

#define WORD_SIZE sizeof(word_sz)

#define DEFAULT_RAM_SIZE ((addr_sz)(STACK_SIZE + 1024))

#define STACK_LENGTH ((addr_sz)1024)
#define STACK_SIZE ((addr_sz)(STACK_LENGTH * WORD_SIZE))

#define ROM_SIZE ((addr_sz)(32 * 1024 * 1024))
#define ROM_ADDRESS ((addr_sz)((addr_sz) - ROM_SIZE))

#endif //C_VM_VMARCH_H
