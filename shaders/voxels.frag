#version 450 core

in vec3 world_position;
in vec3 world_normal;
in vec3 debug_offset;

uniform vec4 model_color;
uniform vec3 light_direction = vec3(0.0, 1.0, 0.5);
uniform float ambient_strength = 0.3;
uniform float diffuse_strength = 0.7;

out vec4 fragment_color;

void main() {
    // Simple directional lighting
    vec3 norm = normalize(world_normal);
    vec3 light_dir = normalize(light_direction);
    
    float diff = max(dot(norm, light_dir), 0.0);
    vec3 ambient = ambient_strength * model_color.rgb;
    vec3 diffuse = diffuse_strength * diff * model_color.rgb;
    
    vec3 result = ambient + diffuse;
    
    // Use debug offset to color the cubes differently
    vec3 debug_color = abs(debug_offset) / 500.0; // Scale to visible range
    if (length(debug_color) > 0.01) {
        result = mix(result, debug_color, 0.7);
    }
    
    fragment_color = vec4(result, model_color.a);
}
