#ifndef H_UTILS
#define H_UTILS

#include <math.h>
#include "types.h"

// General utility functions, mostly math

// Extra float math
float Min(float a, float b);
float Max(float a, float b);
float Sign(float a);
float Clamp(float a, float min, float max);
float Lerp(float a, float b, float t); // Lerp (DON'T USE FOR MOTION)
// Freya Holmer's framerate independent exponential decay function
// Useful exponential decay range is around [1,50]
// Smaller decay = slower
float ExpDecay(float a, float b, float decay, float dt);  
// Cubic interpolation, where t is a value on [0..1]
float CubicHermite(float a, float aprime, float b, float bprime, float t);
// Kochanek-Bartels spline
float KBSpline(float previous, float current, float next, float tension, float bias, float continuity, float t);
// Spring damping as described in this article: https://allenchou.net/2015/04/game-math-precise-control-over-numeric-springing/
void Spring(float* a, float* da, float b, float zeta, float omega, float dt);
void SpringDecayTime(float* a, float* da, float b, float decay, float decayTime, float freq, float dt);

// Basic math operations on Vector2
Vector2 V2Add(Vector2 a, Vector2 b);
Vector2 V2Subtract(Vector2 a, Vector2 b);
float V2Dot(Vector2 a, Vector2 b);
Vector2 V2Cross(Vector2 a, Vector2 b);
Vector2 V2Invert(Vector2 a); // changes a to 1/a
Vector2 V2Scale(Vector2 a, float b);
Vector2 V2Normalize(Vector2 a);
float V2Length(Vector2 a);
float V2LengthSquared(Vector2 a); // Useful for saving some time on occasion
float V2Distance(Vector2 a, Vector2 b);

// Extra Vector2 math
Vector2 V2Project(Vector2 a, Vector2 b); // Project a onto b
Vector2 V2Clamp(Vector2 a, Vector2 min, Vector2 max);
Vector2 V2Lerp(Vector2 a, Vector2 b, float t);
Vector2 V2ExpDecay(Vector2 a, Vector2 b, float decay, float dt);
Vector2 V2CubicHermite(Vector2 a, Vector2 aprime, Vector2 b, Vector2 bprime, float t);
Vector2 V2KBSpline(Vector2 previous, Vector2 current, Vector2 next, float tension, float bias, float continuity, float t);
void V2Spring(Vector2* a, Vector2* da, Vector2 b, float zeta, float omega, float dt);
void V2SpringDecayTime(Vector2* a, Vector2* da, Vector2 b, float decay, float decayTime, float freq, float dt);

// Timing stuff
typedef void (*TimedFunction)(void*, float);
float RunTimedFunction(TimedFunction f, float dt, void* data, float* accumulator, float* time); // Returns 'alpha' value to blend

#endif
