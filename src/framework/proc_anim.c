#include "proc_anim.h"
#include "utils.h"
#include <stdio.h>

void TwoBoneIK(Vector2 origin, Vector2 target, float lengthA, float lengthB, bool angleBPolarity, float* angleA, float* angleB) {
    float epsilon = 0.00001;
    if(lengthA < epsilon || lengthB < epsilon) {
        // *angleB = 0;
        *angleA = 0;
        return;
    } 

    // Solving for angleB
    Vector2 p = V2Subtract(target, origin);
    float cosAngleB = ((p.x*p.x) + (p.y*p.y) - (lengthA*lengthA) - (lengthB*lengthB))/(2*lengthA*lengthB);
    cosAngleB = Clamp(cosAngleB, -1, 1);
    if(angleBPolarity) {
        *angleB = acosf(cosAngleB);
    } else {
        *angleB = -acosf(cosAngleB);
    }

    // Solving for angleA
    *angleA = atan2(p.y*(lengthA + lengthB*cos(*angleB)) - p.x*(lengthB*sin(*angleB)), p.x*(lengthA + lengthB*cos(*angleB)) + p.y*(lengthB*sin(*angleB)));
}

void ChainIK(Vector2* points, float* lengths, Vector2 target, int pointCount) {
    if(!points || !lengths) return;
    points[pointCount-1] = target;
    for(int i = pointCount-2; i >= 0; --i) {
        Vector2 dir = V2Normalize(V2Subtract(points[i], points[i+1]));
        points[i] = V2Add(points[i+1],V2Scale(dir, lengths[i]));
    }
}

void FabrIK(Vector2* points, float* lengths, Vector2 endEffector, int pointCount) {
    if(!points || !lengths) return;
    Vector2 origin = points[0];
    // Forward reaching
    ChainIK(points, lengths, endEffector, pointCount);
    // Backward reaching
    points[0] = origin;
    for(int i = 0; i < pointCount-1; ++i) {
        Vector2 dir = V2Normalize(V2Subtract(points[i+1], points[i]));
        points[i+1] = V2Add(points[i], V2Scale(dir, lengths[i]));
    }
}

VerletData* InitVerlet(Arena* arena, int maxPoints, int maxBones) {
    VerletData* verlet = ArenaAllocStruct(arena,VerletData);
    verlet->positions = ArenaAllocArray(arena,Vector2 ,maxPoints);
    verlet->lastPositions = ArenaAllocArray(arena,Vector2 ,maxPoints);
    verlet->weights = ArenaAllocArray(arena,float ,maxPoints);
    verlet->pinned = ArenaAllocArray(arena,bool ,maxPoints);
    verlet->bones = ArenaAllocArray(arena,VerletBone ,maxBones);

    verlet->pointCount = 0;
    verlet->boneCount = 0;
    return verlet;
}

int AddVerletPoint(VerletData* verlet, Vector2 pos, float weight, bool pinned) {
    int idx = verlet->pointCount;
    verlet->positions[idx] = pos;
    verlet->lastPositions[idx] = pos;
    verlet->weights[idx] = weight;
    verlet->pinned[idx] = pinned;
    verlet->pointCount++;
    return idx;
}

void AddVerletBone(VerletData* verlet, int pointA, int pointB, float distance, float minAngle, float maxAngle) {
    int idx = verlet->boneCount;
    verlet->bones[idx] = (VerletBone) {pointA, pointB, distance, 0, minAngle, maxAngle};
    verlet->boneCount++;
}

void UpdateVerlet(VerletData* verlet) {
    // Update point positions
    for(int i = 0; i < verlet->pointCount; ++i) {
        Vector2 v = V2Subtract(verlet->positions[i], verlet->lastPositions[i]);
        verlet->lastPositions[i] = verlet->positions[i];
        if(!verlet->pinned[i]) {
            verlet->positions[i] = V2Add(verlet->positions[i], v);
        }
    }

    // Apply bone constraints
    for(int i = 0; i < verlet->boneCount; ++i) {
        // Constrain lengths
        bool aPinned = verlet->pinned[verlet->bones[i].a];
        bool bPinned = verlet->pinned[verlet->bones[i].b];
        if(aPinned && bPinned) continue;
        Vector2 diff = V2Subtract(
            verlet->positions[verlet->bones[i].b],
            verlet->positions[verlet->bones[i].a]);
        Vector2 dir = V2Normalize(diff);
        float distChange = V2Length(diff) - verlet->bones[i].distance;
        if(aPinned) {
            verlet->positions[verlet->bones[i].b] = V2Add(verlet->positions[verlet->bones[i].a], V2Scale(dir,verlet->bones[i].distance));
        }
        else if(bPinned) {
            verlet->positions[verlet->bones[i].a] = V2Add(verlet->positions[verlet->bones[i].b], V2Scale(dir,-verlet->bones[i].distance));
        }
        else {
            float bias = verlet->weights[verlet->bones[i].a] / (verlet->weights[verlet->bones[i].a] + verlet->weights[verlet->bones[i].b]);
            verlet->positions[verlet->bones[i].a] = V2Add(verlet->positions[verlet->bones[i].a], V2Scale(dir, distChange*bias));
            verlet->positions[verlet->bones[i].b] = V2Subtract(verlet->positions[verlet->bones[i].b], V2Scale(dir, distChange*(1-bias)));
        }
        // Constrain angles
        if(verlet->bones[i].minAngle == verlet->bones[i].maxAngle) {
            continue;
        }
        float angle = atan2f(dir.y, dir.x);
        angle = Clamp(fmod(angle, 2*M_PI), verlet->bones[i].minAngle, verlet->bones[i].maxAngle);
        verlet->positions[verlet->bones[i].b] = V2Add(verlet->positions[verlet->bones[i].a], (Vector2){verlet->bones[i].distance*cos(angle), verlet->bones[i].distance*sin(angle)});
        verlet->bones[i].rotation = angle;
    }
}

