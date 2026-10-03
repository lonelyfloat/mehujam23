#ifndef H_COLLISION
#define H_COLLISION

#include "utils.h"

// Utilities for Separating Axis Theorem collision

// Returns tuple (min,max)
Vector2 ProjectRectangleOnAxis(Rect r, Vector2 axis);
Vector2 ProjectPolygonOnAxis(Vector2* vertices, int vertexCount, Vector2 axis);
float GetOverlap(Vector2 a, Vector2 b);
float GetSignedOverlap(Vector2 moving, Vector2 still);

bool CheckCollisionPointRect(Vector2 pt, Rect r);
Vector2 ResolveRectCollision(Rect a, Rect b, float* overlap); // returns resolution axis and overlap

#endif
