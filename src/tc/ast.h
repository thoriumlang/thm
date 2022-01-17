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

typedef struct AstNodeIdentifier {
    char *name;
} AstNodeIdentifier;

typedef struct AstNodeType {
    AstNodeIdentifier *identifier;
    int ptr;
} AstNodeType;

typedef struct {
    AstNodeType *type;
    AstNodeIdentifier *name;
    bool pub;
    bool ext;
    bool vol;
} AstNodeVariable;

typedef struct {
    bool pub;
    bool ext;
    AstNodeIdentifier *name;
    AstNodeType *type;
} AstNodeConst;

typedef struct {
    AstNodeIdentifier *name;
    AstNodeType *type;
} AstNodeParameter;

typedef struct {
    List *parameters;
} AstNodeParameters;

typedef struct {
    List *stmts; // of AstNodeStmt
} AstNodeStatements;

typedef struct {
    bool pub;
    bool ext;
    AstNodeType *type;
    AstNodeIdentifier *name;
    AstNodeParameters *parameters;
    AstNodeStatements *statements;
} AstNodeFunction;

typedef struct {
    List *variables;
    List *constants;
    List *functions;
} AstRoot;

typedef enum {
    IF,
    WHILE,
} EStmtKind;

typedef struct {
    // condition
    AstNodeStatements *true_block;
    AstNodeStatements *false_block;
} AstNodeIfStmt;

typedef struct {
    // condition
    AstNodeStatements *block;
} AstNodeWhileStmt;

typedef struct {
    EStmtKind kind;
    union {
        AstNodeIfStmt *ifStmt;
        AstNodeWhileStmt *whileStmt;
    };
} AstNodeStmt;

AstNodeIdentifier *ast_node_identifier_create(Token token);

void ast_node_identifier_destroy(AstNodeIdentifier *this);

AstNodeType *ast_node_type_create(int ptr, AstNodeIdentifier *identifier);

void ast_node_type_destroy(AstNodeType *this);

AstRoot *ast_root_create();

void ast_root_destroy(AstRoot *this);

AstNodeVariable *ast_node_variable_create();

void ast_node_variable_print(AstNodeVariable *this);

void ast_node_variable_destroy(AstNodeVariable *this);

AstNodeFunction *ast_node_function_create();

void ast_node_function_print(AstNodeFunction *this, int ident);

void ast_node_function_destroy(AstNodeFunction *this);

AstNodeParameters *ast_node_parameters_create();

AstNodeParameter *ast_node_parameter_create(AstNodeIdentifier *identifier, AstNodeType *type);

AstNodeConst *ast_node_const_create();

void ast_node_const_print(AstNodeConst *this);

void ast_node_const_destroy(AstNodeConst *this);

AstNodeStatements *ast_node_stmts_create();

AstNodeStmt *ast_node_if_stmt_create();

void ast_node_stmt_print(AstNodeStmt *this, int indent);

void ast_node_stmt_destroy(AstNodeStmt *this);

#endif //THM_AST_H
