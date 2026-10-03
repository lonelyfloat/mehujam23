#ifndef H_ARENA
#define H_ARENA

#include <stdlib.h>
#include <stdalign.h>

typedef struct Arena {
    char* data;
    size_t offset;
    size_t previousOffset;
    size_t capacity;
} Arena;

Arena* ArenaCreate(size_t max_bytes);
void ArenaDestroy(Arena* arena); 

void* ArenaAlloc(Arena* arena, size_t bytes);
void* ArenaAllocAlign(Arena* arena, size_t bytes, size_t align);

void ArenaReset(Arena* arena);

Arena* ArenaGrabSection(Arena* arena, size_t bytes);

char* ArenaAllocString(Arena* arena, const char* cstr);

#define ArenaAllocStruct(arena, type) (type*)(ArenaAllocAlign((arena), sizeof(type), (alignof(type))))
#define ArenaAllocArray(arena, type, count) (type*)(ArenaAllocAlign((arena), sizeof(type)*count, (alignof(type))))

#ifdef ARENA_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#ifndef DEFAULT_ALIGN
    #define DEFAULT_ALIGN (2*sizeof(void*))
#endif 

Arena* ArenaCreate(size_t max_bytes) {
    Arena* arena = malloc(sizeof(Arena));
    arena->offset = 0;
    arena->previousOffset = 0;
    arena->capacity = max_bytes;
    arena->data = calloc(max_bytes,1);
    return arena;
}

void ArenaDestroy(Arena* arena) {
    free(arena->data);
    free(arena);
}

void* ArenaAllocAlign(Arena* arena, size_t bytes, size_t align) {
    // Alignment stuff
    uintptr_t offset;
    {
        uintptr_t currPtr = (uintptr_t)arena->data + (uintptr_t)arena->offset;
        uintptr_t modulo = currPtr & ((uintptr_t)align-1);
        if(modulo != 0) {
            currPtr += align - modulo;
        }
        offset = currPtr - (uintptr_t)arena->data;
    }
    if(arena->offset+bytes <= arena->capacity) {
        // Allocation if there's room
        void* ptr = &arena->data[offset];
        arena->previousOffset = offset;
        arena->offset += bytes;
        memset(ptr, 0, bytes);
        return ptr;
    }
    printf("offset: %d\n", arena->offset);
    // If there is not room for the allocation, abort
    fprintf(stderr, "ERROR: arena offset exceeds capacity\n");
    exit(1);
}

void* ArenaAlloc(Arena* arena, size_t bytes) {
    return ArenaAllocAlign(arena, bytes, DEFAULT_ALIGN);
}

void ArenaReset(Arena* arena) {
    arena->offset = 0;
    arena->previousOffset = 0;
}

Arena* ArenaGrabSection(Arena* arena, size_t bytes) {
    Arena* result = ArenaAlloc(arena, sizeof(Arena));
    result->offset = 0; 
    arena->previousOffset = 0;
    result->capacity = bytes;
    result->data = ArenaAlloc(arena, bytes);
    return result;
}

char* ArenaAllocString(Arena* arena, const char* cstr) {
    int i = 0;
    while(cstr[i] != '\0') {
        i++;
    }
    i+= 1;
    char* ptr = ArenaAlloc(arena, i);
    i = 0;
    while(cstr[i] != '\0') {
        ptr[i] = cstr[i];
        i++;
    }
    ptr[i] = '\0';
    return ptr;
}
#endif


#endif
