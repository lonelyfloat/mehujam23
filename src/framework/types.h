#ifndef H_TYPES
#define H_TYPES

#include <stdint.h>
#include <stdbool.h>
// Simple 2D types, probably used throughout the entire codebase :)

typedef struct Vector2 {
    float x,y;
} Vector2;

typedef struct Rect {
    float x,y,width,height;
} Rect;

typedef struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} Color;

#endif
