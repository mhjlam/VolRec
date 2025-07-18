#include "frame.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include "render/mesh.hpp"
#include "render/shader.hpp"


/* Public methods */

void Frame::initialize() {
    build_frame_geometry();
    needs_rebuild_ = false;
}

void Frame::set_position(const glm::vec3& position) {
    position_ = position;
    
    // Update transform matrix
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), position_);
    set_transform(translation);
}


/* Private methods */

void Frame::build_frame_geometry() {
    if (!needs_rebuild_ && !meshes().empty()) {
        return;
    }
    
    // Clear existing meshes
    clear_meshes();
    
    // Create three axis lines (X, Y, Z)
    glm::vec3 origin(0.0f, 0.0f, 0.0f);
    
    // X-axis (Red): columns (right)
    auto x_axis = Mesh::create_line(origin, glm::vec3(axis_length_, 0.0f, 0.0f), axis_colors_[0]);

    if (x_axis) {
        x_axis->upload_to_gpu();
        add_mesh(std::shared_ptr<Mesh>(x_axis.release()));
    }

    // Y-axis (Green): up
    auto y_axis = Mesh::create_line(origin, glm::vec3(0.0f, axis_length_, 0.0f), axis_colors_[1]);

    if (y_axis) {
        y_axis->upload_to_gpu();
        add_mesh(std::shared_ptr<Mesh>(y_axis.release()));
    }

    // Z-axis (Blue): rows (forward, -Y in OpenCV)
    auto z_axis = Mesh::create_line(origin, glm::vec3(0.0f, 0.0f, -axis_length_), axis_colors_[2]);
    
    if (z_axis) {
        z_axis->upload_to_gpu();
        add_mesh(std::shared_ptr<Mesh>(z_axis.release()));
    }
}
