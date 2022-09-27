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
#include <stdarg.h>
#include <stdlib.h>
#include "macros.h"
#include "parser.h"
#include "lexer.h"
#include "memory.h"
#include "queue.h"
#include "list.h"

typedef struct Parser {
    Lexer *lexer;
    Queue *tokens;
    bool error_found;
    bool error_recovery;
} Parser;

#pragma region helpers

/**
 * Returns a pointer to the nth next token.
 * @param self the parser instance.
 * @param n the index of the next token to return; starts at 0.
 * @return the nth next token, NULL if it does not exist.
 */
static Token *peek(Parser *self, int n) {
    for (size_t i = queue_size(self->tokens); i < n + 1; i++) {
        Token token = lexer_next(self->lexer);
        Token *t = memory_alloc(sizeof(Token));
        memcpy(t, &token, sizeof(Token));
        queue_enqueue(self->tokens, t);
    }
    return queue_peek(self->tokens, n);
}

/**
 * Returns whether the end of the token stream is reached.
 * @param self the parser insrance
 * @return true when no tokens are available anymore.
 */
static bool is_at_end(Parser *self) {
    return peek(self, 0)->type == TOKEN_EOF;
}

/**
 * Consumes and returns the next token. In case there is no next token, returns an EOF token.
 * @param self the parser instance.
 * @return the next token, or an EOF token is no next token exists.
 */
static Token advance(Parser *self) {
    if (queue_is_empty(self->tokens)) {
        return lexer_next(self->lexer);
    }

    Token *token_ptr = queue_dequeue(self->tokens);
    Token token;
    memcpy(&token, token_ptr, sizeof(Token));
    memory_free(token_ptr);

    return token;
}

/**
 * Returns whether the next token is of the expected type without consuming it.
 * @param self the parser instance.
 * @param expected the expected next token type.
 * @return true if the next token is of the expected type; false otherwise.
 */
static bool check(Parser *self, ETokenType expected) {
    return (peek(self, 0)->type == expected);
}

/**
 * Returns whether the next token is any of the expected types without consuming it.
 * @param self the parser instance.
 * @param tokens the count of possible tokens
 * @return true if the next token is of the expected type; false otherwise.
 */
static bool check_any_va_args(Parser *self, size_t tokens, ...) {
    va_list argp;
    va_start(argp, tokens);

    for (size_t i = 0; i < tokens; i++) {
        ETokenType token = va_arg(argp, ETokenType);
        if (check(self, token)) {
            return true;
        }
    }

    return false;
}

#define check_any(self, ...) check_any_va_args((self), num_args(__VA_ARGS__), __VA_ARGS__)

/**
 * Returns whether any of the n next tokens is of the expected type.
 * @param self the parser instance.
 * @param expected the expected token type.
 * @param n the amount of next tokens to check.
 * @return true if any of the next n tokens is of the expected type; false otherwise.
 */
static bool check_within(Parser *self, ETokenType expected, int n) {
    for (int i = 0; i < n; i++) {
        if (peek(self, i)->type == expected) {
            return true;
        }
    }
    return false;
}

/**
 * Returns whether the next token is of the expected type. If it is, it consumes it, otherwise it does not.
 * @param self
 * @param expected
 * @return true if the next token is of the expected type; false otherwise.
 */
static bool match(Parser *self, ETokenType expected) {
    if (is_at_end(self) || !check(self, expected)) {
        return false;
    }
    advance(self);
    return true;
}

static void print_token_expected_error_va_args(Parser *self, size_t tokens, ...) {
    if (self->error_recovery) {
        return;
    }

    self->error_found = true;
    self->error_recovery = true;

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
        fprintf(stderr, " ) ");
    }
    Token *token = peek(self, 0);
    fprintf(stderr, " but got <%s> at %i:%i\n", token_type_to_string(token->type), token->line, token->column);
}

#define print_token_expected_error(self, ...) print_token_expected_error_va_args((self), num_args(__VA_ARGS__), __VA_ARGS__)

/**
 * Triggers an error in case the next token is not of the expected type.
 * @param self the parser instance.
 * @param expected the next token expected type.
 */
