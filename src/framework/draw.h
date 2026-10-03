#ifndef H_DRAW
#define H_DRAW

#include <SDL3/SDL.h>
#include "types.h"

// Wrappers around SDL drawing functions for my types so I don't constantly have to convert

void ClearBackground(SDL_Renderer* renderer, Color color);

void DrawCircle(SDL_Renderer* renderer, Vector2 center, int radius, Color color);
void DrawRect(SDL_Renderer* renderer, Rect rect, Color color);

void DrawTexture(SDL_Renderer* renderer, SDL_Texture* texture, Vector2 position);
void DrawTextureRect(SDL_Renderer* renderer, SDL_Texture* texture, Rect source, Rect dest);
void DrawTexturePro(SDL_Renderer* renderer, SDL_Texture* texture, Rect source, Rect dest, Vector2 origin, float rotation);

#endif
