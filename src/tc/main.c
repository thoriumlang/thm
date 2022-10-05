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
#include "analyser.h"
#include "list.h"
#include "memory.h"
#include "cmdline.h"
#include "result.h"

void repl(void);

MEMORY_GLOBAL()

void compile(char *);

int main(int argc, char **argv) {
    MEMORY_INIT()
    struct gengetopt_args_info args_info;
    if (cmdline_parser(argc, argv, &args_info) != 0) {
        return 1;
    }

    if (args_info.input_given) {
        compile(args_info.input_arg);
    } else {
        repl();
    }

    MEMORY_STATS()
    return 0;
}

long get_file_size(FILE *file) {
    if (fseek(file, 0, SEEK_END) != 0) {
        return -1;
    }
    long sz = ftell(file);
    if (sz == -1) {
        return -1;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        return -1;
    }
    return sz;
}

Result *read_file(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return result_create_error("unable to open file");
    }

    long file_sz = get_file_size(file);
    if (file_sz == -1) {
        return result_create_error("unable to get file size");
    }

    char *file_content = malloc(file_sz + 1);
    if (fread(file_content, 1, file_sz, file) != file_sz) {
        free(file_content);
        return result_create_error("unable to read file content");
    }
    file_content[file_sz] = 0;

    return result_create_success(file_content);
}

void compile(char *filename) {
    Result *file_content_result = read_file(filename);
    if (!result_is_success(file_content_result)) {
        fprintf(stderr, "Unable to read %s: %s\n", filename, (char *) result_get_error(file_content_result));
        exit(1);
    }

    char *file_content = result_unwrap(file_content_result);
    Analyser *analyser = analyzer_create();
    Lexer *lexer = lexer_create(file_content, 1, 1);
    Parser *parser = parser_create(lexer);
    AstRoot *root = parser_parse(parser);

    parser_destroy(parser);
    lexer_destroy(lexer);

    if (root == NULL) {
        fprintf(stderr, "Found syntax errors in %s", filename);
        exit(1);
    }

    analyzer_analyse(analyser, root);

    ast_print(root);
    analyser_dump_symbol_table(analyser);

    analyzer_destroy(analyser);
    ast_root_destroy(root);
    free(file_content);
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
        // todo should we "parser_parse_into(parser, ast) or should we ast_take_*() and ast_append_*()?. With only one
        //  AST we can analyze the whole session and maybe even replace symbols... the while thing might even be useless
        //  as we're not implementing an actual REPL here...
        AstRoot *root = parser_parse(parser);

        parser_destroy(parser);
        lexer_destroy(lexer);

        if (root == NULL) {
            continue;
        }

        ast_print(root);

        list_add(asts, root);
        analyzer_analyse(analyser, root);
    }

    analyser_dump_symbol_table(analyser);

    analyzer_destroy(analyser);
    list_foreach(asts, FN_CONSUMER(ast_root_destroy));
    list_destroy(asts);
}
