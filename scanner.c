#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct 
{
    const char* start_current_lexeme;
    const char* current_pos;
    int current_src_code_line;
} Scanner;

static Scanner scanner; // TODO: global var no good (quick fixed with static but still bad)

static bool isAtEnd()
{
    return *scanner.current_pos == '\0';
}

static Token makeToken(TokenType type)
{
    Token token;
    token.type = type;
    token.lexeme_start = scanner.start_current_lexeme;
    token.lexeme_length = (int)(scanner.current_pos - scanner.start_current_lexeme);
    token.src_code_line = scanner.current_src_code_line;
    return token;
}

static Token errorToken(const char* message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme_start = message;
    token.lexeme_length = (int)strlen(message);
    token.src_code_line = scanner.current_src_code_line;
    return token;
}

static char advance()
{
	return *scanner.current_pos++;
}

static bool match(const char expected)
{
    if (isAtEnd()) return false;
    if (*scanner.current_pos != expected) return false;
    scanner.current_pos++;
    return true;
}

static void skipWhiteSpace()
{
    char c = *scanner.current_pos; // could be peek - but seems like unnecessary function call?
    while(c == ' ' || c == '\r' || c == '\t' || c == '\n')
    {
	if (c == '\n') scanner.current_src_code_line++;
	advance();
	c = *scanner.current_pos; // could be peek - but seems like unnecessary function call?
    }
    return;
}

static char peek()
{
    // TODO: should we even use this function?
    // Could be a macro?
    // Or will the compiler optimize this out if we use it?
    // All these unanswered questions
    return *scanner.current_pos;
}

void initScanner(const char* source)
{
    scanner.start_current_lexeme = source;
    scanner.current_pos = source;
    scanner.current_src_code_line = 1;
}

Token scanToken()
{
    skipWhiteSpace();
    scanner.start_current_lexeme = scanner.current_pos;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();

    switch (c)
    {
		// one-character tokens
		case '(': return makeToken(TOKEN_LEFT_PAREN);
		case ')': return makeToken(TOKEN_RIGHT_PAREN);
		case '{': return makeToken(TOKEN_LEFT_BRACE);
		case '}': return makeToken(TOKEN_RIGHT_BRACE);
		case ';': return makeToken(TOKEN_SEMICOLON);
		case ',': return makeToken(TOKEN_COMMA);
		case '.': return makeToken(TOKEN_DOT);
		case '-': return makeToken(TOKEN_MINUS);
		case '+': return makeToken(TOKEN_PLUS);
		case '/': return makeToken(TOKEN_SLASH);
		case '*': return makeToken(TOKEN_STAR);
		// two-character tokens
		case '!': return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
		case '=': return makeToken(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
		case '<': return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
		case '>': return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    }

    return errorToken("Unexpected character.");
}
