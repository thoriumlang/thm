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
#include "ast.h"
#include "lexer.h"
#include "str.h"
#include "memory.h"

#pragma region AstNodeIdentifier

AstNodeIdentifier *ast_node_identifier_create(Token token) {
    AstNodeIdentifier *node = memory_alloc(sizeof(AstNodeIdentifier));
    node->metadata = memory_alloc(sizeof(AstNode));

    node->name = memory_alloc(token.length * sizeof(char) + 1);
    memcpy(node->name, token.start, token.length);
    node->name[token.length] = 0;

    node->metadata->start_line = token.line;
    node->metadata->start_column = token.column;
    return node;
}

void ast_node_identifier_destroy(AstNodeIdentifier *self) {
    if (self->name) {
        memory_free(self->name);
    }
    memory_free(self->metadata);
    memory_free(self);
}

#pragma endregion

#pragma region AstNodeType

AstNodeType *ast_node_type_create(int ptr, AstNodeIdentifier *identifier, int line, int column) {
    if (identifier == NULL) {
        return NULL;
    }

    AstNodeType *node = memory_alloc(sizeof(AstNodeType));
    node->metadata = memory_alloc(sizeof(AstNode));

    node->ptr = ptr;
    node->identifier = identifier;
    node->metadata->start_line = line;
    node->metadata->start_column = column;

    return node;
}

void ast_node_type_destroy(AstNodeType *self) {
    if (self->identifier) {
        ast_node_identifier_destroy(self->identifier);
    }
    memory_free(self->metadata);
    memory_free(self);
}

void ast_node_type_print(AstNodeType *self) {
    for (int i = 0; i < self->ptr; i++) {
        printf("@");
    }
    printf("%s", self->identifier->name);
}

#pragma endregion

#pragma region AstNodeVariable

AstNodeVariable *ast_node_variable_create(void) {
    AstNodeVariable *node = memory_alloc(sizeof(AstNodeVariable));
    node->metadata = memory_alloc(sizeof(AstNode));
    node->pub = false;
    node->ext = false;
    node->vol = false;
    return node;
}

void ast_node_variable_destroy(AstNodeVariable *self) {
    if (self->name != NULL) {
        ast_node_identifier_destroy(self->name);
    }
    if (self->type != NULL) {
        ast_node_type_destroy(self->type);
    }
    memory_free(self->metadata);
    memory_free(self);
}

void ast_node_variable_print(AstNodeVariable *self) {
    printf("%s%s%svar %s: ",
           self->pub ? "public " : "",
           self->ext ? "external " : "",
           self->vol ? "volatile " : "",
           self->name->name);
    ast_node_type_print(self->type);
    printf(" = <?>;\n");
}

#pragma endregion

#pragma region AstNodeConst

AstNodeConst *ast_node_const_create(void) {
    AstNodeConst *node = memory_alloc(sizeof(AstNodeConst));
    node->metadata = memory_alloc(sizeof(AstNode));
    node->ext = false;
    node->pub = false;
    return node;
}

void ast_node_const_destroy(AstNodeConst *self) {
    if (self->name != NULL) {
        ast_node_identifier_destroy(self->name);
    }
    if (self->type != NULL) {
        ast_node_type_destroy(self->type);
    }
    memory_free(self->metadata);
    memory_free(self);
}

void ast_node_const_print(AstNodeConst *self) {
    printf("%s%sconst %s: ",
           self->pub ? "public " : "",
           self->ext ? "external " : "",
           self->name->name);
    ast_node_type_print(self->type);
    printf(" = <?>;\n");
}

#pragma endregion

#pragma region AstNodeParameter

AstNodeParameter *ast_node_parameter_create(AstNodeIdentifier *identifier, AstNodeType *type, int line, int column) {
    if (identifier && type) {
        AstNodeParameter *node = memory_alloc(sizeof(AstNodeParameter));
        node->metadata = memory_alloc(sizeof(AstNode));

        node->name = identifier;
        node->type = type;
        node->metadata->start_line = line;
        node->metadata->start_column = column;
        return node;
    }
    if (identifier) {
        ast_node_identifier_destroy(identifier);
    }
    if (type) {
        ast_node_type_destroy(type);
    }
    return NULL;
}

void ast_node_parameter_destroy(AstNodeParameter *self) {
    if (self->name != NULL) {
        ast_node_identifier_destroy(self->name);
    }
    if (self->type != NULL) {
        ast_node_type_destroy(self->type);
    }
    memory_free(self->metadata);
    memory_free(self);
}

#pragma endregion

#pragma region AstNodeParameters

AstNodeParameters *ast_node_parameters_create(void) {
    AstNodeParameters *node = memory_alloc(sizeof(AstNodeParameters));
    node->metadata = memory_alloc(sizeof(AstNode));
    node->parameters = list_create();
    return node;
}

