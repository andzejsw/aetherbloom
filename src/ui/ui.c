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
        font_render_text(
            &state.renderer.font, "0",
            (vec2s){{0.0f, state.window->size.y - 48.0f}}, GLMS_VEC4_ONE, 1.0f);
    }
}