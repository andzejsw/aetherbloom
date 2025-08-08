#include "light.h"
#include "../block/block.h"
#include "chunk.h"
#include "world.h"

/**
 * Propagates light from the sources in the queue.
 *
 * @param world The world.
 * @param queue The queue of light sources to propagate.
 * @param mask The bitmask for the light channel being propagated.
 * @param offset The bit offset for the light channel being propagated.
 * @param type The type of light being propagated.
 */
static void propagate_sunlight(struct World *world, struct LightQueue *queue) {
    while (queue->size > 0) {
        struct LightNode node = DEQUEUE(queue);
        u32 light_level = world_get_sunlight(world, node.pos);

        for (enum Direction d = 0; d < 6; d++) {
            ivec3s neighbor_pos = glms_ivec3_add(node.pos, DIR2IVEC3S(d));
            u64 neighbor_data = world_get_data(world, neighbor_pos);
            struct Block neighbor_block = BLOCKS[chunk_data_to_block(neighbor_data)];

            if (!neighbor_block.transparent) {
                continue;
            }

            u32 neighbor_light_level = chunk_data_to_sunlight(neighbor_data);
            
            u32 reduction = 1;

            if (light_level > reduction) {
                u32 new_light_level = light_level - reduction;
                if (new_light_level > neighbor_light_level) {
                    world_set_sunlight(world, neighbor_pos, new_light_level);
                    ENQUEUE(queue, ((struct LightNode) { .pos = neighbor_pos }));
                }
            }
        }
    }
}

static void propagate_blocklight(struct World *world, struct LightQueue *queue) {
    while (queue->size > 0) {
        struct LightNode node = DEQUEUE(queue);
        Blocklight light_level = world_get_blocklight(world, node.pos);

        for (enum Direction d = 0; d < 6; d++) {
            ivec3s neighbor_pos = glms_ivec3_add(node.pos, DIR2IVEC3S(d));
            u64 neighbor_data = world_get_data(world, neighbor_pos);
            struct Block neighbor_block = BLOCKS[chunk_data_to_block(neighbor_data)];

            if (!neighbor_block.transparent) {
                continue;
            }

            Blocklight neighbor_light_level = chunk_data_to_blocklight(neighbor_data);
            Blocklight new_neighbor_light_level = neighbor_light_level;
            bool changed = false;

            for (int i = 0; i < 4; i++) {
                u32 mask = 0xF << (i * 4);
                u8 current_channel = (light_level & mask) >> (i * 4);
                u8 neighbor_channel = (neighbor_light_level & mask) >> (i * 4);

                u8 reduction = 1;
                if (current_channel > reduction) {
                    u8 new_val = current_channel - reduction;
                    if (new_val > neighbor_channel) {
                        new_neighbor_light_level = (new_neighbor_light_level & ~mask) | (new_val << (i * 4));
                        changed = true;
                    }
                }
            }

            if (changed) {
                world_set_blocklight(world, neighbor_pos, new_neighbor_light_level);
                ENQUEUE(queue, ((struct LightNode) { .pos = neighbor_pos }));
            }
        }
    }
}

/**
 * Propagates the removal of light from the sources in the queue.
 *
 * @param world The world.
 * @param queue The queue of light sources to remove.
 * @param prop_queue A queue to be filled with light sources that need to be re-propagated.
 * @param mask The bitmask for the light channel being removed.
 * @param offset The bit offset for the light channel being removed.
 * @param type The type of light being removed.
 */