void ast_node_parameters_destroy(AstNodeParameters *self) {
    if (self->parameters != NULL) {
        list_foreach(self->parameters, FN_CONSUMER(ast_node_parameter_destroy));
        list_destroy(self->parameters);
    }
    memory_free(self->metadata);
    memory_free(self);
}

#pragma endregion

#pragma region AstNodeStatements

AstNodeStatements *ast_node_stmts_create(void) {
    AstNodeStatements *node = memory_alloc(sizeof(AstNodeStatements));
    node->metadata = memory_alloc(sizeof(AstNode));
    node->stmts = list_create();
    return node;
}

void ast_node_stmts_destroy(AstNodeStatements *self) {
    list_destroy(self->stmts);
    memory_free(self->metadata);
    memory_free(self);
}

#pragma endregion

#pragma region AstNodeFunction

AstNodeFunction *ast_node_function_create(void) {
    AstNodeFunction *node = memory_alloc(sizeof(AstNodeFunction));
    node->metadata = memory_alloc(sizeof(AstNode));
    node->pub = false;
    node->ext = false;
    return node;
}

void ast_node_function_destroy(AstNodeFunction *self) {
    if (self->name != NULL) {
        ast_node_identifier_destroy(self->name);
    }
    if (self->type != NULL) {
        ast_node_type_destroy(self->type);
    }
    if (self->parameters != NULL) {
        ast_node_parameters_destroy(self->parameters);
    }
    if (self->statements != NULL) {
        ast_node_stmts_destroy(self->statements);
    }
    memory_free(self->metadata);
    memory_free(self);
}

void ast_node_function_print(AstNodeFunction *self, int ident) {
    printf("%s%sfn %s(",
           self->pub ? "public " : "",
           self->ext ? "external " : "",
           self->name->name
    );

    for (size_t i = 0; i < list_size(self->parameters->parameters); i++) {
        if (i > 0) {
            printf(", ");
        }

        AstNodeParameter *parameter = list_get(self->parameters->parameters, i);

        printf("%s: ", parameter->name->name);
        ast_node_type_print(parameter->type);
    }

    printf("): ");
    ast_node_type_print(self->type);
    printf(" {\n");

    for (size_t i = 0; i < list_size(self->statements->stmts); i++) {
        ast_node_stmt_print(list_get(self->statements->stmts, i), 1);
    }

    printf("}\n");
}

#pragma endregion

static AstNodeStmt *ast_node_stmt_create(EStmtKind kind) {
    AstNodeStmt *node = memory_alloc(sizeof(AstNodeStmt));
    node->kind = kind;
    return node;
}

#pragma region AstNodeStmtConst

AstNodeStmt *ast_node_stmt_const_create(void) {
    AstNodeStmt *node = ast_node_stmt_create(CONST);
    node->constStmt = memory_alloc(sizeof(AstNodeStmtConst));
    node->constStmt->metadata = memory_alloc(sizeof(AstNode));
    return node;
}

static void ast_node_stmt_const_destroy(AstNodeStmtConst *self) {
    if (self->identifier != NULL) {
        ast_node_identifier_destroy(self->identifier);
    }
    if (self->type != NULL) {
        ast_node_type_destroy(self->type);
    }
    memory_free(self->metadata);
    memory_free(self);
}

static void ast_node_stmt_const_print(AstNodeStmtConst *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%sconst %s: ", ident_str, self->identifier->name);
    ast_node_type_print(self->type);
    printf(" = <?>;\n");
}

#pragma endregion

#pragma region AstNodeStmtVar

AstNodeStmt *ast_node_stmt_var_create(void) {
    AstNodeStmt *node = ast_node_stmt_create(VAR);
    node->varStmt = memory_alloc(sizeof(AstNodeStmtVar));
    node->varStmt->metadata = memory_alloc(sizeof(AstNode));
    return node;
}

static void ast_node_stmt_var_destroy(AstNodeStmtVar *self) {
    if (self->identifier != NULL) {
        ast_node_identifier_destroy(self->identifier);
    }
    if (self->type != NULL) {
        ast_node_type_destroy(self->type);
    }
    memory_free(self->metadata);
    memory_free(self);
}

static void ast_node_stmt_var_print(AstNodeStmtVar *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%svar %s: ", ident_str, self->identifier->name);
    ast_node_type_print(self->type);
    printf(" = <?>;\n");
}

#pragma endregion

#pragma region AstNodeStmtAssignment

AstNodeStmt *ast_node_stmt_assignment_create(void) {
    AstNodeStmt *node = ast_node_stmt_create(ASSIGNMENT);
    node->assignmentStmt = memory_alloc(sizeof(AstNodeStmtAssignment));
    node->assignmentStmt->metadata = memory_alloc(sizeof(AstNode));
    return node;
}

