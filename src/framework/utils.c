#include <SDL3/SDL.h>
#include <stdlib.h>
#include "utils.h"

float Min(float a, float b) {
    return a < b ? a : b;
}

float Max(float a, float b) {
    return a > b ? a : b;
}

float Sign(float a) {
    return a / fabs(a);
}

float Clamp(float a, float min, float max) {
    return a > max ? max : a < min ? min : a;
}

float Lerp(float a, float b, float t) {
    return b + (a-b)*t;
}

float ExpDecay(float a, float b, float decay, float dt) {
    return b + (a-b)*expf(-decay*dt);
}

float CubicHermite(float a, float aprime, float b, float bprime, float t) {
    float t3 = t*t*t;
    float t2 = t*t;
    return a*(2*t3 -  3*t2 + 1) + aprime*(t3 + 2*t2 + t) + b*(-2*t3 + 3*t2) + bprime*(t3 - t2);
}

float KBSpline(float previous, float current, float next, float tension, float bias, float continuity, float t) {
    float fact1 = 0.5*(1-tension)*(1 + bias)*(1-continuity);
    float fact2 = 0.5*(1-tension)*(1 - bias)*(1+continuity);
    float fact3 = 0.5*(1-tension)*(1 + bias)*(1+continuity);
    float fact4 = 0.5*(1-tension)*(1 - bias)*(1-continuity);
    float dc = fact1*(current-previous) + fact2*(next-current);
    float dn = fact3*(current-previous) + fact4*(next-current);
    return CubicHermite(current, dc, next, dn, t);
}

void Spring(float* a, float* da, float b, float zeta, float omega, float dt) {
    if(a == NULL || da == NULL) return;
    float f = 1 + 2 * dt * zeta * omega;
    float o2 = omega*omega;
    float dto2 = dt*o2;
    float dt2o2 = dt*dto2;
    float detInv = 1 / (f + dt2o2);
    float detA = f * (*a) + dt*(*da) + dt2o2*b;
    float detDA = (*da) + dto2*(b-(*a));
    *a = detA * detInv;
    *da = detDA * detInv;
}

void SpringDecayTime(float* a, float* da, float b, float decay, float decayTime, float freq, float dt) {
    float omega = 2*M_PI*freq;
    float zeta = log(1-decay)/(-omega*decayTime);
    Spring(a, da, b, zeta, omega, dt);
}

Vector2 V2Add(Vector2 a, Vector2 b) {
    return (Vector2){a.x+b.x,a.y+b.y};
}

Vector2 V2Subtract(Vector2 a, Vector2 b) {
    return (Vector2){a.x-b.x,a.y-b.y};
}

float V2Dot(Vector2 a, Vector2 b) {
    return a.x*b.x + a.y*b.y;
}

Vector2 V2Cross(Vector2 a, Vector2 b) {
    return (Vector2){a.x*b.x,a.y*b.y};
}

Vector2 V2Invert(Vector2 a) {
    return (Vector2){1/a.x, 1/a.y};
}

Vector2 V2Scale(Vector2 a, float b) {
    return (Vector2){a.x*b, a.y*b};
}

Vector2 V2Normalize(Vector2 a) {
    return V2Scale(a, 1/V2Length(a));
}

float V2Length(Vector2 a) {
    return sqrt(a.x*a.x + a.y*a.y);
}

float V2LengthSquared(Vector2 a) {
    return a.x*a.x + a.y*a.y;
}

float V2Distance(Vector2 a, Vector2 b) {
    return V2Length(V2Subtract(b,a));
}

Vector2 V2Clamp(Vector2 a, Vector2 min, Vector2 max) {
    return (Vector2){Clamp(a.x,min.x,max.x), Clamp(a.y,min.y,max.y)};
}

Vector2 V2Project(Vector2 a, Vector2 b) {
    float dot = V2Dot(a,b);
    return (Vector2){( dot / (b.x*b.x + b.y*b.y) ) * b.x, ( dot / (b.x*b.x + b.y*b.y) ) * b.y};
}

Vector2 V2Lerp(Vector2 a, Vector2 b, float t) {
    return (Vector2){Lerp(a.x,b.x,t), Lerp(a.y,b.y, t)};
}

Vector2 V2ExpDecay(Vector2 a, Vector2 b, float decay, float dt) {
    return (Vector2){ExpDecay(a.x,b.x, decay, dt), ExpDecay(a.y,b.y, decay, dt)};
}

Vector2 V2CubicHermite(Vector2 a, Vector2 aprime, Vector2 b, Vector2 bprime, float t) {
    return (Vector2){
        CubicHermite(a.x, aprime.x, b.x, bprime.x, t),
        CubicHermite(a.y, aprime.y, b.y, bprime.y, t)
    };
}


Vector2 V2KBSpline(Vector2 previous, Vector2 current, Vector2 next, float tension, float bias, float continuity, float t) {
    return (Vector2){
        KBSpline(previous.x,current.x,next.x,tension, bias, continuity, t),
        KBSpline(previous.y,current.y,next.y,tension, bias, continuity, t)
    };
}

void V2Spring(Vector2* a, Vector2* da, Vector2 b, float zeta, float omega, float dt) {
    Spring(&a->x, &da->x, b.x, zeta, omega, dt);
    Spring(&a->y, &da->y, b.y, zeta, omega, dt);
}

void V2SpringDecayTime(Vector2* a, Vector2* da, Vector2 b, float decay, float decayTime, float freq, float dt) {
    SpringDecayTime(&a->x, &da->x, b.x, decay, decayTime, freq, dt);
    SpringDecayTime(&a->y, &da->y, b.y, decay, decayTime, freq, dt);
}


float RunTimedFunction(TimedFunction f, float dt, void* data, float* accumulator, float* time) {
    uint64_t newTime = SDL_GetTicksNS();
    uint64_t frameTimeNS = newTime - *time;
    double frameTime = frameTimeNS/1000000000.0;
    *time = newTime;
    *accumulator += frameTime;

    while(*accumulator >= dt) {
        f(data, dt);
        *accumulator -= dt;
    }
    return *accumulator / dt;
}
