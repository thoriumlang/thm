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

#ifndef THM_AST_H
#define THM_AST_H

#include <stdbool.h>
#include <list.h>
#include "lexer.h"
#include "symbol_table.h"

#pragma region AstNode

typedef struct AstNode {
    int start_line;
    int start_column;
    SymbolTable *symbols;
} AstNode;

#pragma endregion

#pragma region AstNodeIdentifier

typedef struct AstNodeIdentifier {
    AstNode base;
    char *name;
} AstNodeIdentifier;

AstNodeIdentifier *ast_node_identifier_create(Token token);

void ast_node_identifier_destroy(AstNodeIdentifier *self);

#pragma endregion

#pragma region AstNodeType

typedef struct AstNodeType {
    AstNode base;
    AstNodeIdentifier *identifier;
    int ptr;
} AstNodeType;

AstNodeType *ast_node_type_create(int ptr, AstNodeIdentifier *identifier, int line, int column);

void ast_node_type_destroy(AstNodeType *self);

void ast_node_type_print(AstNodeType *self);

#pragma endregion

#pragma region AstNodeNumber

typedef struct AstNodeNumber {
    AstNode base;
    int value;
} AstNodeNumber;

AstNodeNumber *ast_node_number_create(Token token);

void ast_node_number_destroy(AstNodeNumber *self);

void ast_node_number_print(AstNodeNumber *self);

#pragma endregion

typedef struct AstNodeExpression AstNodeExpression;

#pragma region Operator

typedef enum EOperator {
    OPERATOR_equals,
    OPERATOR_not_equals,
    OPERATOR_lt,
    OPERATOR_lt_equals,
    OPERATOR_gt,
    OPERATOR_gt_equals,
    OPERATOR_plus,
    OPERATOR_minus,
    OPERATOR_star,
    OPERATOR_slash,
    OPERATOR_exclam,
    OPERATOR_amp,
    OPERATOR_at,

} EOperator;

typedef struct AstNodeOperator {
    AstNode base;
    EOperator op;
} AstNodeOperator;

AstNodeOperator *ast_node_operator_create(EOperator op);

void ast_node_operator_destroy(AstNodeOperator *self);

void ast_node_operator_print(AstNodeOperator *self);

#pragma endregion

#pragma region AstNodeBinaryExpression

typedef struct AstNodeBinaryExpression {
    AstNode base;
    AstNodeExpression *left;
    AstNodeExpression *right;
    AstNodeOperator *op;
} AstNodeBinaryExpression;

AstNodeBinaryExpression *ast_node_binary_expression_create(AstNodeExpression *l,
                                                           AstNodeExpression *r,
                                                           AstNodeOperator *op);

#pragma endregion

#pragma region AstNodeExpression

typedef enum {
    EXPRESSION_NUMBER,
    EXPRESSION_IDENTIFIER,
    EXPRESSION_BINARY
} EExpressionKind;

typedef struct AstNodeExpression {
    union {
        AstNodeNumber *number_expression;
        AstNodeIdentifier *identifier_expression;
        AstNodeBinaryExpression *binary_expression;
    };
    EExpressionKind kind;
} AstNodeExpression;

AstNodeExpression *ast_node_expression_create();

void ast_node_expression_destroy(AstNodeExpression *self);

void ast_node_expression_print(AstNodeExpression *self);

#pragma endregion

#pragma region AstNodeVariable

typedef struct {
    AstNode base;
    AstNodeType *type;
    AstNodeIdentifier *name;
    AstNodeExpression *expression;
    bool pub;
    bool ext;
    bool vol;
} AstNodeVariable;

AstNodeVariable *ast_node_variable_create(void);

void ast_node_variable_destroy(AstNodeVariable *self);

void ast_node_variable_print(AstNodeVariable *self);

#pragma endregion

#pragma region AstNodeConst

typedef struct {
    AstNode base;
    bool pub;
    bool ext;
    AstNodeIdentifier *name;
    AstNodeType *type;
    AstNodeExpression *expression;
} AstNodeConst;

AstNodeConst *ast_node_const_create(void);