static void ast_node_stmt_assignment_destroy(AstNodeStmtAssignment *self) {
    if (self->identifier != NULL) {
        ast_node_identifier_destroy(self->identifier);
    }
    memory_free(self->metadata);
    memory_free(self);
}

static void ast_node_stmt_assignment_print(AstNodeStmtAssignment *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%s%s = <?>;\n", ident_str, self->identifier->name);
}

#pragma endregion

#pragma region AstNodeStmtIf

AstNodeStmt *ast_node_stmt_if_create(void) {
    AstNodeStmt *node = ast_node_stmt_create(IF);
    node->ifStmt = memory_alloc(sizeof(AstNodeStmtIf));
    node->ifStmt->metadata = memory_alloc(sizeof(AstNode));
    node->ifStmt->true_block = ast_node_stmts_create();
    node->ifStmt->false_block = ast_node_stmts_create();
    return node;
}

static void ast_node_stmt_if_destroy(AstNodeStmtIf *self) {
    ast_node_stmts_destroy(self->true_block);
    ast_node_stmts_destroy(self->false_block);
    memory_free(self->metadata);
    memory_free(self);
}

static void ast_node_stmt_if_print(AstNodeStmtIf *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%sif (<?>) {\n", ident_str);
    for (size_t i = 0; i < list_size(self->true_block->stmts); i++) {
        ast_node_stmt_print(list_get(self->true_block->stmts, i), ident + 1);
    }

    printf("%s}\n", ident_str);
    if (list_size(self->false_block->stmts) > 0) {
        printf("%selse {\n", ident_str);
        for (size_t i = 0; i < list_size(self->false_block->stmts); i++) {
            ast_node_stmt_print(list_get(self->false_block->stmts, i), ident + 1);
        }
        printf("%s}\n", ident_str);
    }
}

#pragma endregion

#pragma region AstNodeStmtWhile

AstNodeStmt *ast_node_stmt_while_create(void) {
    AstNodeStmt *node = ast_node_stmt_create(WHILE);
    node->whileStmt = memory_alloc(sizeof(AstNodeStmtWhile));
    node->whileStmt->metadata = memory_alloc(sizeof(AstNode));
    node->whileStmt->block = ast_node_stmts_create();
    return node;
}

static void ast_node_while_stmt_destroy(AstNodeStmtWhile *self) {
    ast_node_stmts_destroy(self->block);
    memory_free(self->metadata);
    memory_free(self);
}

static void ast_node_stmt_while_print(AstNodeStmtWhile *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%swhile (<?>) {\n", ident_str);
    for (size_t i = 0; i < list_size(self->block->stmts); i++) {
        ast_node_stmt_print(list_get(self->block->stmts, i), ident + 1);
    }
    printf("%s}\n", ident_str);
}

#pragma endregion

#pragma region AstNodeStmt

void ast_node_stmt_destroy(AstNodeStmt *self) {
    switch (self->kind) {
        case CONST:
            ast_node_stmt_const_destroy(self->constStmt);
            break;
        case VAR:
            ast_node_stmt_var_destroy(self->varStmt);
            break;
        case ASSIGNMENT:
            ast_node_stmt_assignment_destroy(self->assignmentStmt);
            break;
        case IF:
            ast_node_stmt_if_destroy(self->ifStmt);
            break;
        case WHILE:
            ast_node_while_stmt_destroy(self->whileStmt);
            break;
        default:
            printf("<unknown statement>;\n"); // todo die
            break;
    }
}

void ast_node_stmt_print(AstNodeStmt *self, int ident) {
    switch (self->kind) {
        case CONST:
            ast_node_stmt_const_print(self->constStmt, ident);
            break;
        case VAR:
            ast_node_stmt_var_print(self->varStmt, ident);
            break;
        case ASSIGNMENT:
            ast_node_stmt_assignment_print(self->assignmentStmt, ident);
            break;
        case IF:
            ast_node_stmt_if_print(self->ifStmt, ident);
            break;
        case WHILE:
            ast_node_stmt_while_print(self->whileStmt, ident);
            break;
        default:
            printf("<unknown statement>;\n"); // todo die
            break;
    }
}

#pragma endregion

#pragma region AstRoot

AstRoot *ast_root_create(void) {
    AstRoot *node = memory_alloc(sizeof(AstRoot));
    node->variables = list_create();
    node->constants = list_create();
    node->functions = list_create();
    return node;
}

void ast_root_destroy(AstRoot *self) {
    list_foreach(self->constants, FN_CONSUMER(ast_node_const_destroy));
    list_destroy(self->constants);
    list_foreach(self->variables, FN_CONSUMER(ast_node_variable_destroy));
    list_destroy(self->variables);
    list_foreach(self->functions, FN_CONSUMER(ast_node_function_destroy));
    list_destroy(self->functions);
    memory_free(self);
}

#pragma endregion
