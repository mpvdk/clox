#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "scanner.h"
#include "value.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct
{
    Token current;      // Token being parsed
    Token previous;     // Last token parsed
    bool hadError;
    bool panicMode;
} Parser;

typedef enum 
{
    PREC_NONE,
    PREC_ASSIGNMENT,    // =
    PREC_OR,            // or
    PREC_AND,           // and
    PREC_EQUALITY,      // == !=
    PREC_COMPARISON,    // < > <= >=
    PREC_TERM,          // + -
    PREC_FACTOR,        // * /
    PREC_UNARY,         // ! -
    PREC_CALL,          // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)();

typedef struct 
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

static void expression();
static void statement();
static void declaration();
static ParseRule* getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static Parser parser; // TODO: global no good
static Chunk* compilingChunk; // TODO: global no good

static Chunk* currentChunk()
{
    return compilingChunk; 
}

static void errorAt(Token* token, const char* message)
{
    if (parser.panicMode) return;
    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error", token->src_code_line);
    
    if (token->type == TOKEN_EOF)
    {
            fprintf(stderr, " at end");
    }
    else if (token->type == TOKEN_ERROR)
    {
        // nothing
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->lexeme_length, token->lexeme_start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char* message)
{
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char* message)
{
    errorAt(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;
    
    while(1)
    {
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR) break;
        // below is bit confusing because 'lexeme_start' for an 
        // error token just points to a string literal in static 
        // memory. It is null-teriminated. It doesn't point 
        // to the source code like this member var usually does.
        errorAtCurrent(parser.current.lexeme_start); 
    }
}

static void consume(TokenType type, const char* message)
{
    if (parser.current.type == type)
    {
        advance();
        return;
    }
    errorAtCurrent(message);
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

static bool match(TokenType type)
{
    if (!check(type)) return false;
    advance();
    return true;
}

static void emitByte(uint8_t byte)
{
    writeChunk(currentChunk(), byte, parser.previous.src_code_line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    // TODO make generic? Like argc & argv?
    // so we don't need two functions (emitByte & emitBytes)
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn()
{
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value)
{
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX)
    {
        error("Too many constants in one chunk");
        return 0;
    }
    return (uint8_t)constant;
}

static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler()
{
    emitReturn();
#ifdef DEBUG_PRINT_CODE
if (!parser.hadError)
{
    disassembleChunk(currentChunk(), "code");
}
#endif
}

static void binary()
{
    TokenType operatorType = parser.previous.type;
    ParseRule* rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType)
    {
        case TOKEN_BANG_EQUAL:      emitBytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:     emitByte(OP_EQUAL); break;
        case TOKEN_GREATER:         emitByte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL:   emitBytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:            emitByte(OP_LESS); break;
        case TOKEN_LESS_EQUAL:      emitBytes(OP_GREATER, OP_NOT); break;
        case TOKEN_MINUS:           emitByte(OP_SUBTRACT); break;
        case TOKEN_PLUS:            emitByte(OP_ADD); break;
        case TOKEN_SLASH:           emitByte(OP_DIVIDE); break;
        case TOKEN_STAR:            emitByte(OP_MULTIPLY); break;
        default: return;
    }
}

static void literal()
{
    switch (parser.previous.type)
    {
        case TOKEN_TRUE: emitByte(OP_TRUE); break;
        case TOKEN_FALSE: emitByte(OP_FALSE); break;
        case TOKEN_NIL: emitByte(OP_NIL); break;
        default: return;
    }
}

static void grouping()
{
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void number()
{
    double value = strtod(parser.previous.lexeme_start, NULL);
    emitConstant(NUMBER_VAL(value));
}

static void unary()
{
    TokenType operatorType = parser.previous.type;
    parsePrecedence(PREC_UNARY); // compile operand
    switch (operatorType)
    {
        case TOKEN_BANG: emitByte(OP_NOT); break;
        case TOKEN_MINUS: emitByte(OP_NEGATE); break;
        default: return;
    }
}

ParseRule rules[] = {
  [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_CALL},
  [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE}, // [big]
  [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
  [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_DOT]           = {NULL,     NULL,    PREC_CALL},
  [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
  [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
  [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
  [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
  [TOKEN_BANG]          = {unary,    NULL,   PREC_NONE},
  [TOKEN_BANG_EQUAL]    = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EQUAL_EQUAL]   = {NULL,     binary, PREC_EQUALITY},
  [TOKEN_GREATER]       = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_GREATER_EQUAL] = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS]          = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_LESS_EQUAL]    = {NULL,     binary, PREC_COMPARISON},
  [TOKEN_IDENTIFIER]    = {NULL, NULL,   PREC_NONE},
  [TOKEN_STRING]        = {NULL,   NULL,   PREC_NONE},
  [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
  [TOKEN_AND]           = {NULL,     NULL,   PREC_AND},
  [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FALSE]         = {literal,  NULL,   PREC_NONE},
  [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
  [TOKEN_NIL]           = {literal,  NULL,   PREC_NONE},
  [TOKEN_OR]            = {NULL,     NULL,    PREC_OR},
  [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
  [TOKEN_SUPER]         = {NULL,   NULL,   PREC_NONE},
  [TOKEN_THIS]          = {NULL,    NULL,   PREC_NONE},
  [TOKEN_TRUE]          = {literal,  NULL,   PREC_NONE},
  [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
  [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
  [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

//ParseRule rules[] = {
//    [TOKEN_AND]           = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_BANG]          = {unary,     NULL,   PREC_NONE},
//    [TOKEN_BANG_EQUAL]    = {binary,    NULL,   PREC_NONE},
//    [TOKEN_CLASS]         = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_COMMA]         = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_DOT]           = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_ELSE]          = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_EOF]           = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_EQUAL]         = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_EQUAL_EQUAL]   = {binary,    NULL,   PREC_NONE},
//    [TOKEN_ERROR]         = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_FALSE]         = {literal,   NULL,   PREC_NONE},
//    [TOKEN_FOR]           = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_FUN]           = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_GREATER]       = {binary,    NULL,   PREC_NONE},
//    [TOKEN_GREATER_EQUAL] = {binary,    NULL,   PREC_NONE},
//    [TOKEN_IDENTIFIER]    = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_IF]            = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_LEFT_BRACE]    = {NULL,      NULL,   PREC_NONE}, 
//    [TOKEN_LEFT_PAREN]    = {grouping,  NULL,   PREC_NONE},
//    [TOKEN_LESS]          = {binary,    NULL,   PREC_NONE},
//    [TOKEN_LESS_EQUAL]    = {binary,    NULL,   PREC_NONE},
//    [TOKEN_MINUS]         = {unary,     binary, PREC_TERM},
//    [TOKEN_NIL]           = {literal,   NULL,   PREC_NONE},
//    [TOKEN_NUMBER]        = {number,    NULL,   PREC_NONE},
//    [TOKEN_OR]            = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_PLUS]          = {NULL,      binary, PREC_TERM},
//    [TOKEN_PRINT]         = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_RETURN]        = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_RIGHT_PAREN]   = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_RIGHT_BRACE]   = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_SEMICOLON]     = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_SLASH]         = {NULL,      binary, PREC_FACTOR},
//    [TOKEN_STAR]          = {NULL,      binary, PREC_FACTOR},
//    [TOKEN_STRING]        = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_SUPER]         = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_THIS]          = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_TRUE]          = {literal,   NULL,   PREC_NONE},
//    [TOKEN_VAR]           = {NULL,      NULL,   PREC_NONE},
//    [TOKEN_WHILE]         = {NULL,      NULL,   PREC_NONE},
//};

static void parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }

    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance();
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static void expression()
{
    parsePrecedence(PREC_ASSIGNMENT);
}

static ParseRule* getRule(TokenType type)
{
    return &rules[type];
}

bool compile(const char* source, Chunk* chunk)
{
    initScanner(source);
    compilingChunk = chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    return !parser.hadError;
}
