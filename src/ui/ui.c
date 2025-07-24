#include "ui.h"
#include "../state.h"

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
        renderer_quad_texture(
            &state.renderer, state.renderer.textures[TEXTURE_ZERO],
            (vec2s){{32.0f, 32.0f}}, GLMS_VEC4_ONE,
            GLMS_VEC2_ZERO, GLMS_VEC2_ONE,
            glms_translate(glms_mat4_identity(), (vec3s){{0.0f, state.window->size.y - 32.0f, 0.0f}}));
    }
}