#pragma once

#include "buffer.hpp"
#include <vector>

/**
 * @class IndexBuffer
 * @brief Specialized buffer for storing index data for mesh rendering.
 *
 * Inherits from Buffer and provides convenience methods for uploading and updating index data.
 */
class IndexBuffer : public Buffer {
public: // Constructors
    /** @brief Construct an IndexBuffer object. */
    IndexBuffer() : Buffer(BufferType::INDEX_BUFFER), index_type_(GL_UNSIGNED_INT) {}

    /** @brief Destructor. Cleans up resources. */
    ~IndexBuffer() = default;

public: // Methods
    /**
     * @brief Upload a vector of unsigned int indices to the buffer.
     * @param indices Vector of indices.
     * @param usage Buffer usage hint.
     */
    void upload_indices(const std::vector<unsigned int>& indices, BufferUsage usage = BufferUsage::STATIC_DRAW);

    /**
     * @brief Upload a vector of unsigned short indices to the buffer.
     * @param indices Vector of indices.
     * @param usage Buffer usage hint.
     */
    void upload_indices(const std::vector<unsigned short>& indices, BufferUsage usage = BufferUsage::STATIC_DRAW);

    /**
     * @brief Update a vector of unsigned int indices in the buffer at a given offset.
     * @param indices Vector of indices.
     * @param offset Offset in buffer.
     */
    void update_indices(const std::vector<unsigned int>& indices, size_t offset = 0);

    /**
     * @brief Update a vector of unsigned short indices in the buffer at a given offset.
     * @param indices Vector of indices.
     * @param offset Offset in buffer.
     */
    void update_indices(const std::vector<unsigned short>& indices, size_t offset = 0);

public: // Getters
    /** @brief Get the number of indices in the buffer. */
    size_t index_count() const;

    /** @brief Get the OpenGL index type. */
    GLenum index_type() const { return index_type_; }

private: // Variables
    GLenum index_type_; /**< GL_UNSIGNED_INT, GL_UNSIGNED_SHORT, etc. */
};
