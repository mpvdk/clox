#ifndef clox_memory_h
#define clox_memory_h

#include "common.h"
#include "object.h"

#define MIN_CHUNK_CAPACITY 8

#define NEW_ARRAY_CAPACITY(capacity) \
	((capacity) < MIN_CHUNK_CAPACITY ? MIN_CHUNK_CAPACITY : (capacity) * 2)

#define ALLOCATE(type, count) \
	(type*)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, pointer) reallocate(pointer, sizeof(type), 0)

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
	(type*)reallocate(pointer, sizeof(type) * (oldCount), sizeof(type) * (newCount))

#define FREE_ARRAY(type, pointer, oldCount) \
	reallocate(pointer, sizeof(type) * (oldCount), 0)

void* reallocate(void* pointer, size_t oldSize, size_t newSize);
void collectGarbate();
void markObject(Obj* object);
void markValue(Value value);
void freeObjects();

#endif
