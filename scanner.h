#ifndef clox_scanner_h
#define clox_scanner_h

typedef enum
{
  // Single-character tokens.
  TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN, // 0 - 1
  TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE, // 2 - 3
  TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS, // 4 - 7
  TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR, // 8 - 10
  // One or two character tokens.
  TOKEN_BANG, TOKEN_BANG_EQUAL, // 11 - 12
  TOKEN_EQUAL, TOKEN_EQUAL_EQUAL, // 13 - 14
  TOKEN_GREATER, TOKEN_GREATER_EQUAL, // 15 - 16
  TOKEN_LESS, TOKEN_LESS_EQUAL, // 17 - 18
  // Literals.
  TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER, // 19 - 21
  // Keywords.
  TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE, // 22 - 25
  TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR, // 26 - 30
  TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS, // 31 - 34
  TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE, // 35 - 37

  TOKEN_ERROR, TOKEN_EOF // 38 - 39
} TokenType;

typedef struct
{
	TokenType type;
	const char* lexeme_start; // points to char in source code
	int lexeme_length;	  // length of the lexeme in bytes/chars
	int src_code_line;	  // src code line nr of where token appears
} Token;

void initScanner(const char* source);
Token scanToken();

#endif
