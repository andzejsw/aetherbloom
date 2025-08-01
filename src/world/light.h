#ifndef LIGHT_H
#define LIGHT_H

#include "../util/util.h"
#include "../gfx/frustum.h"

// forward declarations
struct Chunk;
struct World;

#define LIGHT_MAX 15

typedef u16 Blocklight;
typedef u8 Sunlight;
typedef u32 Light;

#define BLOCKLIGHT_R(d) (Blocklight) (((d) & 0xF000) >> 12)
#define BLOCKLIGHT_G(d) (Blocklight) (((d) & 0x0F00) >>  8)
#define BLOCKLIGHT_B(d) (Blocklight) (((d) & 0x00F0) >>  4)
#define BLOCKLIGHT_I(d) (Blocklight) (((d) & 0x000F) >>  0)

#define BLOCKLIGHT_SET_R(d, r) (Blocklight) (((d) & ~0xF000) | ((r) << 12))
#define BLOCKLIGHT_SET_G(d, g) (Blocklight) (((d) & ~0x0F00) | ((g) <<  8))
#define BLOCKLIGHT_SET_B(d, b) (Blocklight) (((d) & ~0x00F0) | ((b) <<  4))
#define BLOCKLIGHT_SET_I(d, i) (Blocklight) (((d) & ~0x000F) | ((i) <<  0))

#define BLOCKLIGHT_OF(r, g, b, i) (\
    (((Blocklight) (r)) << 12) |\
    (((Blocklight) (g)) <<  8) |\
    (((Blocklight) (b)) <<  4) |\
    (((Blocklight) (i)) <<  0))

#define LIGHT_OF(_sun, _torch) ((((u32) (_sun)) << SUNLIGHT_OFFSET) | ((u32) (_torch)))

void blocklight_add(struct World *world, ivec3s pos, Blocklight light);
void blocklight_remove(struct World *world, ivec3s pos);

void light_remove(struct World *world, ivec3s pos);
void light_apply(struct Chunk *chunk);
void light_update(struct World *world, ivec3s pos, Frustum *frustum);

#endif