void light_update(struct World *world, ivec3s pos) {
    struct LightQueue *queue = calloc(1, sizeof(struct LightQueue));

    // Sunlight
    queue->size = 0;
    ENQUEUE(queue, ((struct LightNode) { .pos = pos }));
    for (enum Direction d = 0; d < 6; d++) {
        ivec3s pos_n = glms_ivec3_add(pos, DIR2IVEC3S(d));
        ENQUEUE(queue, ((struct LightNode) { .pos = pos_n }));
    }
    propagate_sunlight(world, queue);

    // Blocklight
    queue->size = 0;
    ENQUEUE(queue, ((struct LightNode) { .pos = pos }));
    for (enum Direction d = 0; d < 6; d++) {
        ivec3s pos_n = glms_ivec3_add(pos, DIR2IVEC3S(d));
        ENQUEUE(queue, ((struct LightNode) { .pos = pos_n }));
    }
    propagate_blocklight(world, queue);

    free(queue);
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

/**
 * Adds a light source to a single channel.
 */
static void add_channel(
    struct World *world, ivec3s pos,
    u8 value, u32 mask, u32 offset, enum PropagationType type) {
    struct LightQueue *queue = calloc(1, sizeof(struct LightQueue));
    world_set_light(world, pos, (world_get_light(world, pos) & ~mask) | (((u32) value) << offset));
    ENQUEUE(queue, ((struct LightNode) { .pos = pos }));
    if (type == SUNLIGHT) {
        propagate_sunlight(world, queue);
    } else {
        propagate_blocklight(world, queue);
    }
    free(queue);
}

/**
 * Removes a light source from a single channel.
 */
static void remove_channel(
    struct World *world, ivec3s pos,
    u32 mask, u32 offset, enum PropagationType type) {
    struct LightQueue *queue = calloc(1, sizeof(struct LightQueue)),
        *prop_queue = calloc(1, sizeof(struct LightQueue));

    u32 light = world_get_light(world, pos);
    world_set_light(world, pos, light & ~mask);

    ENQUEUE(queue, ((struct LightNode) { .pos = pos, .value = (light & mask) >> offset }));
    remove_propagate(world, queue, prop_queue, mask, offset, type);
    if (type == SUNLIGHT) {
        propagate_sunlight(world, prop_queue);
    } else {
        propagate_blocklight(world, prop_queue);
    }

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

void light_add(struct World *world, ivec3s pos, Light light) {
    if (!BLOCKS[world_get_block(world, pos)].transparent) {
        return;
    }

    for (size_t i = 0; i < 5; i++) {
        u32 mask = 0xF << (i * 4), offset = i * 4;
        add_channel(world, pos, (light & mask) >> offset, mask, offset, i == 4 ? SUNLIGHT : DEFAULT_LIGHT);
    }
}

void light_remove(struct World *world, ivec3s pos) {
    for (size_t i = 0; i < 5; i++) {
        u32 mask = 0xF << (i * 4), offset = i * 4;
        remove_channel(world, pos, mask, offset, i == 4 ? SUNLIGHT : DEFAULT_LIGHT);
    }
}

void light_apply(struct Chunk *chunk) {
    if (chunk->empty) {
        return;
    }

    struct LightQueue *sunlight_queue = calloc(1, sizeof(struct LightQueue));
    struct LightQueue *blocklight_queue = calloc(1, sizeof(struct LightQueue));

    // First, calculate vertical sunlight propagation.
    for (s64 x = 0; x < CHUNK_SIZE_X; x++) {
        for (s64 z = 0; z < CHUNK_SIZE_Z; z++) {
            s32 sunlight;

            // Check for chunk above
            ivec3s above_pos = glms_ivec3_add(chunk->position, (ivec3s){{x, CHUNK_SIZE_Y, z}});
            struct Chunk *above_chunk = world_get_chunk(chunk->world, world_pos_to_offset(above_pos));

            if (above_chunk != NULL) {
                sunlight = chunk_get_sunlight(above_chunk, (ivec3s){{x, 0, z}});
            } else {
                sunlight = LIGHT_MAX;
            }

            for (s64 y = CHUNK_SIZE_Y - 1; y >= 0; y--) {
                ivec3s pos_c = {{x, y, z}};
                
                BlockId block_id = chunk_get_block(chunk, pos_c);
                struct Block block = BLOCKS[block_id];

                if (block_id != AIR) {
                    if (!block.transparent) {
                        sunlight = 0;
                    } else {
                        sunlight--;
                    }
                }

                sunlight = max(0, sunlight);
                chunk_set_sunlight(chunk, pos_c, sunlight);
            }
        }
    }

    // Next, find all light sources in the chunk and add them to the queues.
    for (s64 x = 0; x < CHUNK_SIZE_X; x++) {
        for (s64 z = 0; z < CHUNK_SIZE_Z; z++) {
            for (s64 y = 0; y < CHUNK_SIZE_Y; y++) {
                ivec3s pos_c = {{x, y, z}};
                ivec3s pos_w = glms_ivec3_add(chunk->position, pos_c);

                // Sunlight
                if (chunk_get_sunlight(chunk, pos_c) > 0) {
                    ENQUEUE(sunlight_queue, ((struct LightNode) { .pos = pos_w }));
                }

                // Blocklight
                struct Block block = BLOCKS[chunk_get_block(chunk, pos_c)];
                if (block.can_emit_light) {
                    Blocklight value = block.get_blocklight(chunk->world, pos_w);
                    chunk_set_blocklight(chunk, pos_c, value);
                    ENQUEUE(blocklight_queue, ((struct LightNode) { .pos = pos_w, .value = value }));
                }
            }
        }
    }

    // Propagate sunlight.
    propagate_sunlight(chunk->world, sunlight_queue);
    
    // Propagate block light.
    propagate_blocklight(chunk->world, blocklight_queue);

    free(sunlight_queue);
    free(blocklight_queue);
}