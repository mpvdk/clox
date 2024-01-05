#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "debug.h"
#include "value.h"
#include "vm.h"

VM vm; // TODO: create function to get a VM instead of providing one global instance

static void resetValueStack()
{
    vm.valueStackTop = vm.valueStack;
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
    return *vm.valueStackTop;
}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
    do { \
        double b = popValue(); \
        double a = popValue(); \
        pushValue(a op b); \
    } while (false)

    for (;;)
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
                {
                    BINARY_OP(+);
                    break;
                }
            case OP_CONSTANT:
                {
                    Value constant = READ_CONSTANT();
                    pushValue(constant);
                    break;
                }
            case OP_DIVIDE:
                {
                    BINARY_OP(/);
                    break;
                }
            case OP_MULTIPLY:
                {
                    BINARY_OP(*);
                    break;
                }
            case OP_NEGATE:
                {
                    *(vm.valueStackTop - 1) = -*(vm.valueStackTop -1);
                    break;
                }
            case OP_RETURN:
                {
                    printValue(popValue());
                    printf("\n");
                    return INTERPRET_OK;
                }
            case OP_SUBTRACT:
                {
                    BINARY_OP(-);
                    break;
                }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk)
{
    vm.chunk = chunk;
    vm.ip = vm.chunk->code;
    return run();
}
