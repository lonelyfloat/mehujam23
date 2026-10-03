#include <stdio.h>
#include <string.h>
#include "resource.h"

typedef struct ResourceSlot {
    void* value;
    // For validating handles
    uint16_t magic;
    uint16_t id;
    char* key;

    // Object pool stuff
    struct ResourceSlot* next; // Free list pointer if refcount == 0
    int refcount;
} ResourceSlot;


// FNV1a hash algorithm
const static size_t _g_resource_BASE = 0x811c9dc5;
const static size_t _g_resource_PRIME = 0x01000193;
static size_t HashString(const char *str) {
    size_t initial = _g_resource_BASE;
    while(*str) {
        initial ^= *str++;
        initial *= _g_resource_PRIME;
    }
    return initial;
}

ResourceCache* AllocResourceCache(Arena* arena, size_t maxMemory, size_t capacity, size_t elementSize, ResourceLoadCallback load, ResourceUnloadCallback unload) {
    Arena* cacheArena = ArenaGrabSection(arena, maxMemory);
    ResourceCache* result = ArenaAllocStruct(cacheArena, ResourceCache);
    result->arena = cacheArena;
    result->capacity = capacity;
    result->count = 0;
    result->handles = ArenaAllocArray(cacheArena, ResourceHandle, capacity);
    result->slots = ArenaAllocArray(cacheArena, ResourceSlot, capacity);
    result->load = load;
    result->unload = unload;

    result->firstSlot = &result->slots[0];
    for(int i = 0; i < capacity; ++i) {
        result->slots[i].key = NULL;
        if(elementSize > 0) {
            result->slots[i].value = ArenaAlloc(cacheArena, elementSize);
        } else {
            result->slots[i].value = NULL;
        }
        result->slots[i].magic = 0;
        result->slots[i].id = i;
        result->slots[i].refcount = 0;

        result->handles[i].id = 0;
        result->handles[i].magic = 0;
        result->handles[i].source = result;

        // Set up free list
        if(i < capacity-1) {
            result->slots[i].next = &result->slots[i+1];
        }
    }
    result->slots[capacity-1].next = NULL;

    return result;
}

ResourceSlot* GetResourceSlot(ResourceCache* cache, ResourceHandle handle) {
    if(cache->slots[handle.id].magic == handle.magic) {
        return &cache->slots[handle.id];
    }
    return NULL;
}

ResourceSlot* CreateSlot(ResourceCache* cache, char* key, void* userData) {
    // Getting a *new* slot:
    if(cache->firstSlot == NULL) {
        return NULL;
    }
    
    ResourceSlot* new = cache->firstSlot;
    cache->firstSlot = new->next;
    // Load slot  here...

    cache->load(&new->value, key, userData);
    new->magic++;
    new->key = ArenaAllocString(cache->arena, key);

    //
    return new;
}

ResourceHandle CreateHandle(ResourceCache* cache, char* key, void* userData) {
    ResourceSlot* slot = CreateSlot(cache, key, userData);
    if(slot == NULL) {
        fprintf(stderr, "ERROR: ResourceCache pool is full");
        exit(1);
    }
    ResourceHandle handle = (ResourceHandle){.id=slot->id, .magic = slot->magic, cache};
    return handle;
}

ResourceHandle* FindResourceHandle(ResourceCache* cache, char* key) {
    size_t hash = HashString(key);
    size_t idx = hash % cache->capacity;
    while(true) { 
        bool empty = cache->handles[idx].magic == 0 || cache->slots[cache->handles[idx].id].refcount == 0;
        if(
        (cache->handles[idx].magic == cache->slots[cache->handles[idx].id].magic && (cache->slots[cache->handles[idx].id].key != NULL && strcmp(key, cache->slots[cache->handles[idx].id].key) == 0))
        || empty
        ) {
            return &cache->handles[idx];
        }
        idx = (idx + 1) % cache->capacity;
    }
}

ResourceHandle GetResourceHandle(ResourceCache* cache, char* key, void* userData) {
    ResourceHandle* handle = FindResourceHandle(cache, key);
    if(handle->magic == 0 || cache->slots[handle->id].refcount == 0) {
        *handle = CreateHandle(cache, key, userData);
    }
    ResourceSlot* slot = GetResourceSlot(cache, *handle);
    slot->refcount++;
    return *handle;
}

void* GetResource(ResourceHandle handle) {
    if(handle.magic == handle.source->slots[handle.id].magic) {
        return handle.source->slots[handle.id].value;
    }
    return NULL;
}

void ReleaseHandle(ResourceHandle handle) {
    handle.source->slots[handle.id].refcount--;
    if(handle.source->slots[handle.id].refcount == 0) {
        // Add slot to free listj
        handle.source->slots[handle.id].next = handle.source->firstSlot;
        handle.source->firstSlot = &handle.source->slots[handle.id];
        // Unload resource... 
        handle.source->unload(handle.source->firstSlot->value);

    }
}

void ClearCache(ResourceCache* cache) {
    for(int i = 0; i < cache->capacity; ++i) {
        if(cache->handles[i].magic > 0 && cache->slots[cache->handles[i].id].magic == cache->handles[i].magic) {
            cache->slots[cache->handles[i].id].refcount = 1;
            ReleaseHandle(cache->handles[i]);
        }
    }
}
