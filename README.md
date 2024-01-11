# clox

main.c      main()
main.c      runFile()
vm.c        interpret()
    [ chunk.c    initChunk() ]
compiler.c  compile()
    [ scanner.c  initScanner() ]
vm.c        run()


Chunk                           vm.c
    int         count
    int         capacity
    uint8_t*    code
    int         lines
    ValueArray  constants
Token                           scanner.c (makeToken()) 
    TokenType   type
    char*       lexeme_start
    int         lexeme_lenght
    int         src_code_line
Value                           
    ValueType   type
    union {
        bool    boolean
        double  number
    } as
ValueArray                      chunk.c -> initChunk() -> initValueArray()
    int         capacity
    int         count
    Value*      values
Scanner                         scanner.c
    char*       start_current_lexeme
    char*       current_pos
    int         current_src_code_line
Parser                          parser.c
    Token       current
    Token       previous
    bool        hadError
    bool        panicMode
ParseRule                       parser.c
    ParseFn     prefix
    ParseFn     infix
    Precedence  precedence
VM                              vm.c
    Chunk*      chunk
    uint8_t     ip
    Value       valueStack[]
    Value*      valueStackTop


