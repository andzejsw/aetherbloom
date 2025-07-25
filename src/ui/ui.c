#include "ui.h"
#include "../state.h"
#include "../entity/ecs.h"
#include "../entity/ecscomponents.h"
#include "../gfx/window.h" // Include window.h to access global window struct
#include "block_names.h"
#include <stdio.h>
#include <math.h>

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

            char light_str[32];
            snprintf(light_str, sizeof(light_str), "Light: %d", TORCHLIGHT_I(world_get_torchlight(&state.world, c_position->block)));
            font_render_text(
                &state.renderer.font, light_str,
                (vec2s){{10.0f, state.window->size.y - 102.0f}}, GLMS_VEC4_ONE, 1.0f);
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