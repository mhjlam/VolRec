#include "volume.hpp"

#include <cmath>
#include <atomic>
#include <chrono>
#include <ranges>
#include <thread>
#include <iostream>
#include <algorithm>

#include <omp.h>
#include <GL/glew.h>

#include "render/vertex.hpp"


/* Static functions */

std::unique_ptr<Volume> Volume::create_sphere(int radius, const glm::vec4 &color) {
    int size = radius * 2 + 1;
    auto volume = std::make_unique<Volume>(size, size, size);
    glm::vec3 center(0.0f);
    volume->fill_sphere(center, static_cast<float>(radius), color);
    return volume;
}

std::unique_ptr<Volume> Volume::create_cube(int size, const glm::vec4 &color) {
    auto volume = std::make_unique<Volume>(size, size, size);
    glm::vec3 min_pos(-size * 0.5f);
    glm::vec3 max_pos(size * 0.5f);
    volume->fill_box(min_pos, max_pos, color);
    return volume;
}

std::unique_ptr<Volume> Volume::create_plane(int width, int height, const glm::vec4 &color) {
    auto volume = std::make_unique<Volume>(width, height, 1);
    glm::vec3 min_pos(-width * 0.5f, -height * 0.5f, -0.5f);
    glm::vec3 max_pos(width * 0.5f, height * 0.5f, 0.5f);
    volume->fill_box(min_pos, max_pos, color);
    return volume;
}


/* Constructors */

Volume::Volume(int width, int height, int depth, float voxel_size) 
: Model(ModelType::VOLUME_BASED)
, gpu_data_dirty_(true)
, volume_texture_dirty_(true)
, width_(width)
, height_(height)
, depth_(depth)
, voxel_size_(voxel_size)
, rendered_voxel_count_(0)
, render_mode_(VolumeRenderMode::VOXEL_CUBES)
{
    voxels_.resize(width_ * height_ * depth_);
    for (int z = 0; z < depth_; ++z) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                size_t index = z * width_ * height_ + y * width_ + x;
                voxels_[index].position = voxel_to_world(x, y, z);
            }
        }
    }
}


/* Public methods */

void Volume::initialize() {
    // Create OpenGL resources now that context is ready
    vao_ = std::make_unique<VertexArray>();
    vertex_buffer_ = std::make_unique<VertexBuffer>();
    normal_buffer_ = std::make_unique<VertexBuffer>();
    instance_buffer_ = std::make_unique<VertexBuffer>();
    index_buffer_ = std::make_unique<IndexBuffer>();

    // Don't upload to GPU yet - defer until first render call
    gpu_data_dirty_ = true;
}

void Volume::set_voxel(int x, int y, int z, const Voxel &voxel) {
    if (!is_valid_coordinate(x, y, z)) {
        return;
    }

    size_t index = get_index(x, y, z);
    voxels_[index] = voxel;
    voxels_[index].position = voxel_to_world(x, y, z); // Ensure position is correct
    gpu_data_dirty_ = true;
    volume_texture_dirty_ = true;
}

void Volume::set_voxel_active(int x, int y, int z, bool active) {
    if (!is_valid_coordinate(x, y, z)) {
        return;
    }

    size_t index = get_index(x, y, z);
    voxels_[index].active = active;

    gpu_data_dirty_ = true;
    volume_texture_dirty_ = true;
}

void Volume::set_voxel_color(int x, int y, int z, const glm::vec4 &color) {
    if (!is_valid_coordinate(x, y, z)) {
        return;
    }

    size_t index = get_index(x, y, z);
    voxels_[index].color = color;
    gpu_data_dirty_ = true;
    volume_texture_dirty_ = true;
}

void Volume::set_voxel_density(int x, int y, int z, float density) {
    if (!is_valid_coordinate(x, y, z)) {
        return;
    }

    size_t index = get_index(x, y, z);
    voxels_[index].density = density;
    volume_texture_dirty_ = true;
}

void Volume::clear_all() {
    for (auto &voxel : voxels_) {
        voxel.active = false;
        voxel.color = glm::vec4(1.0f);
        voxel.density = 0.0f;
    }

    gpu_data_dirty_ = true;
    volume_texture_dirty_ = true;
}

