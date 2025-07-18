#pragma once

#include <GL/glew.h>

/**
 * @class VertexArray
 * @brief Wrapper for OpenGL Vertex Array Objects (VAO).
 *
 * Manages VAO creation, binding, attribute setup, and lazy initialization.
 */
class VertexArray {
public: // Constructors
    /** @brief Construct a VertexArray object. */
    VertexArray() : vao_id_(0) {}

    /** @brief Destructor. Cleans up resources. */
    ~VertexArray();

public: // Methods
    /** @brief Bind the VAO. */
    void bind() const;

    /** @brief Unbind the VAO. */
    void unbind() const;

    /**
     * @brief Enable a vertex attribute.
     * @param index Attribute index.
     */
    void enable_attribute(GLuint index) const;

    /**
     * @brief Disable a vertex attribute.
     * @param index Attribute index.
     */
    void disable_attribute(GLuint index) const;

    /**
     * @brief Set a vertex attribute pointer.
     * @param index Attribute index.
     * @param size Number of components.
     * @param type Data type.
     * @param normalized Normalization flag.
     * @param stride Stride in bytes.
     * @param offset Offset pointer.
     */
    void set_attribute_pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset) const;

    /**
     * @brief Set a float attribute pointer (convenience).
     * @param index Attribute index.
     * @param size Number of components.
     * @param stride Stride in bytes.
     * @param offset Offset pointer.
     * @param normalized Normalization flag.
     */
    void set_float_attribute(GLuint index, GLint size, GLsizei stride, const void* offset, GLboolean normalized = GL_FALSE) const;

public: // Getters
    /** @brief Get the VAO ID. */
    GLuint id() const { return vao_id_; }

    /** @brief Check if the VAO is valid. */
    bool is_valid() const { return vao_id_ != 0; }

private: // Methods
    /** @brief Ensure the VAO is created (lazy initialization). */
    void ensure_created() const;

private: // Variables
    GLuint vao_id_;
};