static void expect(Parser *self, ETokenType expected) {
    if (!match(self, expected)) {
        print_token_expected_error(self, expected);
    }
}

#define expect_any(self, ...) \
    {                                   \
        size_t n_args = num_args(__VA_ARGS__); \
        if (!check_any_va_args((self), n_args, __VA_ARGS__)) { \
            print_token_expected_error_va_args((self), n_args, __VA_ARGS__); \
        }                                   \
    }

static void print_expected_error(Parser *self, const char *expected) {
    if (self->error_recovery) {
        return;
    }

    self->error_found = true;
    self->error_recovery = true;

    Token *token = peek(self, 0);
    fprintf(stderr, "Expected %s at %i:%i\n", expected, token->line, token->column);
}

static void *parser_memory_alloc(size_t size) {
    return memory_alloc(size);
}

static void parser_memory_free(void *ptr) {
    memory_free(ptr);
}

#pragma endregion

#pragma region parsing

//rule <identifier> := <IDENTIFIER>
static AstNodeIdentifier *parse_identifier(Parser *self) {
    if (!check(self, TOKEN_IDENTIFIER)) {
        print_token_expected_error(self, TOKEN_IDENTIFIER);
        return NULL;
    }

    return ast_node_identifier_create(advance(self));
}

//rule <type> := <@>* <identifier>
static AstNodeType *parse_type(Parser *self) {
    Token *token = peek(self, 0);

    if (token->type != TOKEN_BYTE &&
        token->type != TOKEN_WORD &&
        token->type != TOKEN_IDENTIFIER &&
        token->type != TOKEN_AT) {
        print_token_expected_error(self, 4, TOKEN_WORD, TOKEN_BYTE, TOKEN_IDENTIFIER, TOKEN_AT);
        return NULL;
    }

    Token *token = peek(self, 0);
    int line = token->line;
    int column = token->column;

    int ptr = 0;
    while (match(self, TOKEN_AT)) {
        ptr++;
    }

    AstNodeIdentifier *identifier = NULL;
    token = peek(self, 0);
    switch (token->type) {
        case TOKEN_IDENTIFIER:
        case TOKEN_WORD:
        case TOKEN_BYTE:
            identifier = ast_node_identifier_create(*token);
            advance(self);
            break;
        default:
            print_token_expected_error(self, TOKEN_WORD, TOKEN_BYTE, TOKEN_IDENTIFIER, TOKEN_AT);
            break;
    }

    if (identifier == NULL) {
        return NULL;
    }
    return ast_node_type_create(ptr, identifier, line, column);
}

//rule <number> := <NUMBER>
static AstNodeNumber *parse_number(Parser *self) {
    if (!check(self, TOKEN_NUMBER)) {
        print_token_expected_error(self, TOKEN_NUMBER);
        return NULL;
    }

    return ast_node_number_create(advance(self));
}

typedef enum EPrecedence {
    PREC_LOWEST = 1,
    PREC_EQ_CMP = 2,
    PREC_ORDER_CMP = 3,
    PREC_SUM = 4,
    PREC_PRODUCT = 5,
    PREC_PREFIX = 6,
    PREC_CALL = 7,
} EPrecedence;

static EPrecedence get_precedence(EOperator op) {
    switch (op) {
        case OPERATOR_plus:
        case OPERATOR_minus:
            return PREC_SUM;
        case OPERATOR_star:
        case OPERATOR_slash:
            return PREC_PRODUCT;
        case OPERATOR_equals:
        case OPERATOR_not_equals:
            return PREC_EQ_CMP;
        case OPERATOR_lt:
        case OPERATOR_gt:
        case OPERATOR_lt_equals:
        case OPERATOR_gt_equals:
            return PREC_ORDER_CMP;
        case OPERATOR_exclam:
        case OPERATOR_amp:
        case OPERATOR_at:
            return PREC_PREFIX;
        default:
            printf("Unsupported operator %d\n", op);
            exit(1); // todo check error behavior
    }
}

