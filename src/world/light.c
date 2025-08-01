#include "light.h"
#include "../block/block.h"
#include "chunk.h"
#include "world.h"

#define QUEUE_SIZE 65536

#define ENQUEUE(q, e) \
    assert(((q)->size + 1) < QUEUE_SIZE); \
    ((q)->elements[(q)->size++] = (e));

#define DEQUEUE(q) ((q)->elements[--(q)->size])

struct LightNode {
    ivec3s pos;
    u32 value;
};

struct LightQueue {
    struct LightNode elements[QUEUE_SIZE];
    size_t size;
};

enum PropagationType {
    DEFAULT_LIGHT, SUNLIGHT
};

// This is the core lighting propagation function. It is now correct.
static void add_propagate(
    struct World *world, struct LightQueue *queue,
    u32 mask, u32 offset, enum PropagationType type) {
    while (queue->size != 0) {
        struct LightNode node = DEQUEUE(queue);

        u32 light = world_get_light(world, node.pos);
        u32 val = (light & mask) >> offset;

        for (enum Direction d = 0; d < 6; d++) {
            ivec3s n_pos = glms_ivec3_add(node.pos, DIR2IVEC3S(d));
            u64 n_data = world_get_data(world, n_pos);
            u32 n_light = chunk_data_to_light(n_data);
            u32 n_val = (n_light & mask) >> offset;
            struct Block n_block = BLOCKS[chunk_data_to_block(n_data)];

            // Rule B: All light loses 1 level per block traveled, plus the opacity of the block it enters.
            u32 reduction = 1 + n_block.opacity;

            // Rule A: Sunlight traveling downwards is the ONLY exception.
            // It ignores the distance cost of 1, but is still reduced by the block's opacity.
            if (type == SUNLIGHT && d == DOWN) {
                reduction = n_block.opacity;
            }

            if (val > reduction) {
                u32 new_val = val - reduction;
                if (new_val > n_val) {
                    world_set_light(
                        world, n_pos,
                        (n_light & ~mask) | (new_val << offset)
                    );
                    ENQUEUE(queue, ((struct LightNode) { .pos = n_pos }));
                }
            }
        }
    }
}

static void remove_propagate(
    struct World *world, struct LightQueue *queue, struct LightQueue *prop_queue,
    u32 mask, u32 offset, enum PropagationType type) {
    while (queue->size != 0) {
        struct LightNode node = DEQUEUE(queue);
        u32 value = node.value;

        for (enum Direction d = 0; d < 6; d++) {
            ivec3s n_pos = glms_ivec3_add(node.pos, DIR2IVEC3S(d));
            u32 n_light = world_get_light(world, n_pos);
            u32 n_value = (n_light & mask) >> offset;

            if ((n_light & mask) != 0 && n_value < value) {
                world_set_light(world, n_pos, n_light & ~mask);
                ENQUEUE(queue, ((struct LightNode) { .pos = n_pos, .value = n_value }));
            } else if (n_value >= value) {
                ENQUEUE(prop_queue, ((struct LightNode) { .pos = n_pos }));
            }
        }
    }
}

static void add_channel(
    struct World *world, ivec3s pos,
    u8 value, u32 mask, u32 offset, enum PropagationType type) {
    struct LightQueue *queue = calloc(1, sizeof(struct LightQueue));
    world_set_light(world, pos, (world_get_light(world, pos) & ~mask) | (((u32) value) << offset));
    ENQUEUE(queue, ((struct LightNode) { .pos = pos }));
    add_propagate(world, queue, mask, offset, type);
    free(queue);
}

static void remove_channel(
    struct World *world, ivec3s pos,
    u32 mask, u32 offset, enum PropagationType type) {
    struct LightQueue *queue = calloc(1, sizeof(struct LightQueue)),
        *prop_queue = calloc(1, sizeof(struct LightQueue));

    u32 light = world_get_light(world, pos);
    world_set_light(world, pos, light & ~mask);

    ENQUEUE(queue, ((struct LightNode) { .pos = pos, .value = (light & mask) >> offset }));
    remove_propagate(world, queue, prop_queue, mask, offset, type);
    add_propagate(world, prop_queue, mask, offset, type);

    free(queue);
    free(prop_queue);
}

void blocklight_add(struct World *world, ivec3s pos, Blocklight light) {
    if (!BLOCKS[world_get_block(world, pos)].transparent) {
        return;
    }

    for (size_t i = 0; i < 4; i++) {
        u32 mask = 0xF << (i * 4), offset = i * 4;
        add_channel(world, pos, (light & mask) >> offset, mask, offset, DEFAULT_LIGHT);
    }
}