void ast_node_const_destroy(AstNodeConst *self);

void ast_node_const_print(AstNodeConst *self);

#pragma endregion

#pragma region AstNodeParameter

typedef struct {
    AstNode base;
    AstNodeIdentifier *name;
    AstNodeType *type;
} AstNodeParameter;

AstNodeParameter *ast_node_parameter_create(AstNodeIdentifier *identifier, AstNodeType *type, int line, int column);

void ast_node_parameter_destroy(AstNodeParameter *self);

#pragma endregion

#pragma region AstNodeParameters

typedef struct AstNodeParameters {
    AstNode base;
    List *parameters;
} AstNodeParameters;

AstNodeParameters *ast_node_parameters_create(void);

void ast_node_parameters_destroy(AstNodeParameters *self);

#pragma endregion

#pragma region AstNodeStatements

typedef struct {
    AstNode base;
    List *stmts; // of AstNodeStmt
} AstNodeStatements;

AstNodeStatements *ast_node_stmts_create(void);

void ast_node_stmts_destroy(AstNodeStatements *self);

#pragma endregion

#pragma region AstNodeFunction

typedef struct {
    AstNode base;
    bool pub;
    bool ext;
    AstNodeType *type;
    AstNodeIdentifier *name;
    AstNodeParameters *parameters;
    AstNodeStatements *statements;
} AstNodeFunction;

AstNodeFunction *ast_node_function_create(void);

void ast_node_function_print(AstNodeFunction *self, int ident);

void ast_node_function_destroy(AstNodeFunction *self);

#pragma endregion

typedef struct AstNodeStmt AstNodeStmt;

#pragma region AstNodeStmtConst

typedef struct {
    AstNode base;
    AstNodeIdentifier *identifier;
    AstNodeType *type;
    AstNodeExpression *expression;
} AstNodeStmtConst;

AstNodeStmt *ast_node_stmt_const_create(void);

#pragma endregion

#pragma region AstNodeStmtVar

typedef struct {
    AstNode base;
    AstNodeIdentifier *identifier;
    AstNodeType *type;
    AstNodeExpression *expression;
} AstNodeStmtVar;

AstNodeStmt *ast_node_stmt_var_create(void);

#pragma endregion

#pragma region AstNodeStmtAssignment

typedef struct {
    AstNode base;
    AstNodeIdentifier *identifier;
    // expression
} AstNodeStmtAssignment;

AstNodeStmt *ast_node_stmt_assignment_create(void);

#pragma endregion

#pragma region AstNodeStmtIf

typedef struct {
    AstNode base;
    // condition
    AstNodeStatements *true_block;
    AstNodeStatements *false_block;
} AstNodeStmtIf;

AstNodeStmt *ast_node_stmt_if_create(void);

#pragma endregion

#pragma region AstNodeStmtWhile

typedef struct {
    AstNode base;
    // condition
    AstNodeStatements *block;
} AstNodeStmtWhile;

AstNodeStmt *ast_node_stmt_while_create(void);

#pragma endregion

#pragma region AstNodeStmt

typedef enum {
    CONST,
    VAR,
    ASSIGNMENT,
    IF,
    WHILE,
} EStmtKind;

typedef struct AstNodeStmt {
    union {
        AstNodeStmtConst *const_stmt;
        AstNodeStmtVar *var_stmt;
        AstNodeStmtAssignment *assignment_stmt;
        AstNodeStmtIf *if_stmt;
        AstNodeStmtWhile *while_stmt;
    };
    EStmtKind kind;
} AstNodeStmt;

void ast_node_stmt_destroy(AstNodeStmt *self);

void ast_node_stmt_print(AstNodeStmt *self, int indent);

#pragma endregion

#pragma region AstRoot

typedef struct {
    AstNode base;
    List *variables; // of AstNodeVariable
    List *constants; // of AstNodeConst
    List *functions; // of AstNodeFunction
} AstRoot;

AstRoot *ast_root_create(void);

void ast_root_destroy(AstRoot *self);

#pragma endregion

#endif //THM_AST_H
