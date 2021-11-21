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
#include <printf.h>
#include "opts.h"
#include "vmarch.h"

Options *opts_parse(int argc, char **argv) {
    Options *opts = malloc(sizeof(Options));

    opts->ram_size = DEFAULT_RAM_SIZE;

    struct option long_options[] = {
            {"help", no_argument, &opts->help_flag, 1},
            {"arch", no_argument, &opts->print_arch, 1},
            {"ram",  required_argument, NULL,       'r'},
            {"rom",  required_argument, NULL,       'R'},
            {0, 0,                0,                0},
    };
    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "har:R:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
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
    printf("USAGE:\n    %s [OPTIONS] <rom> <image>\n\n", prog_name);
    printf("OPTIONS:\n");
    printf("    -h, --help                   Prints help information\n");
    printf("    -a, --arch                   Prints arch and exit\n");
    printf("    -r, --ram <RAM>              Amount of ram; default to %ul Bytes\n", DEFAULT_RAM_SIZE);
    printf("    -R, --rom <PATH>             Path to rom to load\n");
    printf("\n");
}