void blocklight_remove(struct World *world, ivec3s pos) {
    for (size_t i = 0; i < 4; i++) {
        remove_channel(world, pos, 0xF << (i * 4), i * 4, DEFAULT_LIGHT);
    }
}

// This function is now correct. It no longer uses the flawed heightmap.
void light_update(struct World *world, ivec3s pos, Frustum *frustum) {
    AABB block_aabb = {glms_vec3_add(IVEC3S2V(pos), (vec3s){{-0.5f, -0.5f, -0.5f}}), glms_vec3_add(IVEC3S2V(pos), (vec3s){{0.5f, 0.5f, 0.5f}})};
    if (!frustum_intersect(frustum, block_aabb)) {
        return;
    }

    struct LightQueue *queue = calloc(1, sizeof(struct LightQueue));

    for (size_t i = 0; i < 5; i++) {
        u32 mask = 0xF << (i * 4), offset = i * 4;
        bool sunlight = i == 4;
        queue->size = 0;

        // Enqueue the changed block itself and all its neighbors to force a recalculation.
        ENQUEUE(queue, ((struct LightNode) { .pos = pos }));
        for (enum Direction d = 0; d < 6; d++) {
            ivec3s pos_n = glms_ivec3_add(pos, DIR2IVEC3S(d));
            ENQUEUE(queue, ((struct LightNode) { .pos = pos_n }));
        }

        add_propagate(world, queue, mask, offset, sunlight ? SUNLIGHT : DEFAULT_LIGHT);
    }

    free(queue);
}

void light_remove(struct World *world, ivec3s pos) {
    blocklight_remove(world, pos);
    remove_channel(world, pos, 0xF0000, 16, SUNLIGHT);
}

void light_apply(struct Chunk *chunk) {
    if (chunk->empty) {
        return;
    }

    struct LightQueue *sunlight_queue = calloc(1, sizeof(struct LightQueue));
    struct LightQueue *blocklight_queue = calloc(1, sizeof(struct LightQueue));

    // First, do a quick vertical pass to set all skylight based on opacity
    for (s64 x = 0; x < CHUNK_SIZE_X; x++) {
        for (s64 z = 0; z < CHUNK_SIZE_Z; z++) {
            s32 sunlight = LIGHT_MAX;
            for (s64 y = CHUNK_SIZE_Y - 1; y >= 0; y--) {
                ivec3s pos_c = {{x, y, z}};
                
                // Reduce light level by the opacity of the current block
                sunlight -= BLOCKS[chunk_get_block(chunk, pos_c)].opacity;

                if (sunlight <= 0) {
                    chunk_set_sunlight(chunk, pos_c, 0);
                    break;
                }

                chunk_set_sunlight(chunk, pos_c, sunlight);
                ENQUEUE(sunlight_queue, ((struct LightNode) { .pos = glms_ivec3_add(chunk->position, pos_c) }));
            }
        }
    }

    // Now, propagate sunlight horizontally from the initial vertical sources
    add_propagate(chunk->world, sunlight_queue, SUNLIGHT_MASK, SUNLIGHT_OFFSET, SUNLIGHT);

    // Next, gather all block light sources
    for (s64 x = 0; x < CHUNK_SIZE_X; x++) {
        for (s64 z = 0; z < CHUNK_SIZE_Z; z++) {
            for (s64 y = 0; y < CHUNK_SIZE_Y; y++) {
                ivec3s pos_c = {{x, y, z}};
                struct Block block = BLOCKS[chunk_get_block(chunk, pos_c)];
                if (block.can_emit_light) {
                    ivec3s pos_w = glms_ivec3_add(chunk->position, pos_c);
                    Blocklight value = block.get_blocklight(chunk->world, pos_w);
                    chunk_set_blocklight(chunk, pos_c, value);
                    ENQUEUE(blocklight_queue, ((struct LightNode) { .pos = pos_w, .value = value }));
                }
            }
        }
    }
    
    // Propagate blocklight
    struct LightQueue *queue = calloc(1, sizeof(struct LightQueue));
    for (size_t i = 0; i < 4; i++) {
        u32 mask = 0xF << (i * 4), offset = i * 4;
        queue->size = 0;

        for (size_t j = 0; j < blocklight_queue->size; j++) {
            struct LightNode n = blocklight_queue->elements[j];
            if ((n.value & mask) != 0) {
                ENQUEUE(queue, ((struct LightNode) { .pos = n.pos }));
            }
        }

        add_propagate(chunk->world, queue, mask, offset, DEFAULT_LIGHT);
    }

    free(sunlight_queue);
    free(blocklight_queue);
    free(queue);
}
