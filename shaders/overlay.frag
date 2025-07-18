#version 450 core

in vec2 vertex_color;

uniform sampler2D image_texture;
uniform float alpha;

out vec4 fragment_color;

void main() {
    vec4 tex_color = texture(image_texture, vertex_color);
    fragment_color = vec4(tex_color.rgb, tex_color.a * alpha);
}
