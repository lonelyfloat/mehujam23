#ifndef H_PROC_ANIM
#define H_PROC_ANIM

#include "arena.h"
#include "types.h"

// Utilities for various procedural animation things likely to be re-used across projects

// Law of cosines two-bone ik, returns angles
void TwoBoneIK(Vector2 origin, Vector2 target, float lengthA, float lengthB, bool angleBPolarity, float* angleA, float* angleB);

// Chain that follows the target (no root position)
void ChainIK(Vector2* points, float* lengths, Vector2 target, int pointCount);

// Iterates the FABRIK algorithm once
void FabrIK(Vector2* points, float* lengths, Vector2 endEffector, int pointCount);

// *VERY SIMPLE* particle physics (for dynamic hair, ropes, etc...)

typedef struct VerletBone {
    int a, b;
    float distance;
    float rotation; // current rotation in radians, changes over time
    float minAngle, maxAngle; // in radians
} VerletBone;

typedef struct VerletData {
    Vector2* positions;
    Vector2* lastPositions;
    float* weights; // each point has a weight that determines how much it affects points around it
    bool* pinned;
    int pointCount;

    VerletBone* bones;
    int boneCount;
} VerletData;

VerletData* InitVerlet(Arena* arena, int maxPoints, int maxBones); 
int AddVerletPoint(VerletData* verlet, Vector2 pos, float weight, bool pinned);
void AddVerletBone(VerletData* verlet, int pointA, int pointB, float distance, float minAngle, float maxAngle);
void UpdateVerlet(VerletData* verlet);

#endif
