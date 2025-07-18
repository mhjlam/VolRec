#version 450 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 tex_coords;
layout(location = 3) in vec4 color;

// Additional color input for chessboard (at location 1, overlapping with normal)
layout(location = 1) in vec4 chessboard_color;

uniform mat4 mvp_matrix;
uniform mat4 model_matrix;
uniform float depth_bias = 0.0; // Optional depth bias for Z-fighting prevention
uniform bool use_chessboard_layout = false; // Flag to indicate chessboard mesh

out vec4 vertex_color;

void main() {
    gl_Position = mvp_matrix * model_matrix * vec4(position, 1.0);
    
    // Apply depth bias if specified
    gl_Position.z -= depth_bias * gl_Position.w;
    
    // Choose color based on mesh type
    if (use_chessboard_layout) {
        vertex_color = chessboard_color;
    }
    else {
        vertex_color = color;
    }
}
