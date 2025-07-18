#include "mesh.hpp"

#include <cmath>


std::unique_ptr<Mesh> Mesh::create_cube(float size) {
    auto mesh = std::make_unique<Mesh>();
    float half_size = size * 0.5f;
    
    std::vector<Vertex> vertices = {
        // Front face
        {{-half_size, -half_size,  half_size}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{ half_size, -half_size,  half_size}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{ half_size,  half_size,  half_size}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-half_size,  half_size,  half_size}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        
        // Back face
        {{ half_size, -half_size, -half_size}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{-half_size, -half_size, -half_size}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
        {{-half_size,  half_size, -half_size}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{ half_size,  half_size, -half_size}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        
        // Left face
        {{-half_size, -half_size, -half_size}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{-half_size, -half_size,  half_size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{-half_size,  half_size,  half_size}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{-half_size,  half_size, -half_size}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        
        // Right face
        {{ half_size, -half_size,  half_size}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{ half_size, -half_size, -half_size}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
        {{ half_size,  half_size, -half_size}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{ half_size,  half_size,  half_size}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        
        // Top face
        {{-half_size,  half_size,  half_size}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ half_size,  half_size,  half_size}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ half_size,  half_size, -half_size}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-half_size,  half_size, -half_size}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        
        // Bottom face
        {{-half_size, -half_size, -half_size}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ half_size, -half_size, -half_size}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ half_size, -half_size,  half_size}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-half_size, -half_size,  half_size}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {
        0,  1,  2,   2,  3,  0,   // Front
        4,  5,  6,   6,  7,  4,   // Back
        8,  9, 10,  10, 11,  8,   // Left
        12, 13, 14,  14, 15, 12,   // Right
        16, 17, 18,  18, 19, 16,   // Top
        20, 21, 22,  22, 23, 20    // Bottom
    };
    
    mesh->set_vertices(vertices);
    mesh->set_indices(indices);
    mesh->set_primitive_type(PrimitiveType::TRIANGLES);
    
    return mesh;
}

std::unique_ptr<Mesh> Mesh::create_plane(float width, float height) {
    auto mesh = std::make_unique<Mesh>();
    
    float half_width = width * 0.5f;
    float half_height = height * 0.5f;
    
    std::vector<Vertex> vertices = {
        {{-half_width, 0.0f, -half_height}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{ half_width, 0.0f, -half_height}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
        {{ half_width, 0.0f,  half_height}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-half_width, 0.0f,  half_height}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
    };
    
    std::vector<unsigned int> indices = {0, 1, 2, 2, 3, 0};
    
    mesh->set_vertices(vertices);
    mesh->set_indices(indices);
    mesh->set_primitive_type(PrimitiveType::TRIANGLES);
    
    return mesh;
}

std::unique_ptr<Mesh> Mesh::create_line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color) {
    auto mesh = std::make_unique<Mesh>();
    
    std::vector<Vertex> vertices = {
        {start, glm::vec3(0.0f), glm::vec2(0.0f), color},
        {end, glm::vec3(0.0f), glm::vec2(0.0f), color}
    };
    
    mesh->set_vertices(vertices);
    mesh->set_primitive_type(PrimitiveType::LINES);
    
    return mesh;
}

std::unique_ptr<Mesh> Mesh::create_lines(const std::vector<glm::vec3>& points, const glm::vec4& color) {
    auto mesh = std::make_unique<Mesh>();
    
    std::vector<Vertex> vertices;
    vertices.reserve(points.size());
    
    for (const auto& point : points) {
        vertices.emplace_back(point, glm::vec3(0.0f), glm::vec2(0.0f), color);
    }
    
    mesh->set_vertices(vertices);
    mesh->set_primitive_type(PrimitiveType::LINES);
    
    return mesh;
}

std::unique_ptr<Mesh> Mesh::create_grid(float size, int divisions, const glm::vec4& color) {
    auto mesh = std::make_unique<Mesh>();
    
    std::vector<glm::vec3> points;
    float step = size / divisions;
    float half_size = size * 0.5f;
    
    // Horizontal lines
    for (int i = 0; i <= divisions; ++i) {
        float z = -half_size + i * step;
        points.emplace_back(-half_size, 0.0f, z);
        points.emplace_back(half_size, 0.0f, z);
    }
    
    // Vertical lines
    for (int i = 0; i <= divisions; ++i) {
        float x = -half_size + i * step;
        points.emplace_back(x, 0.0f, -half_size);
        points.emplace_back(x, 0.0f, half_size);
    }
    
    std::vector<Vertex> vertices;
    vertices.reserve(points.size());
    
    for (const auto& point : points) {
        vertices.emplace_back(point, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f), color);
    }
    
    mesh->set_vertices(vertices);
    mesh->set_primitive_type(PrimitiveType::LINES);
    
    return mesh;
}


Mesh::Mesh() : primitive_type_(PrimitiveType::TRIANGLES), gpu_data_dirty_(true) {
    vao_ = std::make_unique<VertexArray>();
    vertex_buffer_ = std::make_unique<VertexBuffer>();
    index_buffer_ = std::make_unique<IndexBuffer>();
}


void Mesh::set_vertices(const std::vector<Vertex>& vertices) {
    vertices_ = vertices;
    gpu_data_dirty_ = true;
}

void Mesh::set_vertices(const std::vector<glm::vec3>& positions) {
    vertices_.clear();
    vertices_.reserve(positions.size());
    
    for (const auto& pos : positions) {
        vertices_.emplace_back(pos);
    }

    gpu_data_dirty_ = true;
}

void Mesh::set_vertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec4>& colors) {
    vertices_.clear();
    vertices_.reserve(positions.size());

    for (size_t i = 0; i < positions.size(); ++i) {
        Vertex vertex(positions[i]);
        if (i < colors.size()) {
            vertex.color = colors[i];
        }
        vertices_.push_back(vertex);
    }

    gpu_data_dirty_ = true;
}

void Mesh::set_vertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals) {
    vertices_.clear();
    vertices_.reserve(positions.size());

    for (size_t i = 0; i < positions.size(); ++i) {
        Vertex vertex(positions[i]);
        if (i < normals.size()) {
            vertex.normal = normals[i];
        }
        vertices_.push_back(vertex);
    }

    gpu_data_dirty_ = true;
}

