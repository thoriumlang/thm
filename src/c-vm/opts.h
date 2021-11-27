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

typedef enum {
    OPT_VIDEO_MODE_NONE, OPT_VIDEO_MODE_MASTER, OPT_VIDEO_MODE_SLAVE
} OptsVideMode;

typedef struct {
    char *rom;
    char *image;
    addr_t ram_size;
    addr_t pc;
    int registers;
    int *register_values;
    int help_flag;
    int print_steps;
    int print_arch;
    int print_dump;
    int print_json;
    OptsVideMode video;
} Options;

Options *opts_parse(int argc, char **argv);

void opts_free(Options *opts);

void opts_print_help(char *prog_name);


#endif //C_VM_OPTS_H
