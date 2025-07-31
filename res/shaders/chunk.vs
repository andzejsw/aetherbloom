#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;
layout (location = 2) in uint color;

uniform mat4 m, v, p;

uniform vec4 sunlight_color;

// should match enum Direction in direction.h
const uint NORTH = 0U;
const uint SOUTH = 1U;
const uint EAST = 2U;
const uint WEST = 3U;
const uint UP = 4U;
const uint DOWN = 5U;

out vec4 v_color;
out vec2 v_uv;
out vec3 v_viewpos;

void main() {
    gl_Position = p * v * m * vec4(position, 1.0);
    
    // 'color' is packed:
    // - (3) face direction, NORTH, SOUTH, EAST, WEST, UP, DOWN
    // - (4) sunlight intensity
    // - (4) R
    // - (4) G
    // - (4) B
    // - (4) intensity
    // block light
    vec3 block_light_color = vec3(
        float((color & 0x0F000U) >> 12U) / 15.0,
        float((color & 0x00F00U) >>  8U) / 15.0,
        float((color & 0x000F0U) >>  4U) / 15.0
    );
    uint block_light_level = (color & 0x0000FU);
    float block_light_factor = pow(0.8, 0.6 * (15.0 - float(block_light_level)));
    vec3 block_light = block_light_color * block_light_factor;

    // sun light
    uint sun_light_level = (color & 0xF0000U) >> 16U;
    float sun_light_factor = pow(0.8, 0.6 * (15.0 - float(sun_light_level)));
    vec3 sun_light = vec3(sunlight_color.rgb) * sun_light_factor;

    vec3 light = max(sun_light, block_light);

    v_color = vec4(light, 1.0);

    // v_color = vec4(vec3(sunlight_color.rgb) * (((color & 0xF0000U) >> 16U) / 15.0), 1.0);

    v_uv = uv;
    v_viewpos = ((v * m) * vec4(position, 1.0)).xyz;
}