void Mesh::set_vertices(const std::vector<glm::vec3>& positions, 
                        const std::vector<glm::vec3>& normals, 
                        const std::vector<glm::vec2>& tex_coords) {
    vertices_.clear();
    vertices_.reserve(positions.size());

    for (size_t i = 0; i < positions.size(); ++i) {
        Vertex vertex(positions[i]);
        if (i < normals.size()) {
            vertex.normal = normals[i];
        }

        if (i < tex_coords.size()) {
            vertex.tex_coords = tex_coords[i];
        }
        vertices_.push_back(vertex);
    }

    gpu_data_dirty_ = true;
}

void Mesh::set_indices(const std::vector<unsigned int>& indices) {
    indices_ = indices;
    gpu_data_dirty_ = true;
}

void Mesh::upload_to_gpu() {
    if (!gpu_data_dirty_) {
        return;
    }

    vao_->bind();

    // Upload vertex data
    vertex_buffer_->upload_vertices(vertices_);

    // Upload index data if we have indices
    if (!indices_.empty()) {
        index_buffer_->upload_indices(indices_);
    }

    if (is_chessboard_mesh_) {
        set_checkers_layout();
    }
    else {
        setup_vertex_attributes();
    }
    gpu_data_dirty_ = false;

    vao_->unbind();
}

void Mesh::update_vertices() {
    if (vertices_.empty()) { return; }
    vertex_buffer_->update_vertices(vertices_);
}

void Mesh::update_indices() {
    if (indices_.empty()) { return; }
    index_buffer_->update_indices(indices_);
}

void Mesh::bind() const {
    if (gpu_data_dirty_) {
        const_cast<Mesh*>(this)->upload_to_gpu();
    }
    vao_->bind();
}

void Mesh::unbind() const {
    vao_->unbind();
}

void Mesh::set_checkers_vertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec4>& colors) {
    is_chessboard_mesh_ = true;

    vertices_.clear();
    vertices_.reserve(positions.size());

    for (size_t i = 0; i < positions.size(); ++i) {
        Vertex vertex(positions[i]);
        if (i < colors.size()) {
            vertex.color = colors[i];
        }
        vertices_.push_back(vertex);
    }

    gpu_data_dirty_ = true;

    // Custom VAO setup: bind color to location 1
    vao_->bind();

    // Position at 0
    vao_->set_float_attribute(0, 3, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, position)));

    // Color at 1
    vao_->set_float_attribute(1, 4, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));
    vao_->unbind();
}

void Mesh::set_checkers_layout() {
    vao_->bind();
    vao_->set_float_attribute(0, 3, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, position)));
    vao_->set_float_attribute(1, 4, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));
    vao_->unbind();
}


void Mesh::setup_vertex_attributes() {
    // Position attribute (location 0)
    vao_->set_float_attribute(0, 3, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, position)));
    
    // Normal attribute (location 1)
    vao_->set_float_attribute(1, 3, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, normal)));
    
    // Texture coordinate attribute (location 2)
    vao_->set_float_attribute(2, 2, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, tex_coords)));
    
    // Color attribute (location 3)
    vao_->set_float_attribute(3, 4, sizeof(Vertex), reinterpret_cast<const void*>(offsetof(Vertex, color)));
}
