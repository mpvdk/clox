#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "value.h"
#include "vm.h"

VM vm; // TODO: create function to get a VM instead of providing one global instance

static void resetValueStack()
{
    vm.valueStackTop = vm.valueStack;
}

static void runtimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code -1;
    int line = vm.chunk->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetValueStack;
}

void initVM()
{
    resetValueStack();
}

void freeVM()
{
}

void pushValue(Value value)
{
    *vm.valueStackTop = value;
    vm.valueStackTop++;
}

Value popValue()
{
    vm.valueStackTop--;
    return* vm.valueStackTop;
}

static Value peek(int distance)
{
    return vm.valueStackTop[-1 - distance];
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op) \
    do { \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            runtimeError("Operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(popValue()); \
        double a = AS_NUMBER(popValue()); \
        pushValue(valueType(a op b)); \
    } while (false)

    while(1)
    {
#ifdef DEBUG_TRACE_EXECUTION
    printf("           ");
    for (Value* slot = vm.valueStack; slot < vm.valueStackTop; slot++)
    {
        printf("[ ");
        printValue(*slot);
        printf(" ]");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
#endif
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_ADD:
                BINARY_OP(NUMBER_VAL, +);
                break;
            case OP_CONSTANT:
                pushValue(READ_CONSTANT());
                break;
            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;
            case OP_EQUAL:
                pushValue(BOOL_VAL(valuesEqual(popValue(), popValue())));
                break;
            case OP_FALSE:
                pushValue(BOOL_VAL(false));
                break;
            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS:
                BINARY_OP(BOOL_VAL, <); break;
            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;
            case OP_NEGATE:
                if (!IS_NUMBER(peek(0)))
                {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                pushValue(NUMBER_VAL(-AS_NUMBER(popValue())));
                break;
            case OP_NIL:
                pushValue(NIL_VAL);
                break;
            case OP_NOT:
                pushValue(negateBool(toBool(popValue())));
                break;
            case OP_RETURN:
                printValue(popValue());
                printf("\n");
                return INTERPRET_OK;
            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;
            case OP_TRUE:
                pushValue(BOOL_VAL(true));
                break;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(const char* source)
{
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk))
    {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    InterpretResult result = run();

    freeChunk(&chunk);
    return result;
}
