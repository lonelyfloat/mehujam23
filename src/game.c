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

const int screenWidth = 1280;
const int screenHeight = 720;

const int gridSize = 32;


Color deadColor = (Color){0xFF,0,0,0xFF};
Color aliveColor = (Color){0x00,0,0xFF,0xFF};
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

    // Initialize world grid
    game->world.width =  screenWidth/gridSize;
    game->world.height = screenHeight/gridSize;
    // printf("%d,%d\n", game->world.width, game->world.height);
    game->world.grid = ArenaAllocArray(game->arena, float*, game->world.width);
        srand(SDL_GetTicksNS());
    for(int x = 0; x < game->world.width; ++x) {
        game->world.grid[x] = ArenaAllocArray(game->arena, float, game->world.height);
        for(int y = 0; y < game->world.height; ++y) {
            game->world.grid[x][y] = -1;
            if((rand() % 100) > 70) {
                int t = rand() % 100;
                float f = t / 100.0;
                game->world.grid[x][y] = f;
            }
        }
    }

}

void Uninit(GameMemory* memory) {
    Game* game = (Game*)memory->gameMemory;
    ResetAudioSystem(game->audio);
}

void FixedUpdate(void* data, float dt) {
    Game* game = data;
    game->quit = PollInputSystem(game->input);
    for(int y = game->world.height-2; y >= 0; --y) {
        for(int x = 0; x < game->world.width; ++x) {
            if(game->world.grid[x][y] >= 0 && game->world.grid[x][y+1] < 0) {
                float temp = game->world.grid[x][y+1];
                game->world.grid[x][y+1] = game->world.grid[x][y];
                game->world.grid[x][y] = temp;
            }
        }
    }

}


bool UpdateDrawFrame(GameMemory* memory) {
    Game* game = (Game*)memory->gameMemory;
    RunTimedFunction(FixedUpdate, delta, game, &game->accumulator, &game->time);
    UpdateAudioSystem(game->audio);
    
    BeginImGui();
    EndImGui();


    // Draw 

    ClearBackground(game->renderer, (Color){0x25, 0x25, 0x25, 0xFF});
    for(int x = 0; x < game->world.width; ++x) {
        for(int y = 0; y < game->world.height; ++y) {
            if(game->world.grid[x][y] < 0) {
                continue;
            }
            Rect r = (Rect){x*gridSize, y*gridSize, gridSize, gridSize};
            float dead = 1 - game->world.grid[x][y];
            float alive = game->world.grid[x][y];
            Color c = (Color){deadColor.r*dead + aliveColor.r*alive,
                deadColor.g*dead + aliveColor.g*alive,
                deadColor.b*dead + aliveColor.b*alive,
                deadColor.a*dead + aliveColor.a*alive
            };
            DrawRect(game->renderer, r, c);
        }
    }

    DrawImGui(game->renderer);

    return game->quit;
}
