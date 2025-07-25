#ifndef FMATH_H
#define FMATH_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"

#include <cglm/cglm.h>
#include <cglm/struct.h>

#pragma GCC diagnostic pop

#include "types.h"
#include "direction.h"

#define IVEC2S2V(v) ((vec2s) {{ (f32)(v).x, (f32)(v).y }})
#define VEC2S2I(v) ((ivec2s) {{ (int)(v).x, (int)(v).y }})
#define IVEC3S2V(v) ((vec3s) {{ (f32)(v).x, (f32)(v).y, (f32)(v).z }})
#define VEC3S2I(v) ((ivec3s) {{ (int)(v).x, (int)(v).y, (int)(v).z }})

#define IVEC2S23(v) ((ivec3s) {{ (v).x, (v).y, 0 }})

#define PI_2 GLM_PI_2
#define PI GLM_PI
#define TAU (2 * PI)
#define F32_EPSILON FLT_EPSILON
#define F64_EPSILON DBL_EPSILON

#define cosf(_x) ((f32)cos(_x))
#define sinf(_x) ((f32)sin(_x))
#define radians(_x) ((_x) * (__typeof__(_x)) (PI / 180.0f))
#define degrees(_x) ((_x) * (__typeof__(_x)) (180.0f / PI))
#define floori(_x) ((int) (floor((double) (_x))))
#define sign(x) (((x) > 0) - ((x) < 0))
#define lerpf(_a, _b, _t) ({ __typeof__(_t) _u = (_t); ((_a) * (1 - _u)) + ((_b) * _u); })
#define safe_expf(_x, _e) ({ __typeof__(_x) __x = (_x); __typeof__(_e) __e = (_e); sign(__x) * fabsf(powf(fabsf(__x), __e)); })

#define max(a, b) ((a) > (b) ? (a) : (b))

#define min(a, b) ((a) < (b) ? (a) : (b))

#define clamp(x, mn, mx) ({\
    __typeof__ (x) _x = (x); \
    __typeof__ (mn) _mn = (mn); \
    __typeof__ (mx) _mx = (mx); \
    max(_mn, min(_mx, _x)); })

#define SRAND(seed) srand(seed)
#define RAND(min, max) ((rand() % (max - min + 1)) + min)
#define RANDCHANCE(chance) ((RAND(0, 100000) / 100000.0) <= chance)

int ivec3scmp(ivec3s a, ivec3s b);
s64 ivec3shash(ivec3s v);

struct Ray {
    vec3s origin, direction;
};

extern bool ray_block(struct Ray ray, f32 max_distance, void *arg, bool (*f)(void *, ivec3s), ivec3s *out, enum Direction *d_out);

#endif