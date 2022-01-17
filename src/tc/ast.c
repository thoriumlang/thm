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
#include <stdio.h>
#include "ast.h"
#include "lexer.h"
#include "str.h"

AstNodeIdentifier *ast_node_identifier_create(Token token) {
    AstNodeIdentifier *node = malloc(sizeof(AstNodeIdentifier));
    node->name = malloc(token.length * sizeof(char) + 1);
    memcpy(node->name, token.start, token.length);
    node->name[token.length] = 0;
    return node;
}

void ast_node_identifier_destroy(AstNodeIdentifier *this) {
    if (this->name != NULL) {
        free(this->name);
    }
    free(this);
}

AstNodeType *ast_node_type_create(int ptr, AstNodeIdentifier *identifier) {
    if (identifier == NULL) {
        return NULL;
    }

    AstNodeType *node = malloc(sizeof(AstNodeType));
    node->ptr = ptr;
    node->identifier = identifier;

    return node;
}

void ast_node_type_print(AstNodeType *this) {
    for (int i = 0; i < this->ptr; i++) {
        printf("@");
    }
    printf("%s", this->identifier->name);
}

void ast_node_type_destroy(AstNodeType *this) {
    if (this->identifier != NULL) {
        ast_node_identifier_destroy(this->identifier);
    }
    free(this);
}

AstRoot *ast_root_create() {
    AstRoot *node = malloc(sizeof(AstRoot));
    node->variables = list_create();
    node->constants = list_create();
    node->functions = list_create();
    return node;
}

void ast_root_destroy(AstRoot *this) {
    list_destroy(this->constants);
    list_destroy(this->variables);
    list_destroy(this->functions);
    free(this);
}

AstNodeVariable *ast_node_variable_create() {
    AstNodeVariable *node = malloc(sizeof(AstNodeVariable));
    node->pub = false;
    node->ext = false;
    node->vol = false;
    return node;
}

void ast_node_variable_print(AstNodeVariable *this) {
    printf("%s%s%svar %s: ",
           this->pub ? "public " : "",
           this->ext ? "external " : "",
           this->vol ? "volatile " : "",
           this->name->name);
    ast_node_type_print(this->type);
    printf(" = <?>;\n");
}

void ast_node_variable_destroy(AstNodeVariable *this) {
    if (this->name != NULL) {
        ast_node_identifier_destroy(this->name);
    }
    if (this->type != NULL) {
        ast_node_type_destroy(this->type);
    }
    free(this);
}

AstNodeParameters *ast_node_parameters_create() {
    AstNodeParameters *node = malloc(sizeof(AstNodeParameters));
    node->parameters = list_create();
    return node;
}

void ast_node_parameters_destroy(AstNodeParameters *this) {
    if (this->parameters != NULL) {
        list_destroy(this->parameters);
    }
    free(this);
}


AstNodeFunction *ast_node_function_create() {
    AstNodeFunction *node = malloc(sizeof(AstNodeFunction));
    node->pub = false;
    node->ext = false;
    return node;
}

void ast_node_function_print(AstNodeFunction *this, int ident) {
    printf("%s%sfn %s(",
           this->pub ? "public " : "",
           this->ext ? "external " : "",
           this->name->name
    );

    for (size_t i = 0; i < list_size(this->parameters->parameters); i++) {
        if (i > 0) {
            printf(", ");
        }

        AstNodeParameter *parameter = list_get(this->parameters->parameters, i);

        printf("%s: ", parameter->name->name);
        ast_node_type_print(parameter->type);
    }

    printf("): ");
    ast_node_type_print(this->type);
    printf(" {\n");

    for (size_t i = 0; i < list_size(this->statements->stmts); i++) {
        ast_node_stmt_print(list_get(this->statements->stmts, i), 1);
    }

    printf("}\n");
}

void ast_node_function_destroy(AstNodeFunction *this) {
    if (this->name != NULL) {
        ast_node_identifier_destroy(this->name);
    }
    if (this->type != NULL) {
        ast_node_type_destroy(this->type);
    }
    if (this->parameters != NULL) {
        ast_node_parameters_destroy(this->parameters);
    }
    free(this);
}

AstNodeParameter *ast_node_parameter_create(AstNodeIdentifier *identifier, AstNodeType *type) {
    if (identifier != NULL && type != NULL) {
        AstNodeParameter *node = malloc(sizeof(AstNodeParameter));
        node->name = identifier;
        node->type = type;
        return node;
    }
    if (identifier != NULL) {
        ast_node_identifier_destroy(identifier);
    }
    if (type != NULL) {
        ast_node_type_destroy(type);
    }
    return NULL;
}

AstNodeConst *ast_node_const_create() {
    AstNodeConst *node = malloc(sizeof(AstNodeConst));
    node->ext = false;
    node->pub = false;
    return node;
}

void ast_node_const_print(AstNodeConst *this) {
    printf("%s%sconst %s: ",
           this->pub ? "public " : "",
           this->ext ? "external " : "",
           this->name->name);
    ast_node_type_print(this->type);
    printf(" = <?>;\n");
}

void ast_node_const_destroy(AstNodeConst *this) {
    if (this->name != NULL) {
        ast_node_identifier_destroy(this->name);
    }
    if (this->type != NULL) {
        ast_node_type_destroy(this->type);
    }
    free(this);
}

AstNodeStatements *ast_node_stmts_create() {
    AstNodeStatements *node = malloc(sizeof(AstNodeStatements));
    node->stmts = list_create();
    return node;
}

static void ast_node_stmts_destroy(AstNodeStatements *this) {
    list_destroy(this->stmts);
    free(this);
}

AstNodeStmt *ast_node_if_stmt_create() {
    AstNodeStmt *node = malloc(sizeof(AstNodeStmt));
    node->kind = IF;
    node->ifStmt = malloc(sizeof(AstNodeIfStmt));
    node->ifStmt->true_block = ast_node_stmts_create();
    node->ifStmt->false_block = ast_node_stmts_create();
    return node;
}

static void ast_node_if_stmt_destroy(AstNodeIfStmt *this) {
    ast_node_stmts_destroy(this->true_block);
    ast_node_stmts_destroy(this->false_block);
    free(this);
}

void ast_node_stmt_destroy(AstNodeStmt *this) {
    switch (this->kind) {
        case IF:
            ast_node_if_stmt_destroy(this->ifStmt);
            break;
        case WHILE:
            break;
    }
}

void ast_node_if_stmt_print(AstNodeIfStmt *this, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%sif (<?>) {\n", ident_str);
    for (size_t i = 0; i < list_size(this->true_block->stmts); i++) {
        ast_node_stmt_print(list_get(this->true_block->stmts, i), ident + 1);
    }

    printf("%s ...\n", ident_str);

    printf("%s}\n", ident_str);
    if (list_size(this->false_block->stmts)>0) {
        printf("%selse {\n", ident_str);
        for (size_t i = 0; i < list_size(this->false_block->stmts); i++) {
            ast_node_stmt_print(list_get(this->false_block->stmts, i), ident+1);
        }
        printf("%s}\n", ident_str);
    }
}

void ast_node_stmt_print(AstNodeStmt *this, int ident) {
    switch (this->kind) {
        case IF:
            ast_node_if_stmt_print(this->ifStmt, ident);
            break;
        default:
            printf("<unknown statement>;\n");
            break;
    }
}

