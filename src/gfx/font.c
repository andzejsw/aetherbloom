#include "font.h"
#include "shader.h"
#include "../state.h"

#include <stdio.h>
#include <string.h>

void font_init(struct Font *self, char *path, u32 font_size) {
    if (FT_Init_FreeType(&self->ft)) {
        fprintf(stderr, "ERROR::FREETYPE: Could not init FreeType Library\n");
        return;
    }

    if (FT_New_Face(self->ft, path, 0, &self->face)) {
        fprintf(stderr, "ERROR::FREETYPE: Failed to load font\n");
        return;
    }

    FT_Set_Pixel_Sizes(self->face, 0, font_size);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Create font atlas texture
    glGenTextures(1, &self->atlas_texture.handle);
    glBindTexture(GL_TEXTURE_2D, self->atlas_texture.handle);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        FONT_ATLAS_SIZE,
        FONT_ATLAS_SIZE,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        NULL
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    self->atlas_texture.size = (ivec2s){{FONT_ATLAS_SIZE, FONT_ATLAS_SIZE}};

    // Load glyphs into atlas
    u32 x_offset = 0, y_offset = 0;
    u32 row_height = 0;

    for (u8 c = 0; c < FONT_CHAR_COUNT; c++) {
        if (FT_Load_Char(self->face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "ERROR::FREETYPE: Failed to load Glyph %c\n", c);
            continue;
        }

        if (x_offset + self->face->glyph->bitmap.width > FONT_ATLAS_SIZE) {
            x_offset = 0;
            y_offset += row_height;
            row_height = 0;
        }

        if (y_offset + self->face->glyph->bitmap.rows > FONT_ATLAS_SIZE) {
            fprintf(stderr, "ERROR::FREETYPE: Font atlas too small!\n");
            break;
        }

        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            x_offset,
            y_offset,
            self->face->glyph->bitmap.width,
            self->face->glyph->bitmap.rows,
            GL_RED,
            GL_UNSIGNED_BYTE,
            self->face->glyph->bitmap.buffer
        );

        self->characters[c] = (struct Character) {
            .size = (vec2s){{self->face->glyph->bitmap.width, self->face->glyph->bitmap.rows}},
            .bearing = (vec2s){{self->face->glyph->bitmap_left, self->face->glyph->bitmap_top}},
            .advance = self->face->glyph->advance.x,
            .uv_min = (vec2s){{(f32)x_offset / FONT_ATLAS_SIZE, (f32)y_offset / FONT_ATLAS_SIZE}},
            .uv_max = (vec2s){{
                (f32)(x_offset + self->face->glyph->bitmap.width) / FONT_ATLAS_SIZE,
                (f32)(y_offset + self->face->glyph->bitmap.rows) / FONT_ATLAS_SIZE
            }}
        };

        x_offset += self->face->glyph->bitmap.width;
        row_height = max(row_height, self->face->glyph->bitmap.rows);
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // Restore default byte-alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(self->face);
    FT_Done_FreeType(self->ft);

    // Setup VAO and VBO for rendering glyphs
    self->vao = vao_create();
    self->vbo = vbo_create(GL_ARRAY_BUFFER, true);

    vao_attr(self->vao, self->vbo, 0, 4, GL_FLOAT, 0, 0); // x, y, u, v
}

void font_destroy(struct Font *self) {
    texture_destroy(self->atlas_texture);
    vao_destroy(self->vao);
    vbo_destroy(self->vbo);
}

void font_render_text(struct Font *self, char *text, vec2s position, vec4s color, f32 scale) {
    renderer_use_shader(&state.renderer, SHADER_BASIC_TEXT);
    shader_uniform_mat4(state.renderer.shader, "projection", state.renderer.ortho_camera.view_proj.proj);
    shader_uniform_texture2D(state.renderer.shader, "text", self->atlas_texture, 0);
    shader_uniform_vec4(state.renderer.shader, "textColor", color);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, self->atlas_texture.handle);

    vao_bind(self->vao);

    vec2s text_pos = position;

    for (char *p = text; *p; p++) {
        struct Character ch = self->characters[(u8)*p];

        f32 xpos = text_pos.x + ch.bearing.x * scale;
        f32 ypos = text_pos.y - (ch.size.y - ch.bearing.y) * scale;

        f32 w = ch.size.x * scale;
        f32 h = ch.size.y * scale;

        // Update VBO for each character
        f32 vertices[6][4] = {
            {xpos,     ypos + h,   ch.uv_min.x, ch.uv_max.y},
            {xpos,     ypos,       ch.uv_min.x, ch.uv_min.y},
            {xpos + w, ypos,       ch.uv_max.x, ch.uv_min.y},

            {xpos,     ypos + h,   ch.uv_min.x, ch.uv_max.y},
            {xpos + w, ypos,       ch.uv_max.x, ch.uv_min.y},
            {xpos + w, ypos + h,   ch.uv_max.x, ch.uv_max.y}
        };

        vbo_buffer(&self->vbo, (f32*)vertices, 0, sizeof(vertices));

        glDrawArrays(GL_TRIANGLES, 0, 6);

        text_pos.x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (for 1/64th pixels))
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    vao_unbind();
}
