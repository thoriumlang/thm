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
#include "symbol_table.h"
#include "map.h"
#include "ast.h"

typedef struct SymbolTable {
    SymbolTable *parent;
    Map *symbols; // str -> Symbol
} SymbolTable;

SymbolTable *symbol_table_create() {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->parent = NULL;
    table->symbols = map_create(map_hash_fn_str, map_eq_fn_str);
    return table;
}

static void free_symbol(Pair *e) {
    switch (((Symbol *) e->value)->kind) {
        case SYM_FN:
            free(e->value);
            free(e->key);
            break;
        default:
            free(e->value);
            // key is allocated in the scope of the AST and hence freed in the scope of the AST
            break;
    }
}

void symbol_table_destroy(SymbolTable *this) {
    List *entries = map_get_entries(this->symbols);
    list_foreach(entries, fn_consumer(free_symbol));
    list_destroy(entries);
    map_destroy(this->symbols);
    free(this);
}

void symbol_table_add(SymbolTable *this, Symbol *symbol) {
    map_put(this->symbols, symbol->name, symbol);
}

bool symbol_table_symbol_exists_in_current_scope(SymbolTable *this, char *name) {
    return map_is_present(this->symbols, name);
}

Symbol *symbol_table_get(SymbolTable *this, char *name) {
    return map_get(this->symbols, name);
}

void symbol_table_dump(SymbolTable *this) {
    List *values = map_get_values(this->symbols);

    for (size_t i = 0; i < list_size(values); i++) {
        Symbol *symbol = (Symbol *) list_get(values, i);
        AstNode *node = *(AstNode **) symbol->ast_node;
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
        }

    }

    list_destroy(values);
}