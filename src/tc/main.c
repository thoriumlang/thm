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
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "str.h"
#include "analyser.h"
#include "list.h"
#include "memory.h"

void repl(void);

MEMORY_GLOBAL()

int main(int argc, char **argv) {
    MEMORY_INIT()
    repl();
    MEMORY_STATS()
    return 0;
}

void repl(void) {
    int line = 1;
    char buffer[1024];
    List *asts = list_create();

    Analyser *analyser = analyzer_create();

    while (true) {
        printf("> ");

        if (!fgets(buffer, sizeof(buffer), stdin)) {
            printf("\n");
            break;
        }

        if (strcmp(buffer, "exit\n") == 0) {
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

        list_foreach(root->variables, FN_CONSUMER(ast_node_variable_print));
        list_foreach(root->constants, FN_CONSUMER(ast_node_const_print));
        list_foreach(root->functions, FN_CONSUMER(ast_node_function_print));

        list_add(asts, root);
        analyzer_analyse(analyser, root);
    }

    analyser_dump_symbol_table(analyser);

    analyzer_destroy(analyser);
    list_foreach(asts, FN_CONSUMER(ast_root_destroy));
    list_destroy(asts);
}
