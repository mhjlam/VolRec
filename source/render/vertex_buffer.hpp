#pragma once

#include <vector>

#include "buffer.hpp"


/**
 * @class VertexBuffer
 * @brief Manages vertex buffer objects for storing vertex data on the GPU.
 *
 * Inherits from Buffer and provides convenience methods for uploading and updating vertex data.
 */
class VertexBuffer : public Buffer {
public: // Constructors
    /** @brief Construct a VertexBuffer object. */
    VertexBuffer() : Buffer(BufferType::VERTEX_BUFFER) {}

    /** @brief Destructor. Cleans up resources. */
    ~VertexBuffer() = default;

public: // Methods
    /**
     * @brief Upload a vector of vertices to the buffer.
     * @tparam Vertex Vertex type.
     * @param vertices Vector of vertices.
     * @param usage Buffer usage hint.
     */
    template<typename Vertex>
    void upload_vertices(const std::vector<Vertex>& vertices, BufferUsage usage = BufferUsage::STATIC_DRAW) {
        upload_data(vertices.data(), vertices.size(), usage);
    }

    /**
     * @brief Update a vector of vertices in the buffer at a given offset.
     * @tparam Vertex Vertex type.
     * @param vertices Vector of vertices.
     * @param offset Offset in buffer.
     */
    template<typename Vertex>
    void update_vertices(const std::vector<Vertex>& vertices, size_t offset = 0) {
        update_data(vertices.data(), vertices.size(), offset);
    }

    /**
     * @brief Get the number of vertices in the buffer.
     * @tparam Vertex Vertex type.
     * @return Number of vertices.
     */
    template<typename Vertex> size_t vertex_count() const {
        return size() / sizeof(Vertex);
    }
};
