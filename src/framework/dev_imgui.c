#include "dev_imgui.h"

void InitImGui(SDL_Window* window, SDL_Renderer* renderer) {
    ImGui_CreateContext(NULL);
    ImGuiIO* io = ImGui_GetIO(); 
    (void)io;
    // io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io->IniFilename = "build/imgui.ini";
    // tell ImGui the display scale
    ImGui_StyleColorsDark(NULL);
    ImGuiStyle* style = ImGui_GetStyle();
    style->FontScaleDpi = 1.5;
    cImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    cImGui_ImplSDLRenderer3_Init(renderer);
}

void BeginImGui() {
    cImGui_ImplSDLRenderer3_NewFrame();
    cImGui_ImplSDL3_NewFrame();
    ImGui_NewFrame();
}

void EndImGui() {
    ImGui_Render();
}

void DrawImGui(SDL_Renderer* renderer) {
    cImGui_ImplSDLRenderer3_RenderDrawData(ImGui_GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

void DestroyImGui() {
    ImGuiContext* ctx = ImGui_GetCurrentContext();
    cImGui_ImplSDLRenderer3_Shutdown();
    cImGui_ImplSDL3_Shutdown();
    ImGui_DestroyContext(ctx);
}
