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

#ifndef C_VM_OPTS_H
#define C_VM_OPTS_H

#include "vmarch.h"

typedef struct {
    char *rom;
    char *image;
    addr_sz ram_size;
    int registers;
    int help_flag;
    int print_arch;
} Options;

Options *opts_parse(int argc, char **argv);

void opts_free(Options *opts);

void opts_print_help(char *prog_name);


#endif //C_VM_OPTS_H
