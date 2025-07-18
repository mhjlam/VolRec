#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>


class Mesh;
class Volume;
class Shader;
class IndexBuffer;
class VertexArray;
class VertexBuffer;


/**
 * @enum ModelType
 * @brief Specifies the type of model for rendering.
 *
 * - MESH_BASED: Standard mesh-based geometry (triangles, lines, etc.)
 * - VOLUME_BASED: Volumetric data (voxels, 3D textures)
 */
enum class ModelType {
    MESH_BASED,
    VOLUME_BASED
};


/**
 * @class Model
 * @brief Abstract base class for all renderable models in the application.
 *
 * Provides a unified interface for mesh-based and volume-based models, supporting:
 * - Initialization and update lifecycle
 * - Visibility and transform control for scene graph operations
 * - Material/color management
 * - Access to geometry and rendering resources (meshes, shaders)
 * - Helper methods for derived classes to manage rendering data
 *
 * Derived classes must implement initialize() and may override update().
 */
class Model {
public: // Constructors
    /**
     * @brief Construct a Model object.
     * @param type The model type (mesh-based or volume-based).
     */
    Model(ModelType type) : type_(type) {}

    /** @brief Virtual destructor for Model. */
    virtual ~Model() = default;

public: // Methods
    /** @brief Initialize the model (pure virtual, must be implemented by derived classes). */
    virtual void initialize() = 0;

    /**
     * @brief Update the model (optional override for derived classes).
     * @param delta_time Time elapsed since last update.
     */
    virtual void update(float delta_time) {}

public: // Setters
    /**
     * @brief Set the model's visibility (for culling).
     * @param visible True to make visible, false to hide.
     */
    void set_visible(bool visible) { visible_ = visible; }

    /**
     * @brief Set the model's transform matrix.
     * @param transform New transform matrix.
     */
    void set_transform(const glm::mat4& transform) { transform_ = transform; }

    /**
     * @brief Set the model's color.
     * @param color New color value.
     */
    void set_color(const glm::vec4& color) { color_ = color; }

public: // Getters
    /**
     * @brief Get the model type.
     * @return ModelType enum value.
     */
    ModelType type() const { return type_; }

    /**
     * @brief Check if the model is ready to render.
     * @return True if ready, false otherwise.
     */
    virtual bool is_ready_to_render() const { return !meshes_.empty(); }

    /**
     * @brief Check if the model is visible (for culling).
     * @return True if visible, false otherwise.
     */
    bool is_visible() const { return visible_; }

    /**
     * @brief Get the model's transform matrix.
     * @return Reference to the transform matrix.
     */
    const glm::mat4& transform() const { return transform_; }

    /**
     * @brief Get the model's color.
     * @return Reference to the color vector.
     */
    const glm::vec4& color() const { return color_; }

    /**
     * @brief Get the model's meshes.
     * @return Reference to vector of mesh pointers.
     */
    const std::vector<std::shared_ptr<Mesh>>& meshes() const { return meshes_; }

    /**
     * @brief Get the model's shaders.
     * @return Reference to vector of shader pointers.
     */
    const std::vector<std::shared_ptr<Shader>>& shaders() const { return shaders_; }

    /**
     * @brief Get the primary shader for the model.
     * @return Shared pointer to the primary shader.
     */
    std::shared_ptr<Shader> primary_shader() const { return primary_shader_; }

protected: // Methods
    /**
     * @brief Add a mesh to the model.
     * @param mesh Shared pointer to the mesh.
     */
    void add_mesh(std::shared_ptr<Mesh> mesh);

    /**
     * @brief Add a shader to the model.
     * @param shader Shared pointer to the shader.
     */
    void add_shader(std::shared_ptr<Shader> shader);

    /**
     * @brief Set the primary shader for the model.
     * @param shader Shared pointer to the shader.
     */
    void set_primary_shader(std::shared_ptr<Shader> shader);

    /** @brief Remove all meshes from the model. */
    void clear_meshes() { meshes_.clear(); }
    
protected: // Variables
    ModelType type_;
    
    // Geometry data
    std::vector<std::shared_ptr<Mesh>> meshes_;
    
    // Rendering resources
    std::vector<std::shared_ptr<Shader>> shaders_;
    std::shared_ptr<Shader> primary_shader_;
    
    // Transform and state
    bool visible_{true};
    glm::mat4 transform_{1.0f};
    glm::vec4 color_{1.0f, 1.0f, 1.0f, 1.0f};
};
