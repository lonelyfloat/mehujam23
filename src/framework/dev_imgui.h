#ifndef H_DEV_IMGUI
#define H_DEV_IMGUI

#include <SDL3/SDL.h>
#include "dcimgui.h"
#include "dcimgui_impl_sdl3.h"
#include "dcimgui_impl_sdlrenderer3.h"

// Wrapper around ImGui 
void InitImGui(SDL_Window* window, SDL_Renderer* renderer);
void BeginImGui();
void EndImGui();
void DrawImGui(SDL_Renderer* renderer);
void DestroyImGui();

#endif
