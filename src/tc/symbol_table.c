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
#include <assert.h>
#include "symbol_table.h"
#include "map.h"
#include "ast.h"
#include "memory.h"
#include "pair.h"

typedef struct SymbolTable {
    SymbolTable *parent;
    Map *symbols; // str -> Symbol
    List *children;
    AstNode *owning_node;
} SymbolTable;

#pragma region private

static void free_symbol(Pair *e) {
    switch (((Symbol *) pair_get_value(e))->kind) {
        case SYM_FN:
            free(pair_get_value(e));
            free(pair_get_key(e));
            break;
        default:
            free(pair_get_value(e));
            // key is allocated in the scope of the AST and hence freed in the scope of the AST
            break;
    }
}

static void free_child_table(SymbolTable *t) {
    symbol_table_destroy(t);
}

#pragma endregion

#pragma region public

SymbolTable *symbol_table_create(void) {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->parent = NULL;
    table->symbols = map_create(map_hash_fn_str, map_eq_fn_str);
    table->children = list_create();
    return table;
}

SymbolTable *symbol_table_create_child_for(SymbolTable *self, /*actually a (AstNode*) */void *node) {
    SymbolTable *table = symbol_table_create();

    table->owning_node = node;
    table->parent = self;
    list_add(self->children, table);

    return table;
}

void symbol_table_destroy(SymbolTable *self) {
    List *entries = map_get_entries(self->symbols);
    list_foreach(entries, FN_CONSUMER(free_symbol));
    list_foreach(entries, FN_CONSUMER(pair_destroy));
    list_destroy(entries);
    map_destroy(self->symbols);

    list_foreach(self->children, FN_CONSUMER(free_child_table));
    list_destroy(self->children);

    if (self->owning_node) {
        self->owning_node->symbols = NULL;
    }

    free(self);
}

void symbol_table_add(SymbolTable *self, Symbol *symbol) {
    map_put(self->symbols, symbol->name, symbol);
}

bool symbol_table_symbol_exists_in_current_scope(SymbolTable *self, char *name) {
    return map_is_present(self->symbols, name);
}

bool symbol_table_symbol_exists(SymbolTable *self, char *name) {
    if (map_is_present(self->symbols, name)) {
        return true;
    }
    if (self->parent) {
        return symbol_table_symbol_exists(self->parent, name);
    }
    return false;
}

Symbol *symbol_table_get(SymbolTable *self, char *name) {
    return map_get(self->symbols, name);
}

void symbol_table_dump(SymbolTable *self) {
    List *values = map_get_values(self->symbols);

    for (size_t i = 0; i < list_size(values); i++) {
        Symbol *symbol = (Symbol *) list_get(values, i);
        AstNode *node = (AstNode *) symbol->ast_node;
        switch (symbol->kind) {
            case SYM_VAR:
                printf("VAR %s declared at %d:%d\n", symbol->name, node->start_line, node->start_column);
                break;
            case SYM_FN:
                printf("FN %s declared at %d:%d\n", symbol->name, node->start_line, node->start_column);
                break;
            case SYM_CONST:
                printf("CONST %s declared at %d:%d\n", symbol->name, node->start_line, node->start_column);
                break;
            default:
                assert(false);
        }
    }

    list_destroy(values);
}

#pragma endregion