void Volume::activate_all() {
    for (auto &voxel : voxels_) {
        voxel.active = true;
    }
    gpu_data_dirty_ = true;
}

void Volume::deactivate_all() {
    for (auto &voxel : voxels_) {
        voxel.active = false;
    }
    gpu_data_dirty_ = true;
}

void Volume::set_all_color(const glm::vec4 &color) {
    for (auto &voxel : voxels_) {
        voxel.color = color;
    }
    gpu_data_dirty_ = true;
    volume_texture_dirty_ = true;
}

void Volume::fill_sphere(const glm::vec3 &center, float radius, const glm::vec4 &color) {
    float radius_squared = radius * radius;

    for (int z = 0; z < depth_; ++z) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                glm::vec3 voxel_pos = voxel_to_world(x, y, z);
                glm::vec3 diff = voxel_pos - center;
                float distance_squared = glm::dot(diff, diff);

                if (distance_squared <= radius_squared) {
                    size_t index = get_index(x, y, z);
                    voxels_[index].active = true;
                    voxels_[index].color = color;
                    voxels_[index].density = 1.0f - (std::sqrt(distance_squared) / radius);
                }
            }
        }
    }

    gpu_data_dirty_ = true;
    volume_texture_dirty_ = true;
}

void Volume::fill_box(const glm::vec3 &min_pos, const glm::vec3 &max_pos, const glm::vec4 &color) {
    for (int z = 0; z < depth_; ++z) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                glm::vec3 voxel_pos = voxel_to_world(x, y, z);

                if (voxel_pos.x >= min_pos.x && voxel_pos.x <= max_pos.x 
                ||  voxel_pos.y >= min_pos.y && voxel_pos.y <= max_pos.y
                ||  voxel_pos.z >= min_pos.z && voxel_pos.z <= max_pos.z) {
                    size_t index = get_index(x, y, z);
                    voxels_[index].active = true;
                    voxels_[index].color = color;
                    voxels_[index].density = 1.0f;
                }
            }
        }
    }

    gpu_data_dirty_ = true;
    volume_texture_dirty_ = true;
}

void Volume::upload_to_gpu() {
    if (!gpu_data_dirty_) {
        return;
    }

    // Validate OpenGL context before any operations
    if (!glewIsSupported("GL_VERSION_3_0")) {
        std::cerr << "Error: OpenGL context not available in Volume::upload_to_gpu()" << std::endl;
        return;
    }

    // Check if we have any active voxels to upload
    size_t active_count = active_voxel_count();
    if (active_count == 0) {
        std::cerr << "Warning: No active voxels to upload to GPU" << std::endl;
        gpu_data_dirty_ = false;
        return;
    }

    switch (render_mode_) {
        case VolumeRenderMode::POINT_CLOUD:
            setup_point_rendering();
            break;

        case VolumeRenderMode::VOXEL_CUBES:
            setup_instanced_rendering();
            break;
    }

    gpu_data_dirty_ = false;
}

void Volume::update_active_voxels() {
    rendered_voxel_count_ = std::count_if(voxels_.begin(), voxels_.end(), [](const Voxel& v) { return v.active; });
    gpu_data_dirty_ = true;
}

void Volume::set_render_mode(VolumeRenderMode mode) {
    if (render_mode_ != mode) {
        render_mode_ = mode;
        gpu_data_dirty_ = true; // Mark for re-upload with new rendering setup
    }
}

void Volume::bind() const {
    // Check if OpenGL context is available
    if (!glewIsSupported("GL_VERSION_3_0")) {
        std::cerr << "Error: OpenGL context not available in Volume::bind()" << std::endl;
        return;
    }

    // Ensure GPU data is uploaded before binding
    if (gpu_data_dirty_) {
        const_cast<Volume *>(this)->upload_to_gpu();
    }

    if (vao_) {
        vao_->bind();
    }
}

void Volume::unbind() const {
    if (vao_) {
        vao_->unbind();
    }
}

void Volume::create_volume_texture() {
    volume_texture_ = std::make_shared<Texture>();
    update_volume_texture();
}

