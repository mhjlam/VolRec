#pragma once

#include <memory>
#include <vector>
#include <glm/glm.hpp>

#include "model.hpp"
#include "global.hpp"

#include "render/voxel.hpp"
#include "render/texture.hpp"
#include "render/vertex_array.hpp"
#include "render/index_buffer.hpp"
#include "render/vertex_buffer.hpp"


/**
 * @enum VolumeRenderMode
 * @brief Specifies the rendering mode for volumetric data.
 *
 * - POINT_CLOUD: Render voxels as points (point cloud visualization)
 * - VOXEL_CUBES: Render voxels as cubes (solid voxel visualization)
 */
enum class VolumeRenderMode {
    POINT_CLOUD,
    VOXEL_CUBES
};


/**
 * @class Volume
 * @brief Represents volumetric data for rendering, manipulation, and GPU upload.
 *
 * Supports voxel-based storage, bulk operations, rendering modes, and utility methods for common volume patterns.
 * Inherits from Model and implements required rendering interface.
 */
class Volume : public Model 
{
public: // Statics
    /**
     * @brief Create a sphere volume pattern.
     * @param radius Radius of the sphere.
     * @param color Color value.
     * @return Unique pointer to the created Volume.
     */
    static std::unique_ptr<Volume> create_sphere(int radius, const glm::vec4& color = glm::vec4(1.0f));

    /**
     * @brief Create a cube volume pattern.
     * @param size Size of the cube.
     * @param color Color value.
     * @return Unique pointer to the created Volume.
     */
    static std::unique_ptr<Volume> create_cube(int size, const glm::vec4& color = glm::vec4(1.0f));

    /**
     * @brief Create a plane volume pattern.
     * @param width Width of the plane.
     * @param height Height of the plane.
     * @param color Color value.
     * @return Unique pointer to the created Volume.
     */
    static std::unique_ptr<Volume> create_plane(int width, int height, const glm::vec4& color = glm::vec4(1.0f));

public: // Constructors
    /**
     * @brief Construct a new Volume object.
     * @param width Volume width in voxels.
     * @param height Volume height in voxels.
     * @param depth Volume depth in voxels.
     * @param voxel_size Size of each voxel.
     */
    Volume(int width, int height, int depth, float voxel_size = VOLUME_VOXEL_SIZE);

    /** @brief Destructor. Cleans up resources. */
    ~Volume() = default;

public: // Methods
    /** @brief Initialize the volume model. */
    void initialize() override;

    /**
     * @brief Set a voxel's data.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @param voxel Voxel data.
     */
    void set_voxel(int x, int y, int z, const Voxel& voxel);

    /**
     * @brief Set a voxel's active state.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @param active True to activate, false to deactivate.
     */
    void set_voxel_active(int x, int y, int z, bool active);

    /**
     * @brief Set a voxel's color.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @param color Color value.
     */
    void set_voxel_color(int x, int y, int z, const glm::vec4& color);

    /**
     * @brief Set a voxel's density.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @param density Density value.
     */
    void set_voxel_density(int x, int y, int z, float density);

    /** @brief Clear all voxels. */
    void clear_all();

    /** @brief Activate all voxels. */
    void activate_all();

    /** @brief Deactivate all voxels. */
    void deactivate_all();

    /**
     * @brief Set the color for all voxels.
     * @param color Color value.
     */
    void set_all_color(const glm::vec4& color);

    /**
     * @brief Fill a sphere in the volume.
     * @param center Center of the sphere.
     * @param radius Radius of the sphere.
     * @param color Color value.
     */
    void fill_sphere(const glm::vec3& center, float radius, const glm::vec4& color);

    /**
     * @brief Fill a box in the volume.
     * @param min_pos Minimum position.
     * @param max_pos Maximum position.
     * @param color Color value.
     */
    void fill_box(const glm::vec3& min_pos, const glm::vec3& max_pos, const glm::vec4& color);

