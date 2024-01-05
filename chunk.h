#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
	OP_ADD,
	OP_CONSTANT,
	OP_DIVIDE,
	OP_MULTIPLY,
	OP_NEGATE,
	OP_RETURN,
	OP_SUBTRACT,
} OpCode;

typedef struct {
	int count;				// nr of opcodes currently stored in array
	int capacity;			// capacity of array
	uint8_t* code;			// an array of OpCodes and operands (operands are indexes of Chunk.constants)
	int* lines;				// source code line nrs (indexes follow code array)
	// TODO: challenge: make lines more memory efficient (c.f. challenge ch 14)
	ValueArray constants;	// constants used by opcodes in chunk
} Chunk;

void initChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int srcCodeLineNr);
void freeChunk(Chunk* chunk);
int addConstant(Chunk* chunk, Value value);

#endif
