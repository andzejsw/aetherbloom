#ifndef LIGHT_H
#define LIGHT_H

#include "../util/util.h"
#include "../gfx/frustum.h"

// forward declarations
struct Chunk;
struct World;

#define LIGHT_MAX 15

// Blocklight is a 16-bit value that stores 4 channels of light data:
// - R, G, B: For colored light
// - I: For intensity (used for block light sources like torches)
typedef u16 Blocklight;

// Sunlight is an 8-bit value representing the light from the sky.
typedef u8 Sunlight;

// Light is a 32-bit value that combines blocklight and sunlight.
typedef u32 Light;

// Macros for extracting R, G, B, and Intensity channels from a Blocklight value.
#define BLOCKLIGHT_R(d) (Blocklight) (((d) & 0xF000) >> 12)
#define BLOCKLIGHT_G(d) (Blocklight) (((d) & 0x0F00) >>  8)
#define BLOCKLIGHT_B(d) (Blocklight) (((d) & 0x00F0) >>  4)
#define BLOCKLIGHT_I(d) (Blocklight) (((d) & 0x000F) >>  0)

// Macros for setting R, G, B, and Intensity channels in a Blocklight value.
#define BLOCKLIGHT_SET_R(d, r) (Blocklight) (((d) & ~0xF000) | ((r) << 12))
#define BLOCKLIGHT_SET_G(d, g) (Blocklight) (((d) & ~0x0F00) | ((g) <<  8))
#define BLOCKLIGHT_SET_B(d, b) (Blocklight) (((d) & ~0x00F0) | ((b) <<  4))
#define BLOCKLIGHT_SET_I(d, i) (Blocklight) (((d) & ~0x000F) | ((i) <<  0))

// Creates a Blocklight value from R, G, B, and Intensity components.
#define BLOCKLIGHT_OF(r, g, b, i) (\
    (((Blocklight) (r)) << 12) |\
    (((Blocklight) (g)) <<  8) |\
    (((Blocklight) (b)) <<  4) |\
    (((Blocklight) (i)) <<  0))

// Combines sunlight and blocklight into a single Light value.
#define LIGHT_OF(_sun, _torch) ((((u32) (_sun)) << SUNLIGHT_OFFSET) | ((u32) (_torch)))

// Adds a light source at the given position and propagates its light.
void light_add(struct World *world, ivec3s pos, Light light);

// Removes a light source at the given position and updates the surrounding light.
void light_remove(struct World *world, ivec3s pos);

// Applies initial lighting to a chunk. This includes calculating vertical sunlight
// and propagating all light sources within the chunk.
void light_apply(struct Chunk *chunk);

// Updates the light around a specific position. This is typically called
// when a block is placed or removed.
void light_update(struct World *world, ivec3s pos);

#endif