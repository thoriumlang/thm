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
#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include "opts.h"
#include "vmarch.h"

Options *opts_parse(int argc, char **argv) {
    Options *opts = malloc(sizeof(Options));

    opts->ram_size = DEFAULT_RAM_SIZE;
    opts->registers = DEFAULT_REGISTERS_COUNT;
    opts->pc = STACK_SIZE;

    struct option long_options[] = {
            {"help",      no_argument, &opts->help_flag,  1},
            {"arch",      no_argument, &opts->print_arch, 1},
            {"ram",       required_argument, NULL,        'r'},
            {"registers", required_argument, NULL,        'R'},
            {"rom",       required_argument, NULL,        'M'},
            {"pc",        required_argument, NULL,        0},
            {0, 0,                     0,                 0},
    };
    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "har:R:M:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                // if the option sets a flag, we don't have anything to do
                if (long_options[option_index].flag != 0) {
                    break;
                }
                if (strcmp(long_options[option_index].name, "pc") == 0) {
                    opts->pc = strtol(optarg, NULL, 10);
                }
                break;
            case 'h':
                opts->help_flag = 1;
                break;
            case 'a':
                opts->print_arch = 1;
                break;
            case 'r':
                opts->ram_size = strtol(optarg, NULL, 10);
                break;
            case 'R':
                opts->registers = (int) strtol(optarg, NULL, 10);
                break;
            case 'M':
                opts->rom = optarg;
                break;
            default:
                abort();
        }
    }

    if (argc > optind) {
        opts->image = argv[optind++];
    }

    return opts;
}

void opts_free(Options *opts) {
    free(opts);
}

void opts_print_help(char *prog_name) {
    printf("USAGE:\n    %s [OPTIONS] <image>\n\n", prog_name);
    printf("OPTIONS:\n");
    printf("    -h, --help                   Prints help information\n");
    printf("    -a, --arch                   Prints arch and exit\n");
    printf("    -r, --ram <RAM>              Amount of ram; default to %ul Bytes\n", DEFAULT_RAM_SIZE);
    printf("    -r, --registers <VAL>        Amount of registers; default to %i, max. 255\n", DEFAULT_REGISTERS_COUNT);
    printf("    -M, --rom <PATH>             Path to rom to load\n");
    printf("        --pc <ADDRESS>           Initial address of PC; defaults to "AXHEX"\n", STACK_SIZE);
    printf("\n");
}