void Volume::update_volume_texture() {
    if (!volume_texture_ || !volume_texture_dirty_) {
        return;
    }

    // Create 3D texture data from voxel density and color information
    std::vector<float> texture_data(width_ * height_ * depth_ * 4); // RGBA

    for (int z = 0; z < depth_; ++z) {
        for (int y = 0; y < height_; ++y) {
            for (int x = 0; x < width_; ++x) {
                size_t voxel_index = get_index(x, y, z);
                size_t texture_index = (z * width_ * height_ + y * width_ + x) * 4;

                const Voxel &voxel = voxels_[voxel_index];
                if (voxel.active) {
                    texture_data[texture_index + 0] = voxel.color.r;
                    texture_data[texture_index + 1] = voxel.color.g;
                    texture_data[texture_index + 2] = voxel.color.b;
                    texture_data[texture_index + 3] = voxel.density;
                }
                else {
                    texture_data[texture_index + 0] = 0.0f;
                    texture_data[texture_index + 1] = 0.0f;
                    texture_data[texture_index + 2] = 0.0f;
                    texture_data[texture_index + 3] = 0.0f;
                }
            }
        }
    }

    // Upload to 3D texture (would need Texture class support for 3D textures)
    volume_texture_dirty_ = false;
}

glm::vec3 Volume::voxel_to_world(int x, int y, int z) const {
    // Convert from OpenCV grid indices (x, y, z) to OpenGL world coordinates
    // OpenGL X = OpenCV X
    // OpenGL Y = OpenCV Z (OpenCV Z+ becomes OpenGL Y+)
    // OpenGL Z = -OpenCV Y (OpenCV Y+ becomes OpenGL Z-)
    float world_x = (x - width_ * 0.5f) * voxel_size_;
    float world_y = z * voxel_size_ + voxel_size_ * 0.5f; // OpenCV Z becomes OpenGL Y (up), raised by half voxel to sit on floor
    float world_z = -(y - height_ * 0.5f) * voxel_size_; // OpenCV Y becomes -OpenGL Z
    return glm::vec3(world_x, world_y, world_z);
}

glm::ivec3 Volume::world_to_voxel(const glm::vec3 &world_pos) const {
    // Undo the Y offset (half voxel size) that was added in voxel_to_world
    float adjusted_y = world_pos.y - voxel_size_ * 0.5f;
    
    // Convert from OpenGL world coordinates back to OpenCV grid indices
    // Reverse the coordinate transformation from voxel_to_world
    float world_x = world_pos.x;
    float world_y = adjusted_y;  // This will become OpenCV Z
    float world_z = world_pos.z;
    
    // Convert to voxel indices
    int x = static_cast<int>(std::round(world_x / voxel_size_ + width_ * 0.5f));
    int z = static_cast<int>(std::round(world_y / voxel_size_)); // OpenGL Y becomes OpenCV Z
    int y = static_cast<int>(std::round(-world_z / voxel_size_ + height_ * 0.5f)); // OpenGL Z becomes OpenCV Y
    return glm::ivec3(x, y, z);
}


/* Getters */

bool Volume::is_ready_to_render() const {
    return active_voxel_count() > 0 && vao_ != nullptr;
}

Voxel Volume::get_voxel(int x, int y, int z) const {
    if (!is_valid_coordinate(x, y, z)) {
        return Voxel();
    }

    size_t index = get_index(x, y, z);
    return voxels_[index];
}

bool Volume::is_voxel_active(int x, int y, int z) const {
    if (!is_valid_coordinate(x, y, z)) {
        return false;
    }

    size_t index = get_index(x, y, z);
    return voxels_[index].active;
}

size_t Volume::active_voxel_count() const {
    return std::ranges::count_if(voxels_, [](const Voxel &v) {
        return v.active;
    });
}


/* Private methods */

size_t Volume::get_index(int x, int y, int z) const {
    return z * width_ * height_ + y * width_ + x;
}

bool Volume::is_valid_coordinate(int x, int y, int z) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_ && z >= 0 && z < depth_;
}