static EOperator convert_token_to_operator(Token *token) {
    switch (token->type) {
        case TOKEN_PLUS:
            return OPERATOR_plus;
        case TOKEN_MINUS:
            return OPERATOR_minus;
        case TOKEN_STAR:
            return OPERATOR_star;
        case TOKEN_EQUALS:
            return OPERATOR_equals;
        case TOKEN_NOT_EQUALS:
            return OPERATOR_not_equals;
        case TOKEN_GT:
            return OPERATOR_gt;
        case TOKEN_GT_EQUALS:
            return OPERATOR_gt_equals;
        case TOKEN_LT:
            return OPERATOR_lt;
        case TOKEN_LT_EQUALS:
            return OPERATOR_lt_equals;
        default:
            // todo die
            return OPERATOR_plus;
    }
}

#define OPERATORS TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, \
    TOKEN_EQUALS, TOKEN_NOT_EQUALS, \
    TOKEN_GT, TOKEN_LT, TOKEN_GT_EQUALS, TOKEN_LT_EQUALS

inline static bool next_token_is_operator(Parser *self) {
    return check_any(self, OPERATORS);
}

static AstNodeExpression *parse_infix_expression(Parser *self, AstNodeExpression *left);

//rule <expression> := <number> | <identifier> | ( <number> | <identifier> ) <infixExpression>
static AstNodeExpression *parse_expression(Parser *self, EPrecedence precedence) {
    AstNodeExpression *expression = NULL;

    Token *token = peek(self, 0);
    switch (token->type) {
        case TOKEN_NUMBER:
            expression = ast_node_expression_create();
            expression->kind = EXPRESSION_NUMBER;
            expression->number_expression = parse_number(self);
            break;
        case TOKEN_IDENTIFIER:
            expression = ast_node_expression_create();
            expression->kind = EXPRESSION_IDENTIFIER;
            expression->identifier_expression = parse_identifier(self);
            break;
        default:
            print_token_expected_error(self, TOKEN_NUMBER, TOKEN_IDENTIFIER);
    }

    if (self->error_recovery) {
        if (expression != NULL) {
            ast_node_expression_destroy(expression);
        }
        return NULL;
    } else {
        while (next_token_is_operator(self) && precedence < get_precedence(convert_token_to_operator(peek(self, 0)))) {
            if (self->error_recovery) {
                return NULL;
            }
            expression = parse_infix_expression(self, expression);
        }

        return expression;
    }
}

