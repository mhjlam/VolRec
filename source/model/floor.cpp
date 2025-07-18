#include "floor.hpp"

#include "render/mesh.hpp"
#include "render/shader.hpp"


/* Public methods */

void Floor::initialize() {
    build_floor_geometry();
    needs_rebuild_ = false;
}


/* Private methods */

void Floor::build_floor_geometry() {
    if (!needs_rebuild_ && !meshes().empty()) {
        return;
    }
    
    // Clear existing meshes
    clear_meshes();
    
    // Create grid mesh using the factory method
    auto grid_mesh = Mesh::create_grid(size_, divisions_, floor_color_);
    if (grid_mesh) {
        grid_mesh->set_primitive_type(PrimitiveType::LINES);
        grid_mesh->upload_to_gpu();
        add_mesh(std::shared_ptr<Mesh>(grid_mesh.release()));
    }
    
    // Set the model color to match floor color
    set_color(floor_color_);
}
