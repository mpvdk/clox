#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define VALUE_STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct
{
	ObjClosure* closure;
	uint8_t* ip;
	Value* slots;
} CallFrame;

typedef struct
{
	CallFrame frames[FRAMES_MAX];		// Stackframes
	int frameCount;						// Current height of frames
	Value valueStack[VALUE_STACK_MAX];
	Value* valueStackTop;				// First empty slot of value stack
	Table globals;						// global variables
	Table strings;						// Interned strings
	ObjUpvalue* openUpvalues;			// Closed over variables still on stack
	struct Obj* objects;				// Linked list of objects allocated on heap
	// "gray" means processed, but not all children processed
	int grayCount;						// Count of gray objects
	int grayCapacity;					// Max nr of gray objects
	Obj** grayStack;					// List of references to gray objects
} VM;

typedef enum
{
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
