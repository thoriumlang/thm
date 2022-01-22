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

#include <malloc.h>
#include <string.h>
#include "functions.h"
#include "analyser.h"
#include "symbol_table.h"
#include "symbol.h"
#include "str.h"

typedef struct Analyser {
    SymbolTable *symbols;
    bool error_found;
} Analyser;

#pragma region private

static void print_error(Analyser *this, char *error, AstNodeMetadata metadata) {
    fprintf(stderr, "Error at %d:%d: %s\n", metadata.start_line, metadata.start_column, error);
    this->error_found = true;
}

static void insert_variable_in_symbols_table(AstNodeVariable *node, Analyser *this) {
    if (symbol_table_symbol_exists_in_current_scope(this->symbols, node->name->name)) {
        Symbol *symbol = symbol_table_get(this->symbols, node->name->name);
        char buffer[1024];
        sprintf(buffer, "'%s' already defined at %d:%d.",
                node->name->name,
                ((AstNodeMetadata *) symbol->ast_node)->start_line,
                ((AstNodeMetadata *) symbol->ast_node)->start_column);
        print_error(this, buffer, node->metadata);
        return;
    }

    Symbol *symbol = symbol_create(node->name->name, SYM_VAR, node);
    symbol_table_add(this->symbols, symbol);
}

static void insert_const_in_symbols_table(AstNodeConst *node, Analyser *this) {
    if (symbol_table_symbol_exists_in_current_scope(this->symbols, node->name->name)) {
        Symbol *symbol = symbol_table_get(this->symbols, node->name->name);
        char buffer[1024];
        sprintf(buffer, "'%s' already defined at %d:%d.",
                node->name->name,
                ((AstNodeMetadata *) symbol->ast_node)->start_line,
                ((AstNodeMetadata *) symbol->ast_node)->start_column);
        print_error(this, buffer, node->metadata);
        return;
    }

    Symbol *symbol = symbol_create(node->name->name, SYM_CONST, node);
    symbol_table_add(this->symbols, symbol);
}

static char *mangle_function_name(AstNodeFunction *node) {
    // f(): word -> fn_1f_v
    // f(p1: @word, p2: word) -> fn_1f_P4word_4word
    size_t len = strlen(node->name->name);

    // 'fn_' + len(name) + name + '_'
    size_t buffer_size = 3 + str_lenszt(len) + len + 1;

    if (list_size(node->parameters->parameters) == 0) {
        buffer_size += 2; // for the 'v' parameter and the last '\0'
    } else {
        for (size_t i = 0; i < list_size(node->parameters->parameters); i++) {
            AstNodeParameter *p = (AstNodeParameter *) list_get(node->parameters->parameters, i);
            // 'P's + len(type) + type + '_'
            // last '_' will be converted to a '\0'
            len = strlen(p->type->identifier->name);
            buffer_size += p->type->ptr + str_lenszt(len) + len + 1;
        }
    }

    char *buffer = malloc(buffer_size);
    char *pos = buffer;

    memcpy(pos, "fn_", 4);
    pos += 3;
    len = strlen(node->name->name);
    sprintf(pos, "%lu", len);
    pos += str_lenszt(len);
    memcpy(pos, node->name->name, len);
    pos += len;
    *pos++ = '_';

    if (list_size(node->parameters->parameters) == 0) {
        *pos++ = 'v';
        *pos++ = 0;
    } else {
        for (size_t i = 0; i < list_size(node->parameters->parameters); i++) {
            AstNodeParameter *p = (AstNodeParameter *) list_get(node->parameters->parameters, i);
            str_repeat(pos, "P", p->type->ptr);
            pos += p->type->ptr;
            len = strlen(p->type->identifier->name);
            sprintf(pos, "%lu", len);
            pos += str_lenszt(len);
            memcpy(pos, p->type->identifier->name, len);
            pos += len;
            *pos++ = '_';
        }
        *(pos - 1) = 0;
    }

//    printf("%lu %lu\n", buffer_size, strlen(buffer));

    return buffer;
}

static void insert_function_in_symbols_table(AstNodeFunction *node, Analyser *this) {
    char *name = mangle_function_name(node);
    if (symbol_table_symbol_exists_in_current_scope(this->symbols, name)) {
        Symbol *symbol = symbol_table_get(this->symbols, name);
        char buffer[1024];
        sprintf(buffer, "'%s' already defined at %d:%d.",
                node->name->name,
                ((AstNodeMetadata *) symbol->ast_node)->start_line,
                ((AstNodeMetadata *) symbol->ast_node)->start_column);
        print_error(this, buffer, node->metadata);
        free(name);
        return;
    }

    Symbol *symbol = symbol_create(name, SYM_FN, node);
    symbol_table_add(this->symbols, symbol);
}

#pragma endregion

#pragma region public

Analyser *analyzer_create() {
    Analyser *analyser = malloc(sizeof(Analyser));
    analyser->symbols = symbol_table_create();
    return analyser;
}

void analyzer_destroy(Analyser *this) {
    symbol_table_destroy(this->symbols);
    free(this);
}

bool analyzer_analyse(Analyser *this, AstRoot *root) {
    list_foreach(root->variables, fn_consumer_closure(insert_variable_in_symbols_table, this));
    list_foreach(root->constants, fn_consumer_closure(insert_const_in_symbols_table, this));
    list_foreach(root->functions, fn_consumer_closure(insert_function_in_symbols_table, this));



    return this->error_found;
}

void analyser_dump_symbol_table(Analyser *this) {
    symbol_table_dump(this->symbols);
}

#pragma endregion




