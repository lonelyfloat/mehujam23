#include <stdio.h>
#include "game_api.h"

void InitGameCallsStatic(GameCalls* calls, GameInitCallback init, GameUpdateDrawCallback update, GameUninitCallback uninit) {
    calls->init = init;
    calls->update = update;
    calls->uninit = uninit;
}
void InitGameCallsDynamic(GameCalls* calls, char* libPath, char* initName, char* updateName, char* uninitName) {
    calls->initName = initName;
    calls->updateName = updateName;
    calls->uninitName = uninitName;
    calls->libPath = libPath;
    LoadGameCallsDynamic(calls);
}

bool GameCallsUpdated(GameCalls* calls) {
    SDL_PathInfo info;
    SDL_GetPathInfo(calls->libPath, &info);
    if(info.modify_time != calls->modifyTime) {
        if(labs(info.modify_time - calls->modifyTime) > 100000) {
            return true; 
        }
    }
    return false;
}

void LoadGameCallsDynamic(GameCalls* calls) {
    SDL_PathInfo info;
    if(!SDL_GetPathInfo(calls->libPath, &info)) {
        // This will fail sometimes during recompiling, so no log message
        return;
    }

    snprintf(calls->versionedLibPath, 100,"%s.%d", calls->libPath, calls->apiVersion);

    if(!SDL_CopyFile(calls->libPath, calls->versionedLibPath)) {
        fprintf(stderr, "Error copying file: %s\n", SDL_GetError());
        return;
    }

    SDL_SharedObject* newLib = SDL_LoadObject(calls->versionedLibPath);
    if(!newLib) { 
        // This will fail sometimes during recompiling, so no log message
        return;
    }
    GameInitCallback newInit = (GameInitCallback)SDL_LoadFunction(newLib, calls->initName);
    if(!newInit) {
        fprintf(stderr, "Error loading symlink: %s\n", SDL_GetError());
        SDL_UnloadObject(newLib);
        return;
    }
    GameUpdateDrawCallback newUpdate = (GameUpdateDrawCallback)SDL_LoadFunction(newLib, calls->updateName);
    if(!newUpdate) {
        fprintf(stderr, "Error loading symlink: %s\n", SDL_GetError());
        SDL_UnloadObject(newLib);
        return;
    }

    GameUninitCallback newUninit = (GameUninitCallback)SDL_LoadFunction(newLib, calls->uninitName);
    if(!newUninit) {
        fprintf(stderr, "Error loading symlink: %s\n", SDL_GetError());
        SDL_UnloadObject(newLib);
        return;
    }
    if(calls->loadedGameCalls) {
        UnloadGameCalls(calls);
    }

    calls->gameCallsLib = newLib;
    calls->init = newInit;
    calls->update = newUpdate;
    calls->uninit = newUninit;
    calls->modifyTime = info.modify_time;
    calls->loadedGameCalls = true;
    calls->apiVersion++;
}

void UnloadGameCalls(GameCalls* calls) {
    SDL_UnloadObject(calls->gameCallsLib);
    char buf[100];
    snprintf(buf, 100,"%s.%d", calls->libPath, calls->apiVersion-1);
    SDL_RemovePath(buf);
}
