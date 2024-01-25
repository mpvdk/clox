#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum
{
	OP_ADD,			// 0
	OP_CALL,
	OP_CLASS,
	OP_CLOSE_UPVALUE,
	OP_CLOSURE,
	OP_CONSTANT,
	OP_DEFINE_GLOBAL,
	OP_DIVIDE,
	OP_EQUAL,
	OP_FALSE,
	OP_GET_GLOBAL,
	OP_GET_LOCAL,
	OP_GET_PROPERTY,
	OP_GET_SUPER,
	OP_GET_UPVALUE,
	OP_GREATER,
	OP_INHERIT,
	OP_INVOKE,
	OP_JUMP,
	OP_JUMP_IF_FALSE,
	OP_LESS,
	OP_LOOP,
	OP_METHOD,
	OP_MULTIPLY,
	OP_NEGATE,
	OP_NOT,
	OP_NIL,
	OP_POP,
	OP_PRINT,
	OP_RETURN,
	OP_SET_GLOBAL,
	OP_SET_LOCAL,
	OP_SET_PROPERTY,
	OP_SET_UPVALUE,
	OP_SUBTRACT,
	OP_SUPER_INVOKE,
	OP_TRUE,
} OpCode;

typedef struct 
{
	int count;				// nr of opcodes currently stored in array
	int capacity;			// capacity of array
	uint8_t* code;			// an array of OpCodes and operands (operands are indexes into Chunk.constants)
	int* lines;				// source code line nrs (indexes follow code array)
	// TODO: challenge: make lines more memory efficient (c.f. challenge ch 14)
	ValueArray constants;	// constants used by opcodes in chunk
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int srcCodeLineNr);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

#endif
