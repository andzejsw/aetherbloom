#include "frustum.h"

// normalizes a plane
static void normalize(Plane *p) {
    f32 mag = glms_vec3_norm(p->n);
    p->n = glms_vec3_scale(p->n, 1.0f / mag);
    p->d /= mag;
}

void frustum_extract(Frustum *self, const mat4s vp) {
    // left
    self->planes[0].n.x = vp.raw[0][3] + vp.raw[0][0];
    self->planes[0].n.y = vp.raw[1][3] + vp.raw[1][0];
    self->planes[0].n.z = vp.raw[2][3] + vp.raw[2][0];
    self->planes[0].d = vp.raw[3][3] + vp.raw[3][0];

    // right
    self->planes[1].n.x = vp.raw[0][3] - vp.raw[0][0];
    self->planes[1].n.y = vp.raw[1][3] - vp.raw[1][0];
    self->planes[1].n.z = vp.raw[2][3] - vp.raw[2][0];
    self->planes[1].d = vp.raw[3][3] - vp.raw[3][0];

    // bottom
    self->planes[2].n.x = vp.raw[0][3] + vp.raw[0][1];
    self->planes[2].n.y = vp.raw[1][3] + vp.raw[1][1];
    self->planes[2].n.z = vp.raw[2][3] + vp.raw[2][1];
    self->planes[2].d = vp.raw[3][3] + vp.raw[3][1];

    // top
    self->planes[3].n.x = vp.raw[0][3] - vp.raw[0][1];
    self->planes[3].n.y = vp.raw[1][3] - vp.raw[1][1];
    self->planes[3].n.z = vp.raw[2][3] - vp.raw[2][1];
    self->planes[3].d = vp.raw[3][3] - vp.raw[3][1];

    // near
    self->planes[4].n.x = vp.raw[0][3] + vp.raw[0][2];
    self->planes[4].n.y = vp.raw[1][3] + vp.raw[1][2];
    self->planes[4].n.z = vp.raw[2][3] + vp.raw[2][2];
    self->planes[4].d = vp.raw[3][3] + vp.raw[3][2];

    // far
    self->planes[5].n.x = vp.raw[0][3] - vp.raw[0][2];
    self->planes[5].n.y = vp.raw[1][3] - vp.raw[1][2];
    self->planes[5].n.z = vp.raw[2][3] - vp.raw[2][2];
    self->planes[5].d = vp.raw[3][3] - vp.raw[3][2];

    for (size_t i = 0; i < 6; i++) {
        normalize(&self->planes[i]);
    }
}

bool frustum_intersect(Frustum *self, AABB aabb) {
    for (size_t i = 0; i < 6; i++) {
        // find the vertex of the AABB that is furthest in the direction of the plane's normal
        vec3s p = aabb[0];
        if (self->planes[i].n.x >= 0) p.x = aabb[1].x;
        if (self->planes[i].n.y >= 0) p.y = aabb[1].y;
        if (self->planes[i].n.z >= 0) p.z = aabb[1].z;

        // if this vertex is behind the plane, the AABB is outside the frustum
        if (glms_vec3_dot(self->planes[i].n, p) + self->planes[i].d < 0) {
            return false;
        }
    }

    return true;
}