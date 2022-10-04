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
#include <assert.h>
#include "ast.h"
#include "lexer.h"
#include "str.h"
#include "memory.h"
#include "list.h"

#pragma region AstNodeIdentifier

AstNodeIdentifier *ast_node_identifier_create(Token token) {
    AstNodeIdentifier *node = memory_alloc(sizeof(AstNodeIdentifier));

    node->name = memory_alloc(token.length * sizeof(char) + 1);
    memcpy(node->name, token.start, token.length);
    node->name[token.length] = 0;

    node->base.start_line = token.line;
    node->base.start_column = token.column;
    return node;
}

void ast_node_identifier_destroy(AstNodeIdentifier *self) {
    if (self->name != NULL) {
        memory_free(self->name);
    }
    memory_free(self);
}

#pragma endregion

#pragma region AstNodeType

AstNodeType *ast_node_type_create(int ptr, AstNodeIdentifier *identifier, int line, int column) {
    if (identifier == NULL) {
        return NULL;
    }

    AstNodeType *node = memory_alloc(sizeof(AstNodeType));

    node->ptr = ptr;
    node->identifier = identifier;
    node->base.start_line = line;
    node->base.start_column = column;

    return node;
}

void ast_node_type_destroy(AstNodeType *self) {
    if (self->identifier != NULL) {
        ast_node_identifier_destroy(self->identifier);
    }
    memory_free(self);
}

void ast_node_type_print(AstNodeType *self) {
    for (int i = 0; i < self->ptr; i++) {
        printf("@");
    }
    printf("%s", self->identifier->name);
}

#pragma endregion

#pragma region AstNodeNumber

AstNodeNumber *ast_node_number_create(Token token) {
    AstNodeNumber *node = memory_alloc(sizeof(AstNodeNumber));
    node->base.start_line = token.line;
    node->base.start_column = token.column;
    node->value = 0;

    for (const char *c = token.start; c < token.start + token.length; c++) {
        node->value *= 10;
        node->value += *c - '0';
    }

    return node;
}

void ast_node_number_destroy(AstNodeNumber *self) {
    memory_free(self);
}

void ast_node_number_print(AstNodeNumber *self) {
    printf("%d", self->value);
}

#pragma endregion

#pragma region AstNodeOperator

AstNodeOperator *ast_node_operator_create(EOperator op) {
    AstNodeOperator *node = memory_alloc(sizeof(AstNodeOperator));
    node->op = op;
    return node;
}

void ast_node_operator_destroy(AstNodeOperator *self) {
    memory_free(self);
}

void ast_node_operator_print(AstNodeOperator *self) {
    switch (self->op) {
        case OPERATOR_plus:
            printf("+");
            break;
        case OPERATOR_minus:
            printf("-");
            break;
        case OPERATOR_star:
            printf("*");
            break;
        case OPERATOR_equals:
            printf("==");
            break;
        case OPERATOR_not_equals:
            printf("!=");
            break;
        case OPERATOR_gt:
            printf(">");
            break;
        case OPERATOR_gt_equals:
            printf(">=");
            break;
        case OPERATOR_lt:
            printf("<");
            break;
        case OPERATOR_lt_equals:
            printf("<=");
            break;
        default:
            // todo die
            break;
    }
}

#pragma endregion

#pragma region AstNodeBinaryExpression

AstNodeBinaryExpression *ast_node_binary_expression_create(AstNodeExpression *l,
                                                           AstNodeExpression *r,
                                                           AstNodeOperator *op) {
    AstNodeBinaryExpression *node = memory_alloc(sizeof(AstNodeBinaryExpression));
    node->left = l;
    node->right = r;
    node->op = op;
    return node;
}

void ast_node_binary_expression_destroy(AstNodeBinaryExpression *self) {
    ast_node_expression_destroy(self->left);
    ast_node_expression_destroy(self->right);
    memory_free(self->op);
    memory_free(self);
}

void ast_node_binary_expression_print(AstNodeBinaryExpression *self) {
    printf("(");
    ast_node_expression_print(self->left);
    ast_node_operator_print(self->op);
    ast_node_expression_print(self->right);
    printf(")");
}

#pragma endregion

#pragma region AstNodeExpression

AstNodeExpression *ast_node_expression_create() {
    return memory_alloc(sizeof(AstNodeExpression));
}

void ast_node_expression_destroy(AstNodeExpression *self) {
    switch (self->kind) {
        case EXPRESSION_NUMBER:
            ast_node_number_destroy(self->number_expression);
            break;
        case EXPRESSION_IDENTIFIER:
            ast_node_identifier_destroy(self->identifier_expression);
            break;
        case EXPRESSION_BINARY:
            ast_node_binary_expression_destroy(self->binary_expression);
            break;
    }
    memory_free(self);
}

