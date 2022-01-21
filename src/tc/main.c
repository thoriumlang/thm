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
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "str.h"

void repl();

int main(int argc, char **argv) {
    repl();
    return 0;
}

void repl() {
    char buffer[1024];
    while (true) {
        printf("> ");

        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("\n");
            break;
        }

        Lexer *lexer = lexer_create(buffer, line++, 1);
        Parser *parser = parser_create(lexer);
        AstRoot *root = parser_parse(parser);

        parser_destroy(parser);
        lexer_destroy(lexer);

        if (root == NULL) {
            continue;
        }

        list_foreach(root->variables, fn_consumer(ast_node_variable_print));
        list_foreach(root->constants, fn_consumer(ast_node_const_print));
        list_foreach(root->functions, fn_consumer(ast_node_function_print));

        ast_root_destroy(root);
    }
}