//rule <operator> := <+> | <-> | <*> | <==> | <!=> | <>> | <<> | <>=> | <<=>
static AstNodeOperator *parse_operator(Parser *self) {
    expect_any(self, OPERATORS);

    Token token = advance(self);
    AstNodeOperator *node = ast_node_operator_create(convert_token_to_operator(&token));

    node->base.start_line = token.line;
    node->base.start_column = token.column;

    if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_operator_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

//rule <infixExpression> := <operator> <expression>
static AstNodeExpression *parse_infix_expression(Parser *self, AstNodeExpression *left) {
    AstNodeOperator *operator = parse_operator(self);
    AstNodeExpression *right = parse_expression(self, get_precedence(operator->op));

    if (operator == NULL || right == NULL) {
        if (left != NULL) {
            ast_node_expression_destroy(left);
        }
        if (operator != NULL) {
            ast_node_operator_destroy(operator);
        }
        if (right != NULL) {
            ast_node_expression_destroy(right);
        }
        return NULL;
    } else {
        AstNodeExpression *expression = ast_node_expression_create();
        expression->kind = EXPRESSION_BINARY;
        expression->binary_expression = ast_node_binary_expression_create(left, right, operator);

        return expression;
    }
}

//rule <variable> := ( <PUBLIC> | <EXTERN> )? <VOLATILE>? <VAR> <identifier> <:> <type> <;>
static AstNodeVariable *parse_variable(Parser *self) {
    if (!check_within(self, TOKEN_VAR, 3)) {
        return NULL;
    }
    AstNodeVariable *node = ast_node_variable_create();

    Token *token = peek(self, 0);
    node->base.start_line = token->line;
    node->base.start_column = token->column;

    switch (token->type) {
        case TOKEN_PUBLIC:
            advance(self);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(self);
            node->ext = true;
            break;
        case TOKEN_VOLATILE:
        case TOKEN_VAR:
            // keep for later
            break;
        default:
            print_token_expected_error(self, TOKEN_PUBLIC, TOKEN_EXTERN, TOKEN_VOLATILE, TOKEN_VAR);
            break;
    }

    if (match(self, TOKEN_VOLATILE)) {
        node->vol = true;
    }

    if (!match(self, TOKEN_VAR)) {
        print_token_expected_error(self, TOKEN_VAR);
    }

    node->name = parse_identifier(self);

    if (!match(self, TOKEN_COLON)) {
        print_token_expected_error(self, TOKEN_COLON);
    }

    node->type = parse_type(self);

    if (match(self, TOKEN_EQUAL)) {
        node->expression = parse_expression(self, PREC_LOWEST);
    }

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, TOKEN_SEMICOLON);
        ast_node_variable_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_variable_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

//rule <const> := ( <PUBLIC> | <EXTERN> )? <CONST> <IDENTIFIER> <:> <type> <=> <expr> <;>
static AstNodeConst *parse_const(Parser *self) {
    if (!check_within(self, TOKEN_CONST, 2)) {
        return NULL;
    }
    AstNodeConst *node = ast_node_const_create();

    Token *token = peek(self, 0);
    node->base.start_line = token->line;
    node->base.start_column = token->column;

    switch (token->type) {
        case TOKEN_PUBLIC:
            advance(self);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(self);
            node->ext = true;
            break;
        default:
            // nothing
            break;
    }

    if (!match(self, TOKEN_CONST)) {
        print_token_expected_error(self, TOKEN_CONST);
    }

    node->name = parse_identifier(self);

    if (!match(self, TOKEN_COLON)) {
        print_token_expected_error(self, TOKEN_COLON);
    }

    node->type = parse_type(self);

    if (!match(self, TOKEN_EQUAL)) {
        print_token_expected_error(self, TOKEN_EQUAL);
    }

    node->expression = parse_expression(self, PREC_LOWEST);

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, TOKEN_SEMICOLON);
        ast_node_const_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_const_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

//rule <parameter> := <IDENTIFIER> <:> <type>
static AstNodeParameter *parse_parameter(Parser *self) {
    Token *token = peek(self, 0);
    int line = token->line;
    int column = token->column;

    AstNodeIdentifier *identifier = parse_identifier(self);

    if (!match(self, TOKEN_COLON)) {
        print_token_expected_error(self, TOKEN_COLON);
    }

    return ast_node_parameter_create(identifier, parse_type(self), line, column);
}

//rule <parameters> := <(> ( <parameter> ( <,> <parameter> )* <,>? )? <)>
static AstNodeParameters *parse_parameters(Parser *self) {
    if (!match(self, TOKEN_LPAR)) {
        print_token_expected_error(self, TOKEN_LPAR);
        return NULL;
    }

    AstNodeParameters *node = ast_node_parameters_create();
    Token *token = peek(self, 0);
    node->base.start_line = token->line;
    node->base.start_column = token->column;

    AstNodeParameter *parameter;
    if (token->type != TOKEN_RPAR) {
        parameter = parse_parameter(self);
        if (parameter != NULL) {
            list_add(node->parameters, parameter);
        }
        while (match(self, TOKEN_COMMA)) {
            parameter = parse_parameter(self);
            if (parameter != NULL) {
                list_add(node->parameters, parameter);
            }
        }
    }

    match(self, TOKEN_COMMA); // eat the last comma, if any

    if (!match(self, TOKEN_RPAR)) {
        print_token_expected_error(self, TOKEN_COMMA, TOKEN_RPAR);
    }

    return node;
}

static AstNodeStmt *parse_stmt(Parser *self);

//rule <stmts> := <{> <stmt>* <}>
static AstNodeStatements *parse_stmts(Parser *self) {
    if (!match(self, TOKEN_LBRACE)) {
        print_token_expected_error(self, TOKEN_LBRACE);
        return NULL;
    }

    AstNodeStatements *node = ast_node_stmts_create();
    Token *token = peek(self, 0);
    node->base.start_line = token->line;
    node->base.start_column = token->column;

    if (match(self, TOKEN_RBRACE)) {
        return node;
    }

    AstNodeStmt *stmt = parse_stmt(self);
    while (stmt != NULL) {
        list_add(node->stmts, stmt);
        if (match(self, TOKEN_RBRACE)) {
            break;
        }
        stmt = parse_stmt(self);
    }

    return node;
}

//rule <function> := ( <PUBLIC> | <EXTERN> )? <FN> <IDENTIFIER> <parameters> <:> <type> <stmts>
static AstNodeFunction *parse_function(Parser *self) {
    if (!check_within(self, TOKEN_FN, 2)) {
        return NULL;
    }
    AstNodeFunction *node = ast_node_function_create();

    Token *token = peek(self, 0);
    node->base.start_line = token->line;
    node->base.start_column = token->column;

    switch (token->type) {
        case TOKEN_PUBLIC:
            advance(self);
            node->pub = true;
            break;
        case TOKEN_EXTERN:
            advance(self);
            node->ext = true;
            break;
        default:
            // nothing
            break;
    }

    if (!match(self, TOKEN_FN)) {
        print_token_expected_error(self, TOKEN_FN);
    }

    node->name = parse_identifier(self);

    node->parameters = parse_parameters(self);

    if (!match(self, TOKEN_COLON)) {
        print_token_expected_error(self, TOKEN_COLON);
    }
    node->type = parse_type(self);

    node->statements = parse_stmts(self);

    if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_function_destroy(node);
        return NULL;
    }

    return node;
}