void Volume::setup_point_rendering() {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;

    generate_active_voxel_data(positions, colors);

    if (positions.empty()) {
        return;
    }

    // Safety checks for OpenGL resources
    if (!vao_ || !vertex_buffer_ || !normal_buffer_) {
        std::cerr << "Error: OpenGL resources not initialized in Volume::setup_point_rendering()" << std::endl;
        return;
    }

    // Validate vector data integrity
    if (positions.data() == nullptr || colors.data() == nullptr) {
        std::cerr << "Error: Vector data pointer is null!" << std::endl;
        return;
    }

    // Test reading first and last elements to validate memory
    try {
        glm::vec3 first = positions[0];
        glm::vec3 last = positions[positions.size() - 1];
        glm::vec4 first_color = colors[0];
        glm::vec4 last_color = colors[colors.size() - 1];
    }
    catch (...) {
        std::cerr << "Error: Cannot read vector data!" << std::endl;
        return;
    }

    // Upload position data
    vertex_buffer_->bind();
    vertex_buffer_->upload_data(positions, BufferUsage::STATIC_DRAW);
    vertex_buffer_->unbind();

    // Upload color data (reusing normal_buffer_ for colors)
    normal_buffer_->bind();
    normal_buffer_->upload_data(colors, BufferUsage::STATIC_DRAW);
    normal_buffer_->unbind();

    // Now bind VAO and set up vertex attributes
    vao_->bind();
    
    // Set up vertex attributes for positions
    vertex_buffer_->bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    vertex_buffer_->unbind();

    // Set up vertex attributes for colors
    normal_buffer_->bind();
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
    normal_buffer_->unbind();

    vao_->unbind();
}

void Volume::setup_instanced_rendering() {
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;

    generate_active_voxel_data(positions, colors);

    if (positions.empty()) { 
        return;
    }

    // Create base cube geometry (8 vertices, will be instanced)
    float half_size = voxel_size_ * 0.5f;
    std::vector<glm::vec3> cube_vertices = {
        {-half_size, -half_size, -half_size}, // 0
        {half_size, -half_size, -half_size},  // 1
        {half_size, half_size, -half_size},   // 2
        {-half_size, half_size, -half_size},  // 3
        {-half_size, -half_size, half_size},  // 4
        {half_size, -half_size, half_size},   // 5
        {half_size, half_size, half_size},    // 6
        {-half_size, half_size, half_size}    // 7
    };

    // Cube indices (36 indices for 12 triangles)
    std::vector<unsigned int> cube_indices = {
        0, 1, 2, 2, 3, 0,   // Front face
        4, 7, 6, 6, 5, 4,   // Back face
        0, 3, 7, 7, 4, 0,   // Left face
        1, 5, 6, 6, 2, 1,   // Right face
        3, 2, 6, 6, 7, 3,   // Top face
        0, 4, 5, 5, 1, 0    // Bottom face
    };

    // Upload base cube geometry
    vertex_buffer_->bind();
    vertex_buffer_->upload_data(cube_vertices, BufferUsage::STATIC_DRAW);
    vertex_buffer_->unbind();

    // Upload base cube indices
    index_buffer_->bind();
    index_buffer_->upload_data(cube_indices, BufferUsage::STATIC_DRAW);
    index_buffer_->unbind();

    vao_->bind();

    // Set up base cube vertex attributes (location 0)
    vertex_buffer_->bind();
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    
    // Bind index buffer to VAO
    index_buffer_->bind();

    // Upload instance data (positions) and set up instanced vertex attributes (location 1)
    instance_buffer_->bind();
    instance_buffer_->upload_data(positions, BufferUsage::STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glVertexAttribDivisor(1, 1); // This makes attribute 1 advance once per instance

    // Clean up - unbind buffers but keep VAO bound state
    vertex_buffer_->unbind();
    instance_buffer_->unbind();
    vao_->unbind();

    // Update rendered count (number of instances, not vertices)
    rendered_voxel_count_ = positions.size();
}

void Volume::generate_active_voxel_data(std::vector<glm::vec3> &positions, std::vector<glm::vec4> &colors) const {
    positions.clear();
    colors.clear();
    
    // Use only active voxels for rendering
    for (const auto &voxel : voxels_) {
        if (voxel.active) {
            positions.push_back(voxel.position);
            colors.push_back(voxel.color);
        }
    }

    // Update the rendered voxel count (cast away const for this internal tracking)
    const_cast<Volume *>(this)->rendered_voxel_count_ = positions.size();
}
