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
#include <queue.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "parser.h"
#include "lexer.h"

typedef struct Parser {
    Lexer *lexer;
    Queue *tokens;
    bool error_found;
    bool error_recovery;
} Parser;

static bool is_at_end(Parser *this);

static Token *peek(Parser *this, int n);

static Token advance(Parser *this);

static bool match(Parser *this, ETokenType expected);

static void expect(Parser *this, ETokenType expected);

static bool check(Parser *this, ETokenType expected);

static bool check_within(Parser *this, ETokenType expected, int n);

static AstNodeVariable *parse_variable(Parser *this);

static AstNodeFunction *parse_function(Parser *this);

static AstNodeStatements *parse_stmts(Parser *this);

static AstNodeConst *parse_const(Parser *this);

Parser *parser_create(Lexer *lexer) {
    Parser *parser = malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->tokens = queue_create(4);
    parser->error_recovery = false;
    parser->error_found = false;
    return parser;
}

void parser_destroy(Parser *this) {
    queue_destroy(this->tokens);
    free(this);
}

static void print_token_expected_error(Parser *this, size_t tokens, ...) {
    if (this->error_recovery) {
        return;
    }

    this->error_found = true;
    this->error_recovery = true;

    va_list argp;
    va_start(argp, tokens);

    fprintf(stderr, "Expected ");
    if (tokens > 1) {
        fprintf(stderr, "( ");
    }
    for (size_t i = 0; i < tokens; i++) {
        ETokenType token = va_arg(argp, ETokenType);
        if (i > 0) {
            fprintf(stderr, " | ");
        }
        fprintf(stderr, "<%s>", token_type_to_string(token));
    }
    if (tokens > 1) {
        fprintf(stderr, " )");
    }
    Token *token = peek(this, 0);
    fprintf(stderr, " at %i:%i\n", token->line, token->column);
}

static void print_expected_error(Parser *this, const char *expected) {
    if (this->error_recovery) {
        return;
    }

    this->error_found = true;
    this->error_recovery = true;

    Token *token = peek(this, 0);
    fprintf(stderr, "Expected %s at %i:%i\n", expected, token->line, token->column);
}

AstRoot *parser_parse(Parser *this) {
    AstRoot *root = ast_root_create();

    while (peek(this, 0)->type != TOKEN_EOF) {
        while (match(this, TOKEN_SEMICOLON)) {
            // nothing
        }
        if (check_within(this, TOKEN_VAR, 3)) {
            AstNodeVariable *variable = parse_variable(this);
            if (variable != NULL) {
                list_add(root->variables, variable);
            }
        } else if (check_within(this, TOKEN_FN, 2)) {
            AstNodeFunction *function = parse_function(this);
            if (function != NULL) {
                list_add(root->functions, function);
            }
        } else if (check_within(this, TOKEN_CONST, 2)) {
            AstNodeConst *constant = parse_const(this);
            if (constant != NULL) {
                list_add(root->constants, constant);
            }
        } else {
            print_expected_error(this, "constant, variable or function definition");
            advance(this);
        }
    }

    if (this->error_found) {
        ast_root_destroy(root);
        return NULL;
    } else {
        return root;
    }
}

// <identifier> := <IDENTIFIER>
static AstNodeIdentifier *parse_identifier(Parser *this) {
    if (!check(this, TOKEN_IDENTIFIER)) {
        print_token_expected_error(this, 1, TOKEN_IDENTIFIER);
        return NULL;
    }

    return ast_node_identifier_create(advance(this));
}

// <type> := <@>* <IDENTIFIER>
static AstNodeType *parse_type(Parser *this) {
    if (!check(this, TOKEN_IDENTIFIER) && !check(this, TOKEN_AT)) {
        print_token_expected_error(this, 2, TOKEN_IDENTIFIER, TOKEN_AT);
        return NULL;
    }

    int ptr = 0;
    while (match(this, TOKEN_AT)) {
        ptr++;
    }

    AstNodeIdentifier *identifier = parse_identifier(this);
    if (identifier == NULL) {
        return NULL;
    }
    return ast_node_type_create(ptr, identifier);
}

