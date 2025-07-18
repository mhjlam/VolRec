#version 450 core

in vec4 vertex_color;
in vec3 world_position;

out vec4 fragment_color;

void main() {
    // Use position-based coloring similar to instanced cubes
    vec3 debug_color = abs(world_position) / 500.0; // Scale to visible range
    vec3 base_color = vertex_color.rgb;
    
    // Mix the base color with position-based color for variety
    if (length(debug_color) > 0.01) {
        base_color = mix(base_color, debug_color, 0.7);
    }
    
    fragment_color = vec4(base_color, vertex_color.a);
}
