#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define VALUE_STACK_MAX 256

typedef struct {
	Chunk* chunk;			// chunk to be executed
	uint8_t* ip;			// instruction pointer to the NEXT instruction
	Value valueStack[VALUE_STACK_MAX];
	Value* valueStackTop;	// First empty slot of value stack
	Table globals;			// global variables
	Table strings;			// Interned strings
	struct Obj* objects;	// linked list of objects allocated on heap
} VM;

typedef enum {
	INTERPRET_OK,
	INTERPRET_COMPILE_ERROR,
	INTERPRET_RUNTIME_ERROR
} InterpretResult;

extern VM vm;

void initVM();
void freeVM();
InterpretResult interpret(const char* source);
void pushValue(Value value);
Value popValue();

#endif
