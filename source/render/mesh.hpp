#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "vertex.hpp"
#include "vertex_array.hpp"
#include "index_buffer.hpp"
#include "vertex_buffer.hpp"


/**
 * @enum PrimitiveType
 * @brief Specifies the primitive type for mesh rendering.
 */
enum class PrimitiveType {
    POINTS = GL_POINTS,               /**< Points */
    LINES = GL_LINES,                 /**< Lines */
    LINE_STRIP = GL_LINE_STRIP,       /**< Line strip */
    LINE_LOOP = GL_LINE_LOOP,         /**< Line loop */
    TRIANGLES = GL_TRIANGLES,         /**< Triangles */
    TRIANGLE_STRIP = GL_TRIANGLE_STRIP, /**< Triangle strip */
    TRIANGLE_FAN = GL_TRIANGLE_FAN    /**< Triangle fan */
};

/**
 * @class Mesh
 * @brief Represents a renderable mesh with vertex and index data, GPU upload, and configuration.
 */
class Mesh {
public: // Statics
    /** @brief Create a default Cube mesh. */
    static std::unique_ptr<Mesh> create_cube(float size = 1.0f);

    /** @brief Create a Plane mesh. */
    static std::unique_ptr<Mesh> create_plane(float width = 1.0f, float height = 1.0f);

    /** @brief Create a Line mesh from start to end with color. */
    static std::unique_ptr<Mesh> create_line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color = glm::vec4(1.0f));

    /** @brief Create a Line mesh from a vector of points with color. */
    static std::unique_ptr<Mesh> create_lines(const std::vector<glm::vec3>& points, const glm::vec4& color = glm::vec4(1.0f));

    /** @brief Create a Grid mesh with specified size and divisions. */
    static std::unique_ptr<Mesh> create_grid(float size = 10.0f, int divisions = 10, const glm::vec4& color = glm::vec4(1.0f));

public: // Constructors
    /** @brief Construct a Mesh object. */
    Mesh();

    /** @brief Destructor. Cleans up resources. */
    ~Mesh() = default;

public: // Methods
    /**
     * @brief Set vertex data from a vector of Vertex structs.
     * @param vertices Vector of Vertex.
     */
    void set_vertices(const std::vector<Vertex>& vertices);

    /**
     * @brief Set vertex positions only.
     * @param positions Vector of positions.
     */
    void set_vertices(const std::vector<glm::vec3>& positions);

    /**
     * @brief Set vertex positions and colors.
     * @param positions Vector of positions.
     * @param colors Vector of colors.
     */
    void set_vertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec4>& colors);

    /**
     * @brief Set vertex positions and normals.
     * @param positions Vector of positions.
     * @param normals Vector of normals.
     */
    void set_vertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec3>& normals);

    /**
     * @brief Set vertex positions, normals, and texture coordinates.
     * @param positions Vector of positions.
     * @param normals Vector of normals.
     * @param tex_coords Vector of texture coordinates.
     */
    void set_vertices(const std::vector<glm::vec3>& positions, 
                      const std::vector<glm::vec3>& normals, 
                      const std::vector<glm::vec2>& tex_coords);

    /**
     * @brief Set index data for mesh.
     * @param indices Vector of indices.
     */
    void set_indices(const std::vector<unsigned int>& indices);

    /** @brief Upload vertex and index data to GPU. */
    void upload_to_gpu();

    /** @brief Update vertex data on GPU. */
    void update_vertices();

    /** @brief Update index data on GPU. */
    void update_indices();

    /** @brief Bind the mesh for rendering. */
    void bind() const;

    /** @brief Unbind the mesh after rendering. */
    void unbind() const;

    /**
     * @brief Set vertex positions and colors at attribute location 1 (for chessboard).
     * @param positions Vector of positions.
     * @param colors Vector of colors.
     */
    void set_checkers_vertices(const std::vector<glm::vec3>& positions, const std::vector<glm::vec4>& colors);

    /** @brief Force chessboard layout for mesh. */
    void set_checkers_layout();

public: // Getters
    /**
     * @brief Set the primitive type for rendering.
     * @param type PrimitiveType enum.
     */
    void set_primitive_type(PrimitiveType type) { primitive_type_ = type; }

    /** @brief Get the primitive type for rendering. */
    PrimitiveType primitive_type() const { return primitive_type_; }

    /** @brief Get the vertex array object (VAO) for this mesh. */
    const std::vector<Vertex>& vertices() const { return vertices_; }

    /** @brief Get the indices for this mesh. */
    const std::vector<unsigned int>& indices() const { return indices_; }

    /** @brief Get the number of vertices in this mesh. */
    size_t vertex_count() const { return vertices_.size(); }

    /** @brief Get the number of indices in this mesh. */
    size_t index_count() const { return indices_.size(); }

private: // Methods
    void setup_vertex_attributes();

private: // Variables
    std::vector<Vertex> vertices_;
    std::vector<unsigned int> indices_;
    PrimitiveType primitive_type_;
    
    // GPU resources
    std::unique_ptr<VertexArray> vao_;
    std::unique_ptr<VertexBuffer> vertex_buffer_;
    std::unique_ptr<IndexBuffer> index_buffer_;
    
    bool gpu_data_dirty_;
    bool is_chessboard_mesh_ = false;
};
