#ifndef H_GAME
#define H_GAME

#include <SDL3/SDL.h>
#include "framework/game_api.h"
#include "framework/input.h"
#include "framework/audio.h"
#include "framework/arena.h"
#include "framework/proc_anim.h"

typedef struct Game {
    // Fields used in every game
    SDL_Window* window;
    SDL_Renderer* renderer;
    Arena* arena;
    InputSystem* input;
    AudioSystem* audio;

    int sfxBus, musicBus;

    float accumulator, time;
    bool quit;
} Game;

void Init(GameMemory* memory);
bool UpdateDrawFrame(GameMemory* memory); // Returns true if the program should quit
void Uninit(GameMemory* memory);

#endif
