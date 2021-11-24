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
#include <ctype.h>
#include "opts.h"
#include "vmarch.h"

int *parse_register_values(int registers, char *str);

Options *opts_parse(int argc, char **argv) {
    Options *opts = malloc(sizeof(Options));

    opts->ram_size = DEFAULT_RAM_SIZE;
    opts->registers = DEFAULT_REGISTERS_COUNT;
    opts->pc = STACK_SIZE;
    char *register_values = NULL;

    struct option long_options[] = {
            {"help",            no_argument, &opts->help_flag,   1},
            {"print-arch",      no_argument, &opts->print_arch,  1},
            {"print-dump",      no_argument, &opts->print_dump,  1},
            {"print-steps",     no_argument, &opts->print_steps, 1},
            {"print-json",      no_argument, &opts->print_json,  1},
            {"registers",       required_argument, NULL,         'r'},
            {"register-values", required_argument, NULL,         0},
            {"ram",             required_argument, NULL,         'R'},
            {"rom",             required_argument, NULL,         'M'},
            {"pc",              required_argument, NULL,         0},
            {0, 0,                           0,                  0},
    };
    int c;
    int option_index = 0;
    while ((c = getopt_long(argc, argv, "hr:R:M:", long_options, &option_index)) != -1) {
        switch (c) {
            case 0:
                // if the option sets a flag, we don't have anything to do
                if (long_options[option_index].flag != 0) {
                    break;
                }
                if (strcmp(long_options[option_index].name, "pc") == 0) {
                    opts->pc = strtol(optarg, NULL, 10);
                } else if (strcmp(long_options[option_index].name, "register-values") == 0) {
                    register_values = optarg;
                }
                break;
            case 'h':
                opts->help_flag = 1;
                break;
            case 'a':
                opts->print_arch = 1;
                break;
            case 'r':
                opts->registers = (int) strtol(optarg, NULL, 10);
                break;
            case 'R':
                opts->ram_size = strtol(optarg, NULL, 10);
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

    opts->register_values = parse_register_values(opts->registers, register_values);

    return opts;
}

/**
 * Parses the input string to an array of register values
 * @param registers the total amount of registers
 * @param str the string to parse; the format is reg:val[,...]
 * @return the array containing the register values (0 for unspecified registers)
 */
int *parse_register_values(int registers, char *str) {
    int *parsed_values = calloc(registers, sizeof(int));
    if (str == NULL) goto parse_error;

    int mode = 0;
    int reg = 0;
    for (int i = 0; str[i] != 0; i++) {
        char c = str[i];
        switch (mode) {
            case 0: // we are parsing a register
                if (c == ':') {
                    if (reg >= registers) goto register_error;
                    if (str[i + 1] == 0) goto parse_error;
                    mode = 1;
                    continue;
                } else if (!isdigit(c)) goto parse_error;
                reg = reg * 10 + c - '0';
                break;
            case 1: // we are parsing a value
                if (c == ',') {
                    if (str[i + 1] == 0) goto parse_error;
                    mode = 0;
                    reg = 0;
                    continue;
                } else if (!isdigit(c)) goto parse_error;
                parsed_values[reg] = parsed_values[reg]  * 10 + c - '0';
                break;
            default:
                abort();
        }
    }
    if (mode != 1) goto parse_error;

    return parsed_values;

    parse_error:
    fprintf(stderr, "Cannot parse `%s` as a valid --register-values <VAL>\n", str);
    memset(parsed_values, 0, registers * sizeof(int));
    return parsed_values;

    register_error:
    fprintf(stderr, "Register `%i` is not a valid register for --register-values <VAL>\n", reg);
    memset(parsed_values, 0, registers * sizeof(int));
    return parsed_values;
}

void opts_free(Options *opts) {
    free(opts);
}

void opts_print_help(char *prog_name) {
    printf("USAGE:\n    %s [OPTIONS] <image>\n\n", prog_name);
    printf("OPTIONS:\n");
    printf("    -h, --help                   Prints help information\n");
    printf("        --print-arch             Prints arch when starting\n");
    printf("        --print-dump             Prints when dump before and after execution\n");
    printf("        --print-steps            Prints steps\n");
    printf("        --print-json             Prints json after execution\n");
    printf("    -r, --registers <VAL>        Amount of registers; default to %i, max. 255\n", DEFAULT_REGISTERS_COUNT);
    printf("    -r, --register-values <VAL>  Initial register values\n"
           "                                 <VAL> format: `<reg>:<val>[,...]`, <reg> starts at 0\n");
    printf("    -R, --ram <RAM>              Amount of ram; default to %ul Bytes\n", DEFAULT_RAM_SIZE);
    printf("    -M, --rom <PATH>             Path to rom to load\n");
    printf("        --pc <ADDRESS>           Initial address of PC; defaults to "AXHEX"\n", STACK_SIZE);
    printf("\n");
}
