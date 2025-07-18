#version 450 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

uniform vec2 scale = vec2(1.0, 1.0);
uniform vec2 offset = vec2(0.0, 0.0);

out vec2 vertex_color;

void main() {
    // Apply scaling and offset to handle aspect ratio differences
    vec2 scaled_pos = position * scale + offset;
    gl_Position = vec4(scaled_pos, 0.0, 1.0);
    vertex_color = texcoord;
}
