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
#include <string.h>
#include "debugger.h"
#include "debugger_lexer.h"
#include "cpu_internal.h"

typedef struct cpu_debugger_data {
    Lexer *lexer;
    char *previous_line;
    size_t previous_len;
} cpu_debugger_data;

int cpu_debugger_trap_handler(struct CPU *cpu, word_t word, Bus *bus, void *data);

CpuDebugger *cpu_debugger_create() {
    cpu_debugger_data *data = malloc(sizeof(cpu_debugger_data));
    data->lexer = lexer_create();
    data->previous_line = NULL;
    data->previous_len = 0;

    CpuDebugger *debugger = malloc(sizeof(CpuDebugger));
    debugger->call = &cpu_debugger_trap_handler;
    debugger->data = data;

    return debugger;
}

typedef enum {
    CMD_HELP,
    CMD_CONTINUE,
    CMD_STEP,
    CMD_QUIT,
    CMD_PRINT_REGISTER,
    CMD_PRINT_MEMORY,
    CMD_UNKNOWN,
} Commands;

Token *cpu_debugger_next_token(Token *token, Lexer *lexer) {
    if (token != NULL) {
        free(token);
    }
    token = lexer_next(lexer);
    return token;
}

Commands cpu_debugger_decode_command(char *identifier) {
    if (strcmp(identifier, "h") == 0 || strcmp(identifier, "help") == 0) {
        return CMD_HELP;
    }
    if (strcmp(identifier, "c") == 0 || strcmp(identifier, "continue") == 0) {
        return CMD_CONTINUE;
    }
    if (strcmp(identifier, "s") == 0 || strcmp(identifier, "step") == 0) {
        return CMD_STEP;
    }
    if (strcmp(identifier, "q") == 0 || strcmp(identifier, "quit") == 0) {
        return CMD_QUIT;
    }
    if (strcmp(identifier, "r") == 0 || strcmp(identifier, "reg") == 0) {
        return CMD_PRINT_REGISTER;
    }
    if (strcmp(identifier, "m") == 0 || strcmp(identifier, "mem") == 0) {
        return CMD_PRINT_MEMORY;
    }
    return CMD_UNKNOWN;
}

#define RET(v) { \
    free(token);\
    free(line);\
    return (v);\
}

char *cpu_debugger_read_line(cpu_debugger_data *debugger_data) {
    size_t len;
    char *line = NULL;

    printf("> ");
    if (getline(&line, &len, stdin) == 1) {
        if (debugger_data->previous_line == NULL || strlen(debugger_data->previous_line) == 1) {
            return NULL;
        }
        len = debugger_data->previous_len;
        line = realloc(line, len);
        strcpy(line, debugger_data->previous_line);
    } else {
        if (debugger_data->previous_len < len) {
            debugger_data->previous_line = realloc(debugger_data->previous_line, len);
        }
        debugger_data->previous_len = len;
        strcpy(debugger_data->previous_line, line);
    }
    return line;
}

void cpu_debugger_print_register(CPU *cpu, cpu_debugger_data *debugger_data) {
    Token *token = cpu_debugger_next_token(NULL, debugger_data->lexer);

    if (token->type != TOKEN_NUMBER) {
        printf("  Expected <number>\n");
        free(token);
        return;
    }

    word_t value;
    if (cpu_register_get(cpu, token->token.number, &value) != CPU_ERR_OK) {
        printf("  Cannot read register %i\n", token->token.number);
    } else {
        printf("  r%i = "WXHEX"\n", token->token.number, value);
    }
    free(token);
}

void cpu_debugger_print_memory(Bus *bus, cpu_debugger_data *debugger_data) {
    Token *token = cpu_debugger_next_token(NULL, debugger_data->lexer);

    if (token->type != TOKEN_NUMBER) {
        printf("  Expected <number>\n");
        free(token);
        return;
    }

    addr_t from = (addr_t) token->token.number;
    addr_t to = from + WORD_SIZE;

    switch (cpu_debugger_next_token(token, debugger_data->lexer)->type) {
        case TOKEN_EOF:
            break;
        case TOKEN_COMMA: {
            if (cpu_debugger_next_token(token, debugger_data->lexer)->type != TOKEN_NUMBER) {
                printf("  Expected <number>\n");
                return;
            }
            to = from + token->token.number;
            break;
        }
        case TOKEN_COLON: {
            if (cpu_debugger_next_token(token, debugger_data->lexer)->type != TOKEN_NUMBER) {
                printf("  Expected <number>\n");
                return;
            }
            to = token->token.number;
            break;
        }
        default:
            printf("  Expected <eol>, <:> or <,>\n");
            return;
    }

    free(token);
    bus_dump(bus, from, to - from, stdout);
}

int cpu_debugger_trap_handler(struct CPU *cpu, word_t word, Bus *bus, void *data) {
    cpu_debugger_data *debugger_data = (cpu_debugger_data *) data;

    while (true) {
        printf("\n");
        printf("  cs="AXHEX"      sp="AXHEX"\n", cpu->cs, cpu->sp);
        printf("* %s\n", cpu_instruction_to_string(cpu, word));

        char *line;
        if ((line = cpu_debugger_read_line(debugger_data)) == NULL) {
            continue;
        }

        lexer_reset(debugger_data->lexer, line);
        Token *token = cpu_debugger_next_token(NULL, debugger_data->lexer);
        switch (token->type) {
            case TOKEN_IDENTIFIER:
                switch (cpu_debugger_decode_command(token->token.string)) {
                    case CMD_HELP:
                        printf("  [c]ontinue\n"
                               "  [s]tep\n"
                               "  [r]eg <number>\n"
                               "  [m]em <number> [, <number>]\n"
                               "  [q]uit\n"
                        );
                        break;
                    case CMD_CONTINUE:
                        cpu->debug.trap = 0;
                        RET(0)
                    case CMD_STEP: RET(1)
                    case CMD_QUIT: RET(0)
                    case CMD_PRINT_REGISTER:
                        cpu_debugger_print_register(cpu, debugger_data);
                        break;
                    case CMD_PRINT_MEMORY:
                        cpu_debugger_print_memory(bus, debugger_data);
                        break;
                    case CMD_UNKNOWN:
                        printf("Unknown command `%s`; [h] for help\n", token->token.string);
                        break;
                }
                break;
            default:
                printf("Expected string, got `%s`. See [h] help\n", token->token.string);
        }
        free(token);
    }
}

void cpu_debugger_destroy(CpuDebugger *this) {
    free(this);
}
