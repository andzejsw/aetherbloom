#ifndef FONT_H
#define FONT_H

#include "gfx.h"
#include "texture.h"
#include "vao.h"
#include "vbo.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#define FONT_ATLAS_SIZE 512
#define FONT_CHAR_COUNT 128 // ASCII characters

struct Character {
    vec2s size;     // Size of glyph
    vec2s bearing;  // Offset from baseline to left/top of glyph
    u32 advance;    // Offset to advance to next glyph
    vec2s uv_min;   // UV coordinates in atlas
    vec2s uv_max;   // UV coordinates in atlas
};

struct Font {
    FT_Library ft;
    FT_Face face;
    struct Texture atlas_texture;
    struct Character characters[FONT_CHAR_COUNT];
    struct VAO vao;
    struct VBO vbo;
};

void font_init(struct Font *self, char *path, u32 font_size);
void font_destroy(struct Font *self);
void font_render_text(struct Font *self, char *text, vec2s position, vec4s color, f32 scale);

#endif