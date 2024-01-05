#include <stdint.h>
#include <stdio.h>

#include "chunk.h"
#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;)
    {
        // We set offset here because instructions vary in length
        offset = disassembleInstruction(chunk, offset);
    }
}

static int constantInstruction(const char* name, Chunk* chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

int disassembleInstruction(Chunk* chunk, int offset)
{
    // print opcode offset
    printf("%04d ", offset);

    // source code line nrs
    if (offset > 0 && chunk->lines[offset] == chunk->lines[offset -1])
    {
        // source code line nr is same as previous opcode; print '|'
        printf("   |  ");
    }
    else {
        // print source code line nr
        printf("%4d  ", chunk->lines[offset]);
    }

    // print actual instructions
    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);
        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);
        default:
            printf("unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

int simpleInstruction(const char* name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}
