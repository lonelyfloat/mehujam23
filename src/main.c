#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "framework/audio.h"
#include "framework/dev_imgui.h"
#include "framework/game_api.h"
#include "framework/arena.h"
#include "game.h"
#include <stdio.h>

#ifdef __EMSCRIPTEN__
    #include <emscripten/emscripten.h>
#endif

void MainLoop();

GameMemory gameMemory = {};
GameCalls calls = {};
bool done = false;

int main(int argc, char* argv[]) {
    // Initialize SDL context

    Arena* programArena = ArenaCreate(100000); // Allocate 10 000 bytes for the game as a whole

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD);
    MIX_Init();

    SDL_Window* window = SDL_CreateWindow("SDL Template", 1280, 720, SDL_WINDOW_OPENGL);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);

    // Initialize program context
    Game* game = ArenaAllocStruct(programArena, Game);
    game->arena = ArenaGrabSection(programArena, 90000);
    game->window = window;
    game->renderer = renderer;

    // Initialize audio system

    MIX_Mixer* mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    game->audio = InitAudioSystem(programArena, mixer, 8, 30, 50);


    gameMemory.gameMemory = (char*)game;
    gameMemory.gameMemorySize = sizeof(Game);

    InitGameCalls(&calls, Init, UpdateDrawFrame, Uninit);

    InitImGui(window, renderer);

    calls.init(&gameMemory);

    // Main loop
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    while(!done) {
        MainLoop();
    }
#endif

    // Destroy program memory
    calls.uninit(&gameMemory);

    ArenaDestroy(programArena);
    UnloadGameCalls(&calls);
    DestroyImGui();
    // Destroy SDL Context  
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    MIX_Quit();
    SDL_Quit();
    return 0;
    
}
void MainLoop() {
    done = calls.update(&gameMemory);
#ifdef HOT_RELOAD_LIB
    if(GameCallsUpdated(&calls)) {
        calls.uninit(&gameMemory);
        LoadGameCallsDynamic(&calls);
        calls.init(&gameMemory);
    }
#endif
}
