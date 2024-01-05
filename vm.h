#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define VALUE_STACK_MAX 256

typedef struct {
	Chunk* chunk;	// chunk to be executed
	uint8_t* ip;	// instruction pointer to the NEXT instruction
	Value valueStack[VALUE_STACK_MAX];
	Value* valueStackTop;
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

void initVM();
void freeVM();
InterpretResult interpret(Chunk* chunk);
void pushValue(Value value);
Value popValue();

#endif
