#ifndef H_RESOURCE
#define H_RESOURCE

#include <stdbool.h>
#include <stdint.h>
#include "arena.h"

typedef void (*ResourceLoadCallback)(void** output, char* key, void* userData);
typedef void (*ResourceUnloadCallback)(void* data);

typedef struct ResourceHandle {
    uint16_t id;
    uint16_t magic;
    struct ResourceCache* source;
} ResourceHandle;

typedef struct ResourceSlot ResourceSlot;

typedef struct ResourceCache {
    Arena* arena;

    ResourceSlot *slots; // Pool
    ResourceSlot* firstSlot;

    ResourceHandle *handles; // This *should* actually be a hash map 

    size_t capacity;
    size_t count;

    ResourceLoadCallback load;
    ResourceUnloadCallback unload;
} ResourceCache;

ResourceCache* AllocResourceCache(Arena* arena, size_t maxMemory, size_t capacity, size_t elementSize, ResourceLoadCallback load, ResourceUnloadCallback unload);
ResourceHandle GetResourceHandle(ResourceCache* cache, char* key, void* userData);
void* GetResource(ResourceHandle handle);
void ReleaseHandle(ResourceHandle handle);
void ClearCache(ResourceCache* cache);


/*

End-user API interface:
ResourceHandle tex = GetResourceHandle(textureCache, "assets/texture.png"); // carry this around everywhere
SDL_Texture* texPtr = GetResource(tex); // when you need to actually use the resource

ReleaseHandle(tex); // makes the 'tex' handle invalid
                    //

handles = {1, 2, 0, 3}
slots[0] = tex; 
slots[1] = tex2; 
slots[2] = sound; 
slots[3] = sound2; 

GetHandle(tex);
ReleaseHandle(tex); 
// slots[0] is killed and added to the free-list

 */

#endif
