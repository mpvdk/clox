# clox - a bytecode interpreter for lox

This is a bytecode interpreter for the lox language built in c. It was created by guidance of the book _Crafting Interpreters_ by Robert Nystrom.

To learn more about the book, check out [craftinginterpreters.com](https://www.craftinginterpreters.com)

To learn more about the lox language, check out [this particular chapter](https://craftinginterpreters.com/the-lox-language.html)

## Building

First clone this repository and `cd` into it:

```shell
git clone git@github.com:mpvdk/clox.git && cd plox
```

Then just run make for the release build:

```shell
make
```

To add debug symbols and disable optimization:

```shell
make debug
```

For debugging you can also uncomment the debug defines in `common.h`

## Running

To run the interpreter on a lox source file:

``` shell
./bld/clox <file>
```

To run the interpreter in interactive mode:

``` shell
./bld/clox
```
