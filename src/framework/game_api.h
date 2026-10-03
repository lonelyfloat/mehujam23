#ifndef GAME_API
#define GAME_API

#include "arena.h"
#include <SDL3/SDL.h>


typedef struct GameMemory {
    char* gameMemory; // A pointer to a stored user-defined state struct
    size_t gameMemorySize;
} GameMemory;

typedef void (*GameInitCallback)(GameMemory* memory); // Initializes game state with Arena* 
typedef bool (*GameUpdateDrawCallback)(GameMemory* memory); // Returns true if the program should quit
typedef void (*GameUninitCallback)(GameMemory* memory);

typedef struct GameCalls {
    GameInitCallback init;
    char* initName;
    GameUpdateDrawCallback update;
    char* updateName;
    GameUninitCallback uninit;
    char* uninitName;

    // All of this is just for hot-reloading: 
    SDL_SharedObject* gameCallsLib; // Will be null if game is being statically linked
    char* libPath;
    char versionedLibPath[100];
    SDL_Time modifyTime;
    bool loadedGameCalls;
    int apiVersion;
} GameCalls;

void InitGameCallsStatic(GameCalls* calls, GameInitCallback init, GameUpdateDrawCallback update, GameUninitCallback uninit);
void InitGameCallsDynamic(GameCalls* calls, char* libPath, char* initName, char* updateName, char* uninitName);

bool GameCallsUpdated(GameCalls* calls);
void LoadGameCallsDynamic(GameCalls* calls);

void UnloadGameCalls(GameCalls* calls);

#ifdef HOT_RELOAD_LIB
#define _STRIFY(X) #X
#define _STR(X) _STRIFY(X)
#define InitGameCalls(calls, inc, upc, unc) InitGameCallsDynamic((calls), _STR(HOT_RELOAD_LIB), _STR(inc), _STR(upc), _STR(unc));
#else 
#define InitGameCalls(calls, inc, upc, unc) InitGameCallsStatic((calls), (inc), (upc), (unc));
#endif


#endif
