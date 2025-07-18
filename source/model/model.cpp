#include "model.hpp"

#include <ranges>

#include "render/mesh.hpp"
#include "render/shader.hpp"


/* Protected methods */

void Model::add_mesh(std::shared_ptr<Mesh> mesh) {
    if (mesh) {
        meshes_.push_back(mesh);
    }
}

void Model::add_shader(std::shared_ptr<Shader> shader) {
    if (shader) {
        shaders_.push_back(shader);
        if (!primary_shader_) {
            primary_shader_ = shader;
        }
    }
}

void Model::set_primary_shader(std::shared_ptr<Shader> shader) {
    primary_shader_ = shader;
    
    // Also add to shaders list if not already there
    if (std::ranges::find(shaders_, shader) == shaders_.end()) {
        shaders_.push_back(shader);
    }
}
