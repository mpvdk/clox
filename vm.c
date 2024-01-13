#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "chunk.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "memory.h"
#include "table.h"
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
    resetValueStack();
}

void initVM()
{
    resetValueStack();
    vm.objects = NULL;
    initTable(&vm.globals);
    initTable(&vm.strings);
}

void freeVM()
{
    freeTable(&vm.globals);
    freeTable(&vm.strings);
    freeObjects();
}

void pushValue(Value value)
{
    *vm.valueStackTop = value;
    vm.valueStackTop++;
}

Value popValue()
{
    vm.valueStackTop--;
    return *vm.valueStackTop;
}

static Value peek(int distance)
{
    return vm.valueStackTop[-1 - distance];
}

static void concatenate()
{
    ObjString* b = AS_STRING(popValue());
    ObjString* a = AS_STRING(popValue());

    int length = a->length + b->length;
    char* chars = ALLOCATE(char, length + 1);
    memcpy(chars, a->chars, a->length);
    memcpy(chars + a->length, b->chars, b->length);
    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    pushValue(OBJ_VAL(result));
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
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
        //uint8_t instruction;
        switch (READ_BYTE())
        {
            case OP_ADD:
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) 
                {
                    concatenate();
                }
                else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
                {
                    double b = AS_NUMBER(popValue());
                    double a = AS_NUMBER(popValue());
                    pushValue(NUMBER_VAL(a + b));
                }
                else
                {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            case OP_CONSTANT:
                pushValue(READ_CONSTANT());
                break;
            case OP_DEFINE_GLOBAL:
                tableSet(&vm.globals, READ_STRING(), peek(0));
                popValue();
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
            case OP_GET_GLOBAL:
            {
                ObjString* name = READ_STRING();
                Value value;
                if (!tableGet(&vm.globals, name, &value))
                {
                    runtimeError("Undefined variable name '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                pushValue(value);
                break;
            }
            case OP_GET_LOCAL:
                pushValue(vm.valueStack[READ_BYTE()]);
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
            case OP_POP:
                popValue();
                break;
            case OP_PRINT:
                printValue(popValue());
                printf("\n");
                break;
            case OP_RETURN:
                return INTERPRET_OK;
            case OP_SET_GLOBAL:
            {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0)))
                {
                    // If tableSet() returns true, the given key did not exist yet.
                    // As lox doesn't allow implicit var decl, this is a runtime error.
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_SET_LOCAL:
                vm.valueStack[READ_BYTE()] = peek(0);
                break;
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
#undef READ_STRING
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
