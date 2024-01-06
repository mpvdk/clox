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

static void skipWhiteSpaceAndComments()
{
    while(1) {
	char c = *scanner.current_pos;
	switch (c) {
	    // whitespace
	    case ' ':
	    case '\r':
	    case '\t':
		advance();
		break;
	    // newline
	    case '\n':
		scanner.current_src_code_line++;
		advance();
		break;
	    // comment
	    case '/':
		if (!isAtEnd() && scanner.current_pos[1] == '/') {
		    // A comment goes until the end of the line.
		    while (*scanner.current_pos != '\n' && !isAtEnd()) advance();
		} else {
		    return;
		}
		break;
	    default:
		return;
	}
    }
}

static Token string()
{
    while (*scanner.current_pos != '"' && !isAtEnd())
    {
	if (*scanner.current_pos == '\n') scanner.current_src_code_line++;
	advance();
    }

    if (isAtEnd()) return errorToken("Unterminated string.");

    advance(); // closing quote
    return makeToken(TOKEN_STRING);
}

static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z' 
	|| c >= 'A' && c <= 'Z' 
	|| c == '_');
}

static char peek()
{
    return *scanner.current_pos;
}

static char peekNext()
{
    if (isAtEnd()) return '\0';
    return scanner.current_pos[1];
}

static Token number()
{
    // consume all whole numbers
    while (isDigit(*scanner.current_pos)) advance();

    // Check for decimal fraction
    if (*scanner.current_pos == '.' && isDigit(peekNext()))
    {
	// consume decimal point
	advance();
	// and all fractional numbers
	while (isDigit(*scanner.current_pos)) advance();
    }
    
    return makeToken(TOKEN_NUMBER);
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type)
{
    if (scanner.current_pos - scanner.start_current_lexeme == start + length
	&& memcmp(scanner.start_current_lexeme + start, rest, length) == 0)
    {
	return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType()
{
    char firstChar = *scanner.start_current_lexeme;
    switch (firstChar)
    {
	case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
	case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
	case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
	case 'f':
	    if (scanner.current_pos - scanner.start_current_lexeme > 1)
	    {
		switch (scanner.start_current_lexeme[1]) 
		{
		    case 'a': return checkKeyword(2, 3, "lse", TOKEN_ELSE);
		    case 'o': return checkKeyword(2, 1, "r", TOKEN_OR);
		    case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
		}
	    }
	    break;
	case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
	case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
	case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
	case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
	case 'r': return checkKeyword(1, 5, "return", TOKEN_RETURN);
	case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
	case 't':
	    if (scanner.current_pos - scanner.start_current_lexeme > 1)
	    {
		switch (scanner.start_current_lexeme[1]) 
		{
		    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
		    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
		}
	    }
	    break;
	case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
	case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }
    return TOKEN_IDENTIFIER;
}

static Token identifier()
{
    while (isAlpha(*scanner.current_pos)) advance();
    return makeToken(identifierType());
}

void initScanner(const char* source)
{
    scanner.start_current_lexeme = source;
    scanner.current_pos = source;
    scanner.current_src_code_line = 1;
}

Token scanToken()
{
    skipWhiteSpaceAndComments();
    scanner.start_current_lexeme = scanner.current_pos;

    if (isAtEnd()) return makeToken(TOKEN_EOF);

    char c = advance();
    if (isAlpha(c)) return identifier();
    if (isDigit(c)) return number();

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
	// strings
	case '"': return string();
    }

    return errorToken("Unexpected character.");
}
