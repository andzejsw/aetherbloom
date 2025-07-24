#include "vbo.h"

struct VBO vbo_create(GLint type, bool dynamic) {
    struct VBO self = {
        .type = type,
        .dynamic = dynamic,
        .size = 0
    };
    glGenBuffers(1, &self.handle);
    return self;
}

void vbo_destroy(struct VBO self) {
    glDeleteBuffers(1, &self.handle);
}

void vbo_bind(struct VBO self) {
    glBindBuffer(self.type, self.handle);
}

void vbo_buffer(struct VBO *self, void *data, size_t offset, size_t count) {
    vbo_bind(*self);
    size_t data_size = count - offset;
    if (data_size <= self->size) {
        glBufferSubData(self->type, offset, data_size, data);
    } else {
        glBufferData(self->type, data_size, data, self->dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        self->size = data_size;
    }
}