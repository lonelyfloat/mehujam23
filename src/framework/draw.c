#include "draw.h"
#include <math.h>

#include <stdio.h>

void ClearBackground(SDL_Renderer* renderer, Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderer);
}

static inline SDL_Vertex VertexFromInfo(Vector2 pt, Color color) {
    return (SDL_Vertex){
        .position = {pt.x,pt.y},
        .color = {color.r/255.0,color.g/255.0,color.b/255.0,color.a/255.0}
    };
}

#define CIRCLE_ERROR_RATE 0.25

// Adapted from here: https://siegelord.net/circle_draw and raylib's circle drawing
void DrawCircle(SDL_Renderer* renderer, Vector2 center, int radius, Color color) {
    float theta = acosf(2*powf(1 - CIRCLE_ERROR_RATE/radius, 2) - 1);
    int segmentCount = (int)ceilf(2*M_PI/theta);

    const int vertexCount = segmentCount+1;
    SDL_Vertex vertices[vertexCount];

    const int indexCount = 3*segmentCount;
    int indices[indexCount];

    float c = cosf(theta);//precalculate the sine and cosine
	float s = sinf(theta);

    float t;
	float x = radius;//we start at angle = 0 
	float y = 0;
    for(int i = 0; i < segmentCount; ++i) {
        // Add vertex
        vertices[i] = VertexFromInfo((Vector2){x+center.x, y+center.y}, color);
        // Add indices
        indices[3*i] = i;
        indices[(3*i)+1] = (i+1) % segmentCount;
        indices[(3*i)+2] = vertexCount-1;
        //apply the rotation matrix
		t = x;
		x = c * x - s * y;
		y = s * t + c * y;

    }

    vertices[vertexCount-1] = VertexFromInfo(center, color);
    SDL_RenderGeometry(renderer, NULL, vertices, vertexCount, indices, indexCount);

}

void DrawRect(SDL_Renderer* renderer, Rect rect, Color color) {
    SDL_FRect frect = (SDL_FRect){rect.x, rect.y, rect.width, rect.height};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &frect);
}

void DrawTexture(SDL_Renderer* renderer, SDL_Texture* texture, Vector2 position) {
    DrawTextureRect(renderer, texture, (Rect){0,0,texture->w, texture->h}, (Rect){position.x,position.y, texture->w, texture->h});
}
void DrawTextureRect(SDL_Renderer* renderer, SDL_Texture* texture, Rect source, Rect dest) {
    DrawTexturePro(renderer, texture, source, dest, (Vector2){0,0}, 0);
}

void DrawTexturePro(SDL_Renderer* renderer, SDL_Texture* texture, Rect source, Rect dest, Vector2 origin, float rotation) {
    SDL_FRect src = (SDL_FRect){source.x,source.y, source.width, source.height};
    SDL_FRect destRect = (SDL_FRect){dest.x,dest.y, dest.width, dest.height};
    SDL_FPoint center = (SDL_FPoint){origin.x, origin.y};
    SDL_FlipMode flip = 0;
    flip |= source.x < 0;
    flip |= SDL_FLIP_VERTICAL * (source.y < 0);
    SDL_RenderTextureRotated(renderer, texture, &src, &destRect, rotation, &center, flip);
}
