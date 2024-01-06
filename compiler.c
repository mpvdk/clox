#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

void compile(const char* source)
{
    initScanner(source);
    int line = -1;
    while(1)
    {
        Token token = scanToken();
        if (token.src_code_line != line)
        {
            printf("%04d ", token.src_code_line);
            line = token.src_code_line;
        }
        else
        {
            printf("   | ");
        }
        printf("%2d '%.*s'\n", token.type, token.lexeme_length, token.lexeme_start);

        if (token.type == TOKEN_EOF) break;
    }

}
