#include "ui.h"
#include "../world/chunk.h"
#include "../state.h"
#include "../entity/ecs.h"
#include "../entity/ecscomponents.h"
#include "../gfx/window.h" // Include window.h to access global window struct
#include "block_names.h"
#include <stdio.h>
#include <math.h>

static inline s32 floor_div(s32 a, s32 b) {
    s32 res = a / b;
    if ((a % b != 0) && ((a < 0) != (b < 0))) {
        res--;
    }
    return res;
}

static inline s32 pos_mod(s32 i, s32 n) {
    return (i % n + n) % n;
}

extern struct Window window; // Declare global window struct

#define DECL_UI_FN(_name)\
    void ui_##_name(struct UI *self) {\
        for (size_t i = 0; i < self->components.count; i++) {\
            struct UIComponent c = self->components.elements[i];\
            if (c._name != NULL && c.enabled) {\
                c._name(c.component);\
            }\
        }\
    }

DECL_UI_FN(destroy)
DECL_UI_FN(update)
DECL_UI_FN(tick)

void ui_render(struct UI *self) {
    for (size_t i = 0; i < self->components.count; i++) {
        struct UIComponent c = self->components.elements[i];
        if (c.render != NULL && c.enabled) {
            c.render(c.component);
        }
    }

    if (state.show_overlay) {
        struct PositionComponent *c_position = ecs_get(state.world.entity_load, C_POSITION);
        if (c_position) {
            char coords_str[64];
            snprintf(coords_str, sizeof(coords_str), "x: %.0f y: %.0f z: %.0f",
                     floorf(c_position->position.x),
                     floorf(c_position->position.y),
                     floorf(c_position->position.z));

            font_render_text(
                &state.renderer.font, coords_str,
                (vec2s){{10.0f, state.window->size.y - 34.0f}}, GLMS_VEC4_ONE, 1.0f);

            char fps_str[32];
            snprintf(fps_str, sizeof(fps_str), "FPS: %lld", window.fps);
            font_render_text(
                &state.renderer.font, fps_str,
                (vec2s){{10.0f, state.window->size.y - 68.0f}}, GLMS_VEC4_ONE, 1.0f);

            char light_str[64];
            Blocklight block_light = world_get_blocklight(&state.world, c_position->block);
            Sunlight sky_light = world_get_sunlight(&state.world, c_position->block);
            snprintf(light_str, sizeof(light_str), "Light: %d (Sky: %d, Block: %d)",
                     max(BLOCKLIGHT_I(block_light), sky_light),
                     sky_light,
                     BLOCKLIGHT_I(block_light));
            font_render_text(
                &state.renderer.font, light_str,
                (vec2s){{10.0f, state.window->size.y - 102.0f}}, GLMS_VEC4_ONE, 1.0f);

            char chunk_coords_str[64];
            ivec3s world_pos = c_position->block;
            ivec3s chunk_pos = {{
                floor_div(world_pos.x, CHUNK_SIZE_X),
                floor_div(world_pos.y, CHUNK_SIZE_Y),
                floor_div(world_pos.z, CHUNK_SIZE_Z)
            }};
            ivec3s inner_pos = {{
                pos_mod(world_pos.x, CHUNK_SIZE_X),
                pos_mod(world_pos.y, CHUNK_SIZE_Y),
                pos_mod(world_pos.z, CHUNK_SIZE_Z)
            }};
            snprintf(chunk_coords_str, sizeof(chunk_coords_str), "Chunk: %d %d %d / %d %d %d",
                     chunk_pos.x, chunk_pos.y, chunk_pos.z,
                     inner_pos.x, inner_pos.y, inner_pos.z);

            font_render_text(
                &state.renderer.font, chunk_coords_str,
                (vec2s){{10.0f, state.window->size.y - 136.0f}}, GLMS_VEC4_ONE, 1.0f);
        }

        struct BlockLookComponent *c_blocklook = ecs_get(state.world.entity_load, C_BLOCKLOOK);
        if (c_blocklook && c_blocklook->hit) {
            char block_str[64];
            enum BlockId block_id = world_get_block(&state.world, c_blocklook->pos);
            snprintf(block_str, sizeof(block_str), "Block: %s, x: %d, y: %d, z: %d",
                     block_names[block_id],
                     c_blocklook->pos.x,
                     c_blocklook->pos.y,
                     c_blocklook->pos.z);

            f32 width = font_get_text_width(&state.renderer.font, block_str, 1.0f);
            font_render_text(
                &state.renderer.font, block_str,
                (vec2s){{state.window->size.x - width - 10.0f, state.window->size.y - 10.0f - 30.0f}},
                GLMS_VEC4_ONE, 1.0f);
        }
    }
}