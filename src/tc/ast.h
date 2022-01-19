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

#pragma region AstNodeIdentifier

typedef struct AstNodeIdentifier {
    char *name;
} AstNodeIdentifier;

AstNodeIdentifier *ast_node_identifier_create(Token token);

void ast_node_identifier_destroy(AstNodeIdentifier *this);

#pragma endregion

#pragma region AstNodeType

typedef struct AstNodeType {
    AstNodeIdentifier *identifier;
    int ptr;
} AstNodeType;

AstNodeType *ast_node_type_create(int ptr, AstNodeIdentifier *identifier);

void ast_node_type_destroy(AstNodeType *this);

 void ast_node_type_print(AstNodeType *this);

#pragma endregion

#pragma region AstNodeVariable

 typedef struct {
    AstNodeType *type;
    AstNodeIdentifier *name;
    bool pub;
    bool ext;
    bool vol;
} AstNodeVariable;

AstNodeVariable *ast_node_variable_create();

void ast_node_variable_destroy(AstNodeVariable *this);

void ast_node_variable_print(AstNodeVariable *this);

#pragma endregion

#pragma region AstNodeConst

typedef struct {
    bool pub;
    bool ext;
    AstNodeIdentifier *name;
    AstNodeType *type;
} AstNodeConst;

AstNodeConst *ast_node_const_create();

void ast_node_const_destroy(AstNodeConst *this);

void ast_node_const_print(AstNodeConst *this);

#pragma endregion

#pragma region AstNodeParameter

typedef struct {
    AstNodeIdentifier *name;
    AstNodeType *type;
} AstNodeParameter;

AstNodeParameter *ast_node_parameter_create(AstNodeIdentifier *identifier, AstNodeType *type);

#pragma endregion

#pragma region AstNodeParameters

typedef struct AstNodeParameters {
    List *parameters;
} AstNodeParameters;

AstNodeParameters *ast_node_parameters_create();

void ast_node_parameters_destroy(AstNodeParameters *this);

#pragma endregion

#pragma region AstNodeStatements

typedef struct {
    List *stmts; // of AstNodeStmt
} AstNodeStatements;

AstNodeStatements *ast_node_stmts_create();

void ast_node_stmts_destroy(AstNodeStatements *this);

#pragma endregion

#pragma region AstNodeFunction

typedef struct {
    bool pub;
    bool ext;
    AstNodeType *type;
    AstNodeIdentifier *name;
    AstNodeParameters *parameters;
    AstNodeStatements *statements;
} AstNodeFunction;

AstNodeFunction *ast_node_function_create();

void ast_node_function_print(AstNodeFunction *this, int ident);

void ast_node_function_destroy(AstNodeFunction *this);

#pragma endregion

typedef struct AstNodeStmt AstNodeStmt;

#pragma region AstNodeStmtVar

typedef struct {
    AstNodeIdentifier *identifier;
    AstNodeType *type;
    // expression
} AstNodeStmtVar;

AstNodeStmt *ast_node_stmt_var_create();

#pragma endregion

#pragma region AstNodeStmtAssignment

typedef struct {
    AstNodeIdentifier *identifier;
    // expression
} AstNodeStmtAssignment;

AstNodeStmt *ast_node_stmt_assignment_create();

#pragma endregion

#pragma region AstNodeStmtIf

typedef struct {
    // condition
    AstNodeStatements *true_block;
    AstNodeStatements *false_block;
} AstNodeStmtIf;

AstNodeStmt *ast_node_stmt_if_create();

#pragma endregion

#pragma region AstNodeStmtWhile

typedef struct {
    // condition
    AstNodeStatements *block;
} AstNodeStmtWhile;

AstNodeStmt *ast_node_stmt_while_create();

#pragma endregion

#pragma region AstNodeStmt

typedef enum {
    VAR,
    ASSIGNMENT,
    IF,
    WHILE,
} EStmtKind;

typedef struct AstNodeStmt {
    EStmtKind kind;
    union {
        AstNodeStmtVar *varStmt;
        AstNodeStmtAssignment *assignmentStmt;
        AstNodeStmtIf *ifStmt;
        AstNodeStmtWhile *whileStmt;
    };
} AstNodeStmt;

void ast_node_stmt_destroy(AstNodeStmt *this);

void ast_node_stmt_print(AstNodeStmt *this, int indent);

#pragma endregion

#pragma region AstRoot

typedef struct {
    List *variables; // of AstNodeVariable
    List *constants; // of AstNodeConst
    List *functions; // of AstNodeFunction
} AstRoot;

AstRoot *ast_root_create();

void ast_root_destroy(AstRoot *this);

#pragma endregion

#endif //THM_AST_H
