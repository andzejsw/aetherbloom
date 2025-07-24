#ifndef FRUSTUM_H
#define FRUSTUM_H

#include "../util/util.h"
#include "../util/aabb.h"

// a plane is defined by a normal and a distance from the origin
typedef struct {
    vec3s n;
    f32 d;
} Plane;

// a frustum has 6 planes
typedef struct {
    Plane planes[6];
} Frustum;

void frustum_extract(Frustum *self, const mat4s vp);
bool frustum_intersect(Frustum *self, AABB aabb);

#endif