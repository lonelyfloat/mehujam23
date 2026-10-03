#include <stdio.h>
#include <SDL3/SDL.h>
#include "framework/dev_imgui.h"
#include "framework/audio.h"
#include "framework/input.h"
#include "framework/draw.h"
#include "framework/collision.h"
#include "framework/utils.h"
#include "game.h"
#include "framework/resource.h"

const int screenWidth = 64;
const int screenHeight = 64;


float delta = 1/60.0;

void Init(GameMemory* memory) {
    // Refresh ImGui context
    ImGuiContext* ctx = ImGui_GetCurrentContext();
    ImGui_SetCurrentContext(ctx);

    // Initialize game memory
    Game* game = (Game*)memory->gameMemory;
    ArenaReset(game->arena);

    // Initialize game systems
    game->input = InitInputSystem(game->arena, 30, 5, INPUT_TYPE_GAMEPAD | INPUT_TYPE_KEYBOARD | INPUT_TYPE_MOUSE);


    game->musicBus = AddAudioBus(game->arena, game->audio, 1);
    game->sfxBus = AddAudioBus(game->arena, game->audio, 5);

}

void Uninit(GameMemory* memory) {
    Game* game = (Game*)memory->gameMemory;
    ResetAudioSystem(game->audio);
}

void FixedUpdate(void* data, float dt) {
    Game* game = data;
    game->quit = PollInputSystem(game->input);
}


bool UpdateDrawFrame(GameMemory* memory) {
    Game* game = (Game*)memory->gameMemory;
    RunTimedFunction(FixedUpdate, delta, game, &game->accumulator, &game->time);
    UpdateAudioSystem(game->audio);
    
    BeginImGui();
    EndImGui();


    // Draw 

    ClearBackground(game->renderer, (Color){0x25, 0x25, 0x25, 0xFF});
    DrawImGui(game->renderer);

    return game->quit;
}
