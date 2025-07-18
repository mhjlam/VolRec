#include "checkers.hpp"

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "global.hpp"
#include "render/mesh.hpp"
#include "render/shader.hpp"


/* Public methods */

void Checkers::initialize() {
    clear_meshes();

    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;
    std::vector<unsigned int> indices;
    
    unsigned int idx = 0;
    float square = square_size_ + 100.0f; // Intentional +100 offset like in calibration corners

    for (int x = -1; x < cols_; ++x) {
        for (int z = -1; z < rows_; ++z) {
            bool is_black = ((x + 1) + (z + 1)) % 2 == 0;
            glm::vec4 color = is_black ? glm::vec4(0.1f, 0.1f, 0.1f, 1.0f) : glm::vec4(0.95f, 0.95f, 0.95f, 1.0f);

            float x0 = x * square;
            float x1 = (x + 1) * square;
            float z0 = -z * square;
            float z1 = -(z + 1) * square;
            float y0 = 0.0f;

            positions.push_back({x0, y0, z0});
            positions.push_back({x1, y0, z0});
            positions.push_back({x1, y0, z1});
            positions.push_back({x0, y0, z1});
            for (int i = 0; i < 4; ++i) { 
                colors.push_back(color);
            }

            // Add two triangles (winding order to face +Y after Z flip)
            indices.push_back(idx + 0);
            indices.push_back(idx + 1);
            indices.push_back(idx + 2);
            indices.push_back(idx + 2);
            indices.push_back(idx + 3);
            indices.push_back(idx + 0);

            // Add two triangles for the bottom side (winding order to face -Y)
            indices.push_back(idx + 0);
            indices.push_back(idx + 2);
            indices.push_back(idx + 1);
            indices.push_back(idx + 2);
            indices.push_back(idx + 0);
            indices.push_back(idx + 3);
            idx += 4;
        }
    }

    auto mesh = std::make_shared<Mesh>();
    mesh->set_checkers_vertices(positions, colors);
    mesh->set_indices(indices);
    mesh->set_primitive_type(PrimitiveType::TRIANGLES);
    mesh->upload_to_gpu();
    add_mesh(mesh);
}

void Checkers::force_upload_all_meshes() {
    for (const auto& mesh : meshes_) {
        if (mesh) { 
            mesh->upload_to_gpu();
        }
    }
}
