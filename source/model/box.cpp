#include "box.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "render/mesh.hpp"
#include "render/shader.hpp"


/* Public methods */

void Box::initialize() {
    build_box_geometry();
    needs_rebuild_ = false;
}

void Box::set_size(const glm::vec3& size) { 
    size_ = size; 
    needs_rebuild_ = true;
}

void Box::set_position(const glm::vec3& position) {
    position_ = position;
    // Update transform matrix
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), position_);
    set_transform(translation);
}

void Box::set_wireframe(bool wireframe) { 
    wireframe_ = wireframe; 
    needs_rebuild_ = true;
}

void Box::set_box_color(const glm::vec4& color) { 
    box_color_ = color;
}


/* Private methods */

void Box::build_box_geometry() {
    if (!needs_rebuild_ && !meshes().empty()) {
        return;
    }
    
    // Clear existing meshes
    clear_meshes();
    
    if (wireframe_) {
        // Create wireframe box using lines
        std::vector<glm::vec3> line_vertices;
        
        float half_x = size_.x * 0.5f;
        float half_y = size_.y * 0.5f;
        float half_z = size_.z * 0.5f;
        
        // Bottom face edges
        line_vertices.insert(line_vertices.end(), {
            glm::vec3(-half_x, -half_y, -half_z), glm::vec3( half_x, -half_y, -half_z),
            glm::vec3( half_x, -half_y, -half_z), glm::vec3( half_x, -half_y,  half_z),
            glm::vec3( half_x, -half_y,  half_z), glm::vec3(-half_x, -half_y,  half_z),
            glm::vec3(-half_x, -half_y,  half_z), glm::vec3(-half_x, -half_y, -half_z)
        });
        
        // Top face edges
        line_vertices.insert(line_vertices.end(), {
            glm::vec3(-half_x,  half_y, -half_z), glm::vec3( half_x,  half_y, -half_z),
            glm::vec3( half_x,  half_y, -half_z), glm::vec3( half_x,  half_y,  half_z),
            glm::vec3( half_x,  half_y,  half_z), glm::vec3(-half_x,  half_y,  half_z),
            glm::vec3(-half_x,  half_y,  half_z), glm::vec3(-half_x,  half_y, -half_z)
        });
        
        // Vertical edges
        line_vertices.insert(line_vertices.end(), {
            glm::vec3(-half_x, -half_y, -half_z), glm::vec3(-half_x,  half_y, -half_z),
            glm::vec3( half_x, -half_y, -half_z), glm::vec3( half_x,  half_y, -half_z),
            glm::vec3( half_x, -half_y,  half_z), glm::vec3( half_x,  half_y,  half_z),
            glm::vec3(-half_x, -half_y,  half_z), glm::vec3(-half_x,  half_y,  half_z)
        });
        
        auto line_mesh = Mesh::create_lines(line_vertices, box_color_);
        if (line_mesh) {
            line_mesh->upload_to_gpu();
            add_mesh(std::shared_ptr<Mesh>(line_mesh.release()));
        }
    }
    else {
        // Create solid box using cube mesh
        auto cube_mesh = Mesh::create_cube(std::max({size_.x, size_.y, size_.z}));
        if (cube_mesh) {
            cube_mesh->set_primitive_type(PrimitiveType::TRIANGLES);
            cube_mesh->upload_to_gpu();
            add_mesh(std::shared_ptr<Mesh>(cube_mesh.release()));
        }
    }
    
    // Set the model color to match box color
    set_color(box_color_);
}