void ast_node_expression_print(AstNodeExpression *self) {
    switch (self->kind) {
        case EXPRESSION_NUMBER:
            ast_node_number_print(self->number_expression);
            break;
        case EXPRESSION_IDENTIFIER:
            printf("%s", self->identifier_expression->name);
            break;
        case EXPRESSION_BINARY:
            ast_node_binary_expression_print(self->binary_expression);
            break;
    }
}

#pragma endregion

#pragma region AstNodeVariable

AstNodeVariable *ast_node_variable_create(void) {
    AstNodeVariable *node = memory_alloc(sizeof(AstNodeVariable));
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
    if (self->expression != NULL) {
        ast_node_expression_destroy(self->expression);
    }
    memory_free(self);
}

void ast_node_variable_print(AstNodeVariable *self) {
    printf("%s%s%svar %s: ",
           self->pub ? "public " : "",
           self->ext ? "external " : "",
           self->vol ? "volatile " : "",
           self->name->name);
    ast_node_type_print(self->type);
    printf(" = ");
    ast_node_expression_print(self->expression);
    printf(";\n");
}

#pragma endregion

#pragma region AstNodeConst

AstNodeConst *ast_node_const_create(void) {
    AstNodeConst *node = memory_alloc(sizeof(AstNodeConst));
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
    if (self->expression != NULL) {
        ast_node_expression_destroy(self->expression);
    }
    memory_free(self);
}

void ast_node_const_print(AstNodeConst *self) {
    printf("%s%sconst %s: ",
           self->pub ? "public " : "",
           self->ext ? "external " : "",
           self->name->name);
    ast_node_type_print(self->type);
    printf(" = ");
    ast_node_expression_print(self->expression);
    printf(";\n");
}

#pragma endregion

#pragma region AstNodeParameter