// <variable> := ( <PUBLIC> | <EXTERN> )? <VOLATILE>? <VAR> <IDENTIFIER> <:> <type> <;>
static AstNodeVariable *parse_variable(Parser *this) {
    AstNodeVariable *node = ast_node_variable_create();

    switch (peek(this, 0)->type) {
        case TOKEN_PUBLIC:
            advance(this);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(this);
            node->ext = true;
            break;
        case TOKEN_VOLATILE:
        case TOKEN_VAR:
            // keep for later
            break;
        default:
            print_token_expected_error(this, 4, TOKEN_PUBLIC, TOKEN_EXTERN, TOKEN_VOLATILE, TOKEN_VAR);
            break;
    }

    if (match(this, TOKEN_VOLATILE)) {
        node->vol = true;
    }

    if (!match(this, TOKEN_VAR)) {
        print_token_expected_error(this, 1, TOKEN_VAR);
    }

    node->name = parse_identifier(this);

    if (!match(this, TOKEN_COLON)) {
        print_token_expected_error(this, 1, TOKEN_COLON);
    }

    node->type = parse_type(this);

    // todo add expr

    if (!match(this, TOKEN_SEMICOLON)) {
        print_token_expected_error(this, 1, TOKEN_SEMICOLON);
        ast_node_variable_destroy(node);
        return NULL;
    } else if (this->error_recovery) {
        this->error_recovery = false;
        ast_node_variable_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <parameter> := <IDENTIFIER> <:> <type>
static AstNodeParameter *parse_parameter(Parser *this) {
    AstNodeIdentifier *identifier = parse_identifier(this);

    if (!match(this, TOKEN_COLON)) {
        print_token_expected_error(this, 1, TOKEN_COLON);
    }

    return ast_node_parameter_create(identifier, parse_type(this));
}

// <parameters> := ( <parameter> ( <,> <parameter> )* )?
// todo allow trailing comma
static AstNodeParameters *parse_parameters(Parser *this) {
    AstNodeParameters *node = ast_node_parameters_create();

    AstNodeParameter *parameter;
    if (!check(this, TOKEN_RPAR)) {
        parameter = parse_parameter(this);
        if (parameter != NULL) {
            list_add(node->parameters, parameter);
        }
        while (match(this, TOKEN_COMMA)) {
            parameter = parse_parameter(this);
            if (parameter != NULL) {
                list_add(node->parameters, parameter);
            }
        }
    }

    return node;
}

// <function> != ( <PUBLIC> | <EXTERN> )? <FN> <IDENTIFIER> <(> <parameters> <)> <:> <type> <{> <statements> <}>
static AstNodeFunction *parse_function(Parser *this) {
    AstNodeFunction *node = ast_node_function_create();

    switch (peek(this, 0)->type) {
        case TOKEN_PUBLIC:
            advance(this);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(this);
            node->ext = true;
            break;
        default:
            // nothing
            break;
    }

    if (!match(this, TOKEN_FN)) {
        print_token_expected_error(this, 1, TOKEN_FN);
    }

    node->name = parse_identifier(this);

    if (!match(this, TOKEN_LPAR)) {
        print_token_expected_error(this, 1, TOKEN_LPAR);
    }

    node->parameters = parse_parameters(this);

    if (!match(this, TOKEN_RPAR)) {
        print_token_expected_error(this, 2, TOKEN_COMMA, TOKEN_RPAR);
    }

    if (!match(this, TOKEN_COLON)) {
        print_token_expected_error(this, 1, TOKEN_COLON);
    }

    node->type = parse_type(this);

    if (!match(this, TOKEN_LBRACE)) { // todo may move to stmts?
        print_token_expected_error(this, 1, TOKEN_LBRACE);
    }

    node->statements = parse_stmts(this);

    if (!match(this, TOKEN_RBRACE)) {
        print_token_expected_error(this, 1, TOKEN_RBRACE);
        ast_node_function_destroy(node);
        return NULL;
    } else if (this->error_recovery) {
        this->error_recovery = false;
        ast_node_function_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <ifStmt> := <IF> <(> <expr> <)> <{> <statements> <}> ( <ELSE> ( <ifStmt> | <{> <statements> <}> ) )?
static AstNodeStmt *parse_stmt_if(Parser *this) {
    AstNodeStmt *node = ast_node_stmt_if_create();

    expect(this, TOKEN_IF);
    expect(this, TOKEN_LPAR);

    // todo expr

    expect(this, TOKEN_RPAR);
    expect(this, TOKEN_LBRACE);

    node->ifStmt->true_block = parse_stmts(this);

    expect(this, TOKEN_RBRACE);
    bool rbrace_required = false;

    if (match(this, TOKEN_ELSE)) {
        if (check(this, TOKEN_IF)) {
            node->ifStmt->false_block = ast_node_stmts_create();
            list_add(node->ifStmt->false_block->stmts, parse_stmt_if(this));
        } else if (match(this, TOKEN_LBRACE)) {
            rbrace_required = true;
            node->ifStmt->false_block = parse_stmts(this);
        } else {
            print_token_expected_error(this, 2, TOKEN_IF, TOKEN_LBRACE);
        }
    } else {
        node->ifStmt->false_block = ast_node_stmts_create();
    }

    if (rbrace_required && !match(this, TOKEN_RBRACE)) {
        print_token_expected_error(this, 1, TOKEN_RBRACE);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (this->error_recovery) {
        this->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <whileStmt> := <WHILE> <(> <expr> <)> <{> <statements> <}>
static AstNodeStmt *parse_stmt_while(Parser *this) {
    AstNodeStmt *node = ast_node_stmt_while_create();

    expect(this, TOKEN_WHILE);
    expect(this, TOKEN_LPAR);

    // todo expr

    expect(this, TOKEN_RPAR);
    expect(this, TOKEN_LBRACE);

    node->whileStmt->block = parse_stmts(this);

    if (!match(this, TOKEN_RBRACE)) {
        print_token_expected_error(this, 1, TOKEN_RBRACE);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (this->error_recovery) {
        this->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <stmt> := ( <;> | <ifStmt> | <whileStmt> )
static AstNodeStmt *parse_stmt(Parser *this) {
    switch (peek(this, 0)->type) {
        case TOKEN_SEMICOLON: // just eat it as an empty statement
            advance(this);
            return NULL;
        case TOKEN_IF:
            return parse_stmt_if(this);
        case TOKEN_WHILE:
            return parse_stmt_while(this);
        default:
            print_expected_error(this, "<statement>");
            return NULL;
    }
}

// <stmts> := <stmts>*
static AstNodeStatements *parse_stmts(Parser *this) {
    AstNodeStatements *node = ast_node_stmts_create();

    while (!check(this, TOKEN_EOF) && !check(this, TOKEN_RBRACE)) {
        AstNodeStmt *stmt = parse_stmt(this);
        if (stmt != NULL) {
            list_add(node->stmts, stmt);
        }
    }

    return node;
}

// <const> := ( <PUBLIC> | <EXTERN> )? <CONST> <IDENTIFIER> <:> <type> <=> <expr> <;>
static AstNodeConst *parse_const(Parser *this) {
    AstNodeConst *node = ast_node_const_create();

    switch (peek(this, 0)->type) {
        case TOKEN_PUBLIC:
            advance(this);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(this);
            node->ext = true;
            break;
        default:
            // nothing
            break;
    }

    if (!match(this, TOKEN_CONST)) {
        print_token_expected_error(this, 1, TOKEN_CONST);
    }

    node->name = parse_identifier(this);

    if (!match(this, TOKEN_COLON)) {
        print_token_expected_error(this, 1, TOKEN_COLON);
    }

    node->type = parse_type(this);

    if (!match(this, TOKEN_EQUAL)) {
        print_token_expected_error(this, 1, TOKEN_EQUAL);
    }

    // todo expr

    if (!match(this, TOKEN_SEMICOLON)) {
        print_token_expected_error(this, 1, TOKEN_SEMICOLON);
        ast_node_const_destroy(node);
        return NULL;
    } else if (this->error_recovery) {
        this->error_recovery = false;
        ast_node_const_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

static bool is_at_end(Parser *this) {
    return peek(this, 0)->type == TOKEN_EOF;
}

static Token *peek(Parser *this, int n) {
    for (size_t i = queue_size(this->tokens); i < n + 1; i++) {
        Token token = lexer_next(this->lexer);
        Token *t = malloc(sizeof(Token));
        memcpy(t, &token, sizeof(Token));
        queue_enqueue(this->tokens, t);
    }
    return queue_peek(this->tokens, n);
}

static Token advance(Parser *this) {
    if (queue_is_empty(this->tokens)) {
        return lexer_next(this->lexer);
    }

    Token *token_ptr = queue_dequeue(this->tokens);
    Token token;
    memcpy(&token, token_ptr, sizeof(Token));
    free(token_ptr);

    return token;
}

static bool check(Parser *this, ETokenType expected) {
    return (peek(this, 0)->type == expected);
}

static bool check_within(Parser *this, ETokenType expected, int n) {
    for (int i = 0; i < n; i++) {
        if (peek(this, i)->type == expected) {
            return true;
        }
    }
    return false;
}

static bool match(Parser *this, ETokenType expected) {
    if (is_at_end(this) || !check(this, expected)) {
        return false;
    }
    advance(this);
    return true;
}

static void expect(Parser *this, ETokenType expected) {
    if (!match(this, expected)) {
        print_token_expected_error(this, 1, expected);
    }
}