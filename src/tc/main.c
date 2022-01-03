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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"

void repl();

int main(int argc, char **argv) {
    repl();
    return 0;
}

void str_repeat(char *dst, const char *str, size_t count) {
    if (count < 1) {
        return;
    }
    for (size_t i = count; i > 0; i--) {
        strcat(dst, str);
    }
}

void repl() {
    char buffer[1024];
    while (true) {
        printf("> ");

        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("\n");
            break;
        }

        Lexer *lexer = lexer_create(buffer);
        Parser *parser = parser_create(lexer);
        AstRoot *root = parser_parse(parser);

        if (root == NULL) {
            continue;
        }

        for (size_t i = 0; i < list_size(root->variables); i++) {
            AstNodeVariable *variable = list_get(root->variables, i);
            char *ptr_str = malloc(sizeof(char) * variable->type->ptr + 1);
            str_repeat(ptr_str, "@", variable->type->ptr);
            printf("(%s%s%svar %s : %s%s)\n",
                   variable->pub ? "public " : "",
                   variable->ext ? "external " : "",
                   variable->vol ? "volatile " : "",
                   variable->name->name,
                   ptr_str,
                   variable->type->identifier->name);
            free(ptr_str);
        }
        for (size_t i = 0; i < list_size(root->functions); i++) {
            AstNodeFunction *function = list_get(root->functions, i);

            printf("(%s%sfn %s(",
                   function->pub ? "public " : "",
                   function->ext ? "external " : "",
                   function->name->name
            );

            for (size_t i = 0; i < list_size(function->parameters->parameters); i++) {
                if (i > 0) {
                    printf(", ");
                }

                AstNodeParameter *parameter = list_get(function->parameters->parameters, i);
                char *ptr_str = malloc(sizeof(char) * parameter->type->ptr + 1);
                str_repeat(ptr_str, "@", parameter->type->ptr);

                printf("%s: %s%s",
                       parameter->name->name,
                       ptr_str,
                       parameter->type->identifier->name);
                free(ptr_str);
            }

            char *ptr_str = malloc(sizeof(char) * function->type->ptr + 1);
            str_repeat(ptr_str, "@", function->type->ptr);
            printf(") : %s%s {})\n",
                   ptr_str,
                   function->type->identifier->name);
            free(ptr_str);
        }

        ast_root_destroy(root);
        parser_destroy(parser);
        lexer_destroy(lexer);
    }
}
