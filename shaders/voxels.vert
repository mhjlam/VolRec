#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 instance_offset;

uniform mat4 mvp_matrix;
uniform mat4 model_matrix;
uniform mat4 normal_matrix;
uniform float near_plane;
uniform float far_plane;

out vec3 world_position;
out vec3 world_normal;
out vec3 debug_offset;

void main() {
    // For instanced rendering, add the instance offset to the base vertex position
    vec3 final_position = position + instance_offset;
    
    vec4 world_pos = model_matrix * vec4(final_position, 1.0);
    world_position = world_pos.xyz;
    
    // For cubes, we can compute normals based on the vertex position relative to center
    vec3 local_normal = normalize(position);
    world_normal = normalize((normal_matrix * vec4(local_normal, 0.0)).xyz);
    
    // Pass instance offset to fragment shader for debugging
    debug_offset = instance_offset;
    
    gl_Position = mvp_matrix * world_pos;
}
