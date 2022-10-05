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

#include <string.h>
#include <stdio.h>
#include "functions.h"
#include "analyser.h"
#include "symbol_table.h"
#include "symbol.h"
#include "str.h"
#include "memory.h"

typedef struct Analyser {
    SymbolTable *symbols;
    bool error_found;
} Analyser;

#pragma region private

static void print_error(Analyser *self, char *error, AstNode *base) {
    fprintf(stderr, "Error at %d:%d: %s\n", base->start_line, base->start_column, error);
    self->error_found = true;
}

static void insert_variable_in_symbols_table(AstNodeVariable *node, Analyser *self) {
    if (symbol_table_symbol_exists_in_current_scope(self->symbols, node->name->name)) {
        Symbol *symbol = symbol_table_get(self->symbols, node->name->name);
        char buffer[1024];
        sprintf(buffer, "'%s' already defined at %d:%d.",
                node->name->name,
                ((AstNode *) symbol->ast_node)->start_line,
                ((AstNode *) symbol->ast_node)->start_column);
        print_error(self, buffer, &node->base);
        return;
    }

    Symbol *symbol = symbol_create(node->name->name, SYM_VAR, node);
    symbol_table_add(self->symbols, symbol);
}

static void insert_const_in_symbols_table(AstNodeConst *node, Analyser *self) {
    if (symbol_table_symbol_exists_in_current_scope(self->symbols, node->name->name)) {
        Symbol *symbol = symbol_table_get(self->symbols, node->name->name);
        char buffer[1024];
        sprintf(buffer, "'%s' already defined at %d:%d.",
                node->name->name,
                ((AstNode *) symbol->ast_node)->start_line,
                ((AstNode *) symbol->ast_node)->start_column);
        print_error(self, buffer, &node->base);
        return;
    }

    Symbol *symbol = symbol_create(node->name->name, SYM_CONST, node);
    symbol_table_add(self->symbols, symbol);
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

    return buffer;
}

static void insert_function_in_symbols_table(AstNodeFunction *node, Analyser *self) {
    char *name = mangle_function_name(node);
    if (symbol_table_symbol_exists_in_current_scope(self->symbols, name)) {
        Symbol *symbol = symbol_table_get(self->symbols, name);
        char buffer[1024];
        sprintf(buffer, "'%s' already defined at %d:%d.",
                node->name->name,
                ((AstNode *) symbol->ast_node)->start_line,
                ((AstNode *) symbol->ast_node)->start_column);
        print_error(self, buffer, &node->base);
        free(name);
        return;
    }

    Symbol *symbol = symbol_create(name, SYM_FN, node);
    symbol_table_add(self->symbols, symbol);
}

#pragma endregion

#pragma region public

Analyser *analyzer_create(void) {
    Analyser *analyser = malloc(sizeof(Analyser));
    analyser->symbols = symbol_table_create();
    return analyser;
}

void analyzer_destroy(Analyser *self) {
    symbol_table_destroy(self->symbols);
    free(self);
}

bool analyzer_analyse(Analyser *self, AstRoot *root) {
    list_foreach(root->variables, FN_CONSUMER_CLOSURE(insert_variable_in_symbols_table, self));
    list_foreach(root->constants, FN_CONSUMER_CLOSURE(insert_const_in_symbols_table, self));
    list_foreach(root->functions, FN_CONSUMER_CLOSURE(insert_function_in_symbols_table, self));

    char buffer[1204];

    for (size_t i = 0; i < list_size(root->functions); i++) {
        AstNodeFunction *f = (AstNodeFunction *) list_get(root->functions, i);
        f->base.symbols = symbol_table_create_child_for(self->symbols, &f->base);

        for (size_t s = 0; s < list_size(f->statements->stmts); s++) {
            AstNodeStmt *stmt = (AstNodeStmt *) list_get(f->statements->stmts, s);
            switch (stmt->kind) {
                case STMT_CONST:
                    symbol_table_add(
                            f->base.symbols,
                            symbol_create(stmt->const_stmt->identifier->name, SYM_CONST, stmt)
                    );
                    break;
                case STMT_VAR:
                    symbol_table_add(
                            f->base.symbols,
                            symbol_create(stmt->var_stmt->identifier->name, SYM_VAR, stmt)
                    );
                    break;
                case STMT_ASSIGNMENT:
                    if (!symbol_table_symbol_exists(f->base.symbols, stmt->assignment_stmt->identifier->name)) {
                        sprintf(buffer, "identifier '%s' not defined", stmt->assignment_stmt->identifier->name);
                        print_error(self, buffer, stmt->base);
                    }
                    break;
                case STMT_IF:
                    break;
                case STMT_WHILE:
                    break;
                default:
                    break;
                    // todo die
            }
        }

    }

    return self->error_found;
}

void analyser_dump_symbol_table(Analyser *self) {
    symbol_table_dump(self->symbols);
}

#pragma endregion