AstNodeParameter *ast_node_parameter_create(AstNodeIdentifier *identifier, AstNodeType *type, int line, int column) {
    if (identifier && type) {
        AstNodeParameter *node = memory_alloc(sizeof(AstNodeParameter));

        node->name = identifier;
        node->type = type;
        node->base.start_line = line;
        node->base.start_column = column;
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
    memory_free(self);
}

#pragma endregion

#pragma region AstNodeParameters

AstNodeParameters *ast_node_parameters_create(void) {
    AstNodeParameters *node = memory_alloc(sizeof(AstNodeParameters));
    node->parameters = list_create(); // FIXME make sure it's freed
    return node;
}

void ast_node_parameters_destroy(AstNodeParameters *self) {
    if (self->parameters != NULL) {
        list_foreach(self->parameters, FN_CONSUMER(ast_node_parameter_destroy));
        list_destroy(self->parameters);
    }
    memory_free(self);
}

#pragma endregion

#pragma region AstNodeStatements

AstNodeStatements *ast_node_stmts_create(void) {
    AstNodeStatements *node = memory_alloc(sizeof(AstNodeStatements));
    node->stmts = list_create(); // FIXME make sure it's freed
    return node;
}

void ast_node_stmts_destroy(AstNodeStatements *self) {
    list_foreach(self->stmts, FN_CONSUMER(ast_node_stmt_destroy));
    list_destroy(self->stmts);
    memory_free(self);
}

#pragma endregion

#pragma region AstNodeFunction

AstNodeFunction *ast_node_function_create(void) {
    AstNodeFunction *node = memory_alloc(sizeof(AstNodeFunction));
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
    switch (kind) {
        case STMT_ASSIGNMENT:
            node->assignment_stmt = memory_alloc(sizeof(AstNodeStmtAssignment));
            break;
        case STMT_CONST:
            node->assignment_stmt = memory_alloc(sizeof(AstNodeStmtConst));
            break;
        case STMT_VAR:
            node->assignment_stmt = memory_alloc(sizeof(AstNodeStmtVar));
            break;
        case STMT_IF:
            node->assignment_stmt = memory_alloc(sizeof(AstNodeStmtIf));
            break;
        case STMT_WHILE:
            node->assignment_stmt = memory_alloc(sizeof(AstNodeStmtWhile));
            break;
        default:
            assert(false);
    }
    node->kind = kind;
    return node;
}

#pragma region AstNodeStmtConst

AstNodeStmt *ast_node_stmt_const_create(void) {
    return ast_node_stmt_create(STMT_CONST);
}

static void ast_node_stmt_const_destroy(AstNodeStmtConst *self) {
    if (self->identifier != NULL) {
        ast_node_identifier_destroy(self->identifier);
    }
    if (self->type != NULL) {
        ast_node_type_destroy(self->type);
    }
    if (self->expression != NULL) {
        ast_node_expression_destroy(self->expression);
    }
    memory_free(self);
}

static void ast_node_stmt_const_print(AstNodeStmtConst *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%sconst %s: ", ident_str, self->identifier->name);
    ast_node_type_print(self->type);
    printf(" = ");
    ast_node_expression_print(self->expression);
    printf(";\n");
}

#pragma endregion

#pragma region AstNodeStmtVar

AstNodeStmt *ast_node_stmt_var_create(void) {
    return ast_node_stmt_create(STMT_VAR);
}

static void ast_node_stmt_var_destroy(AstNodeStmtVar *self) {
    if (self->identifier != NULL) {
        ast_node_identifier_destroy(self->identifier);
    }
    if (self->type != NULL) {
        ast_node_type_destroy(self->type);
    }
    if (self->expression != NULL) {
        ast_node_expression_destroy(self->expression);
    }
    memory_free(self);
}

static void ast_node_stmt_var_print(AstNodeStmtVar *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%svar %s: ", ident_str, self->identifier->name);
    ast_node_type_print(self->type);
    if (self->expression != NULL) {
        printf(" = ");
        ast_node_expression_print(self->expression);
    }
    printf(";\n");
}

#pragma endregion

#pragma region AstNodeStmtAssignment

AstNodeStmt *ast_node_stmt_assignment_create(void) {
    return ast_node_stmt_create(STMT_ASSIGNMENT);
}

static void ast_node_stmt_assignment_destroy(AstNodeStmtAssignment *self) {
    if (self->identifier != NULL) {
        ast_node_identifier_destroy(self->identifier);
    }
    if (self->expression != NULL) {
        ast_node_expression_destroy(self->expression);
    }
    memory_free(self);
}

static void ast_node_stmt_assignment_print(AstNodeStmtAssignment *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%s%s = ", ident_str, self->identifier->name);
    ast_node_expression_print(self->expression);
    printf(";\n");
}

#pragma endregion

#pragma region AstNodeStmtIf

AstNodeStmt *ast_node_stmt_if_create(void) {
    return ast_node_stmt_create(STMT_IF);
}

static void ast_node_stmt_if_destroy(AstNodeStmtIf *self) {
    if (self->expression != NULL) {
        ast_node_expression_destroy(self->expression);
    }
    if (self->true_block != NULL) {
        ast_node_stmts_destroy(self->true_block);
    }
    if (self->false_block != NULL) {
        ast_node_stmts_destroy(self->false_block);
    }
    memory_free(self);
}

static void ast_node_stmt_if_print(AstNodeStmtIf *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%sif (", ident_str);
    ast_node_expression_print(self->expression);
    printf(") {\n");
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
    return ast_node_stmt_create(STMT_WHILE);
}

static void ast_node_while_stmt_destroy(AstNodeStmtWhile *self) {
    if (self->expression != NULL) {
        ast_node_expression_destroy(self->expression);
    }
    if (self->block != NULL) {
        ast_node_stmts_destroy(self->block);
    }
    memory_free(self);
}

static void ast_node_stmt_while_print(AstNodeStmtWhile *self, int ident) {
    char *ident_str = calloc(ident * 2 + 1, sizeof(char));
    str_repeat(ident_str, " ", ident * 2);
    printf("%swhile (", ident_str);
    ast_node_expression_print(self->expression);
    printf(") {\n");
    for (size_t i = 0; i < list_size(self->block->stmts); i++) {
        ast_node_stmt_print(list_get(self->block->stmts, i), ident + 1);
    }
    printf("%s}\n", ident_str);
}

#pragma endregion

#pragma region AstNodeStmt

void ast_node_stmt_destroy(AstNodeStmt *self) {
    switch (self->kind) {
        case STMT_CONST:
            ast_node_stmt_const_destroy(self->const_stmt);
            break;
        case STMT_VAR:
            ast_node_stmt_var_destroy(self->var_stmt);
            break;
        case STMT_ASSIGNMENT:
            ast_node_stmt_assignment_destroy(self->assignment_stmt);
            break;
        case STMT_IF:
            ast_node_stmt_if_destroy(self->if_stmt);
            break;
        case STMT_WHILE:
            ast_node_while_stmt_destroy(self->while_stmt);
            break;
        default:
            printf("<unknown statement>;\n"); // todo die
            break;
    }
    memory_free(self);
}

void ast_node_stmt_print(AstNodeStmt *self, int ident) {
    switch (self->kind) {
        case STMT_CONST:
            ast_node_stmt_const_print(self->const_stmt, ident);
            break;
        case STMT_VAR:
            ast_node_stmt_var_print(self->var_stmt, ident);
            break;
        case STMT_ASSIGNMENT:
            ast_node_stmt_assignment_print(self->assignment_stmt, ident);
            break;
        case STMT_IF:
            ast_node_stmt_if_print(self->if_stmt, ident);
            break;
        case STMT_WHILE:
            ast_node_stmt_while_print(self->while_stmt, ident);
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
    node->variables = list_create(); // FIXME make sure it's freed
    node->constants = list_create(); // FIXME make sure it's freed
    node->functions = list_create(); // FIXME make sure it's freed
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