    /** @brief Upload voxel data to the GPU. */
    void upload_to_gpu();

    /** @brief Update only the visible (active) voxels. */
    void update_active_voxels();

    /**
     * @brief Set the rendering mode for the volume.
     * @param mode Rendering mode.
     */
    void set_render_mode(VolumeRenderMode mode);

    /** @brief Bind the volume for rendering. */
    void bind() const;

    /** @brief Unbind the volume after rendering. */
    void unbind() const;

    /** @brief Create the 3D texture for volume rendering. */
    void create_volume_texture();

    /** @brief Update the 3D texture for volume rendering. */
    void update_volume_texture();

    /**
     * @brief Convert voxel coordinates to world space.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @return World space position.
     */
    glm::vec3 voxel_to_world(int x, int y, int z) const;

    /**
     * @brief Convert world space position to voxel coordinates.
     * @param world_pos World space position.
     * @return Voxel coordinates.
     */
    glm::ivec3 world_to_voxel(const glm::vec3& world_pos) const;

public: // Getters
    /** @brief Check if the volume is ready to render. */
    bool is_ready_to_render() const override;

    /**
     * @brief Get a voxel's data.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @return Voxel data.
     */
    Voxel get_voxel(int x, int y, int z) const;

    /**
     * @brief Check if a voxel is active.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @return True if active, false otherwise.
     */
    bool is_voxel_active(int x, int y, int z) const;

    /** @brief Get the volume texture. */
    std::shared_ptr<Texture> volume_texture() const { return volume_texture_; }

    /** @brief Get the volume width in voxels. */
    int width() const { return width_; }

    /** @brief Get the volume height in voxels. */
    int height() const { return height_; }

    /** @brief Get the volume depth in voxels. */
    int depth() const { return depth_; }

    /** @brief Get the size of each voxel. */
    float voxel_size() const { return voxel_size_; }

    /** @brief Get the grid size in world units. */
    glm::vec3 grid_size() const { return glm::vec3(width_ * voxel_size_, height_ * voxel_size_, depth_ * voxel_size_); }

    /** @brief Get the total number of voxels. */
    size_t voxel_count() const { return voxels_.size(); }

    /** @brief Get the number of active voxels. */
    size_t active_voxel_count() const;
    
    /** @brief Get the number of rendered voxels. */
    size_t rendered_voxel_count() const { return rendered_voxel_count_; }

    /** @brief Get the current volume render mode. */
    VolumeRenderMode render_mode() const { return render_mode_; }

private: // Methods
    /** @brief Get the index in the voxel array for given coordinates. */
    size_t get_index(int x, int y, int z) const;

    /**
     * @brief Check if the given coordinates are valid.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param z Z coordinate.
     * @return True if valid, false otherwise.
     */
    bool is_valid_coordinate(int x, int y, int z) const;

    /** @brief Setup point cloud rendering for the volume. */
    void setup_point_rendering();

    /** @brief Setup instanced rendering for the volume. */
    void setup_instanced_rendering();

    /**
     * @brief Generate data for active voxels (positions and colors).
     * @param positions Output vector of positions.
     * @param colors Output vector of colors.
     */
    void generate_active_voxel_data(std::vector<glm::vec3>& positions, std::vector<glm::vec4>& colors) const;

private: // Variables
    bool gpu_data_dirty_;
    bool volume_texture_dirty_;

    int width_, height_, depth_;
    float voxel_size_;
    size_t rendered_voxel_count_; // Track how many voxels are actually rendered

    std::vector<Voxel> voxels_;
    VolumeRenderMode render_mode_;

    std::unique_ptr<VertexArray> vao_;
    std::unique_ptr<VertexBuffer> vertex_buffer_;
    std::unique_ptr<VertexBuffer> normal_buffer_;
    std::unique_ptr<VertexBuffer> instance_buffer_;
    std::unique_ptr<IndexBuffer> index_buffer_;
    std::shared_ptr<Texture> volume_texture_;
};