//rule <stmt_const> := <CONST> <identifier> <:> <type> <=> <expr> <;>
static AstNodeStmt *parse_stmt_const(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_const_create();

    Token *token = peek(self, 0);
    ((AstNode *) node->const_stmt)->start_line = token->line;
    ((AstNode *) node->const_stmt)->start_column = token->column;

    expect(self, TOKEN_CONST);
    node->const_stmt->identifier = parse_identifier(self);
    expect(self, TOKEN_COLON);
    node->const_stmt->type = parse_type(self);
    expect(self, TOKEN_EQUAL);

    node->const_stmt->expression = parse_expression(self, PREC_LOWEST);

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, TOKEN_SEMICOLON);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

//rule <stmt_var> := <VAR> <identifier> <:> <type> ( <=> <expr> )? <;>
static AstNodeStmt *parse_stmt_var(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_var_create();

    Token *token = peek(self, 0);
    ((AstNode *) node->var_stmt)->start_line = token->line;
    ((AstNode *) node->var_stmt)->start_column = token->column;

    expect(self, TOKEN_VAR);
    node->var_stmt->identifier = parse_identifier(self);
    expect(self, TOKEN_COLON);
    node->var_stmt->type = parse_type(self);

    if (match(self, TOKEN_EQUAL)) {
        node->var_stmt->expression = parse_expression(self, PREC_LOWEST);
    }

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, TOKEN_SEMICOLON);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

