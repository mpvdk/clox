#include <stdlib.h>

#include "chunk.h"
#include "memory.h"
#include "value.h"
#include "vm.h"

void initChunk(Chunk* chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int srcCodeLineNr)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int oldCapacity = chunk->capacity;
        chunk->capacity = NEW_ARRAY_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    }

    // arrays are obv zero-indexed so count points to first free slot
    chunk->code[chunk->count] = byte; 
    chunk->lines[chunk->count] = srcCodeLineNr;
    chunk->count++;
}

void freeChunk(Chunk* chunk)
{
    FREE_ARRAY(size_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value)
{
    pushValue(value); 
    writeValueArray(&chunk->constants, value);
    popValue();
    // return array index of the stored value
    return chunk->constants.count - 1;
}
