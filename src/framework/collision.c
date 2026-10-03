#include "collision.h"

Vector2 ProjectRectangleOnAxis(Rect r, Vector2 axis) {
    Vector2 pts[4];
    pts[0] = (Vector2){r.x,r.y};
    pts[1] = (Vector2){r.x+r.width,r.y};
    pts[2] = (Vector2){r.x+r.width,r.y+r.height};
    pts[3] = (Vector2){r.x,r.y+r.height};
    return ProjectPolygonOnAxis(pts,4,axis);
}

Vector2 ProjectPolygonOnAxis(Vector2* vertices, int vertexCount, Vector2 axis) {
    float min = V2Dot(axis, vertices[0]);
    float max = min;
    for(int i = 1; i < vertexCount; ++i) {
        float dot = V2Dot(axis, vertices[i]);
        if(dot < min) {
            min = dot;
        } else if(dot > max) {
            max = dot;
        }
    }
    return (Vector2){min,max};
}

float GetOverlap(Vector2 a, Vector2 b) {
    return Max(0, Min(a.y, b.y) - Max(a.x, b.x));
}

float GetSignedOverlap(Vector2 moving, Vector2 still) {
    float movingHW = moving.x + moving.y/2;
    float stillHW = still.x + still.y/2;
    float sign = 1;
    if(movingHW < stillHW) sign = -1;
    return sign*GetOverlap(moving,still);
}

bool CheckCollisionPointRect(Vector2 pt, Rect r) {
    return (pt.x > r.x && pt.x < r.x+r.width) && (pt.y > r.y && pt.y < r.y+r.height);
}

Vector2 ResolveRectCollision(Rect moving, Rect still, float* overlap) {
    *overlap = 100000;
    Vector2 resolveAxis = (Vector2){};
    const Vector2 axes[] = {(Vector2){1,0}, (Vector2){0,1}};
    for(int i = 0; i < 2; ++i) {
        Vector2 movingProj = ProjectRectangleOnAxis(moving, axes[i]);
        Vector2 stillProj = ProjectRectangleOnAxis(still, axes[i]);
        float over = GetSignedOverlap(movingProj, stillProj);
        if(over == 0) {
            *overlap = 0;
            break;
        }
        if(fabs(over) < fabs(*overlap)) {
            *overlap = over;
            resolveAxis = axes[i];
        }
    }
    return resolveAxis;
}