//rule <stmt_assignment> := <identifier> <=> <expr> <;>
static AstNodeStmt *parse_stmt_assignment(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_assignment_create();

    Token *token = peek(self, 0);
    ((AstNode *) node->assignment_stmt)->start_line = token->line;
    ((AstNode *) node->assignment_stmt)->start_column = token->column;

    node->assignment_stmt->identifier = parse_identifier(self);

    expect(self, TOKEN_EQUAL);

    // todo expr

    if (!match(self, TOKEN_SEMICOLON)) {
        print_token_expected_error(self, TOKEN_SEMICOLON);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

// <stmt_if> := <IF> <(> <expr> <)> <{> <stmts> <}> ( <ELSE> ( <stmt_if> | <{> <stmts> <}> ) )?
static AstNodeStmt *parse_stmt_if(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_if_create();

    Token *token = peek(self, 0);
    ((AstNode *) node->if_stmt)->start_line = token->line;
    ((AstNode *) node->if_stmt)->start_column = token->column;

    expect(self, TOKEN_IF);
    expect(self, TOKEN_LPAR);

    // todo expr

    expect(self, TOKEN_RPAR);
    expect(self, TOKEN_LBRACE);

    node->if_stmt->true_block = parse_stmts(self);

    expect(self, TOKEN_RBRACE);
    bool rbrace_required = false;

    if (match(self, TOKEN_ELSE)) {
        if (check(self, TOKEN_IF)) {
            node->if_stmt->false_block = ast_node_stmts_create();
            list_add(node->if_stmt->false_block->stmts, parse_stmt_if(self));
        } else if (match(self, TOKEN_LBRACE)) {
            rbrace_required = true;
            node->if_stmt->false_block = parse_stmts(self);
        } else {
            print_token_expected_error(self, TOKEN_IF, TOKEN_LBRACE);
        }
    } else {
        node->if_stmt->false_block = ast_node_stmts_create();
    }

    if (rbrace_required && !match(self, TOKEN_RBRACE)) {
        print_token_expected_error(self, TOKEN_RBRACE);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

//rule <stmt_while> := <WHILE> <(> <expr> <)> <{> <stmts> <}>
static AstNodeStmt *parse_stmt_while(Parser *self) {
    AstNodeStmt *node = ast_node_stmt_while_create();

    Token *token = peek(self, 0);
    ((AstNode *) node->while_stmt)->start_line = token->line;
    ((AstNode *) node->while_stmt)->start_column = token->column;

    expect(self, TOKEN_WHILE);
    expect(self, TOKEN_LPAR);

    // todo expr

    expect(self, TOKEN_RPAR);
    expect(self, TOKEN_LBRACE);

    node->while_stmt->block = parse_stmts(self);

    if (!match(self, TOKEN_RBRACE)) {
        print_token_expected_error(self, TOKEN_RBRACE);
        ast_node_stmt_destroy(node);
        return NULL;
    } else if (self->error_recovery) {
        self->error_recovery = false;
        ast_node_stmt_destroy(node);
        return NULL;
    } else {
        return node;
    }
}

//rule <stmt> := <;> | <stmt_const> | <stmt_var> | <stmt_assignment> | <stmt_if> | <stmt_while>
static AstNodeStmt *parse_stmt(Parser *self) {
    switch (peek(self, 0)->type) {
        case TOKEN_SEMICOLON: // just eat it as an empty statement
            advance(self);
            return NULL;
        case TOKEN_CONST:
            return parse_stmt_const(self);
        case TOKEN_VAR:
            return parse_stmt_var(self);
        case TOKEN_IDENTIFIER:
            return parse_stmt_assignment(self);
        case TOKEN_IF:
            return parse_stmt_if(self);
        case TOKEN_WHILE:
            return parse_stmt_while(self);
        default:
            print_token_expected_error(self, TOKEN_SEMICOLON, TOKEN_CONST, TOKEN_VAR, TOKEN_IDENTIFIER,
                                             TOKEN_IF, TOKEN_WHILE);
            return NULL;
    }
}

#pragma endregion

#pragma region public

Parser *parser_create(Lexer *lexer) {
    Parser *parser = memory_alloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->tokens = queue_create(
            4,
            .malloc = parser_memory_alloc,
            .free = parser_memory_free
    );
    parser->error_recovery = false;
    parser->error_found = false;
    return parser;
}

void parser_destroy(Parser *self) {
    while (!queue_is_empty(self->tokens)) {
        memory_free(queue_dequeue(self->tokens));
    }
    queue_destroy(self->tokens);
    memory_free(self);
}

AstRoot *parser_parse(Parser *self) {
    AstRoot *root = ast_root_create();

    while (peek(self, 0)->type != TOKEN_EOF) {
        while (match(self, TOKEN_SEMICOLON)) {
            // nothing
        }

        // here, we try to parse a function, a variable or a const by calling each parse functions sequentially.
        // we restart the loop as soon as one returned something; if all returned NULL we have an error.

        AstNodeVariable *variable = parse_variable(self);
        if (variable != NULL) {
            list_add(root->variables, variable);
            continue;
        }

        AstNodeFunction *function = parse_function(self);
        if (function != NULL) {
            list_add(root->functions, function);
            continue;
        }

        AstNodeConst *constant = parse_const(self);
        if (constant != NULL) {
            list_add(root->constants, constant);
            continue;
        }

        print_expected_error(self, "constant, variable or function definition");
        advance(self);
    }

    if (self->error_found) {
        ast_root_destroy(root);
        return NULL;
    } else {
        return root;
    }
}

#pragma endregion

