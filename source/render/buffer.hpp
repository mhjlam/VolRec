#pragma once

#include <vector>
#include <memory>

#include <GL/glew.h>


/**
 * @enum BufferType
 * @brief Specifies the type of buffer for OpenGL operations.
 */
enum class BufferType {
    VERTEX_BUFFER   = GL_ARRAY_BUFFER,          /**< Vertex buffer */
    INDEX_BUFFER    = GL_ELEMENT_ARRAY_BUFFER   /**< Index buffer */
};

/**
 * @enum BufferUsage
 * @brief Specifies the usage pattern for OpenGL buffers.
 */
enum class BufferUsage {
    STATIC_DRAW     = GL_STATIC_DRAW,           /**< Static draw usage */
    DYNAMIC_DRAW    = GL_DYNAMIC_DRAW,          /**< Dynamic draw usage */
    STREAM_DRAW     = GL_STREAM_DRAW            /**< Stream draw usage */
};

/**
 * @class Buffer
 * @brief Abstract base class for OpenGL buffer objects, supporting data upload and management.
 */
class Buffer {
public:
    /**
     * @brief Construct a Buffer object.
     * @param type Buffer type enum.
     */
    Buffer(BufferType type)
    : buffer_id_(0)
    , type_(type)
    , size_bytes_(0) {}

    /** @brief Virtual destructor. Cleans up resources. */
    virtual ~Buffer();

public:
    /** @brief Bind the buffer. */
    virtual void bind() const;

    /** @brief Unbind the buffer. */
    virtual void unbind() const;

    /**
     * @brief Upload raw data to the buffer.
     * @param data Pointer to data.
     * @param size_bytes Size in bytes.
     * @param usage Buffer usage hint.
     */
    void upload_data(const void* data, size_t size_bytes, BufferUsage usage = BufferUsage::STATIC_DRAW);

    /**
     * @brief Upload data from a vector to the buffer.
     * @tparam T Data type.
     * @param data Vector of data.
     * @param usage Buffer usage hint.
     */
    template<typename T>
    void upload_data(const std::vector<T>& data, BufferUsage usage = BufferUsage::STATIC_DRAW) {
        upload_data(data.data(), data.size(), usage);  // Pass count, not bytes
    }

    /**
     * @brief Upload data from a pointer to the buffer.
     * @tparam T Data type.
     * @param data Pointer to data.
     * @param count Number of elements.
     * @param usage Buffer usage hint.
     */
    template<typename T>
    void upload_data(const T* data, size_t count, BufferUsage usage = BufferUsage::STATIC_DRAW) {
        upload_data(static_cast<const void*>(data), count * sizeof(T), usage);
    }

    /**
     * @brief Update raw data in the buffer at a given offset.
     * @param data Pointer to data.
     * @param size_bytes Size in bytes.
     * @param offset Offset in buffer.
     */
    virtual void update_data(const void* data, size_t size_bytes, size_t offset = 0);

    /**
     * @brief Update data from a vector in the buffer at a given offset.
     * @tparam T Data type.
     * @param data Vector of data.
     * @param offset Offset in buffer.
     */
    template<typename T>
    void update_data(const std::vector<T>& data, size_t offset = 0) {
        update_data(data.data(), data.size() * sizeof(T), offset);
    }

    /**
     * @brief Update data from a pointer in the buffer at a given offset.
     * @tparam T Data type.
     * @param data Pointer to data.
     * @param count Number of elements.
     * @param offset Offset in buffer.
     */
    template<typename T>
    void update_data(const T* data, size_t count, size_t offset = 0) {
        update_data(static_cast<const void*>(data), count * sizeof(T), offset);
    }

    /** @brief Get the OpenGL buffer ID. */
    GLuint id() const { return buffer_id_; }

    /** @brief Get the buffer type. */
    BufferType type() const { return type_; }

    /** @brief Get the buffer size in bytes. */
    size_t size() const { return size_bytes_; }

    /** @brief Check if the buffer is valid. */
    bool is_valid() const { return buffer_id_ != 0; }

protected:
    /** @brief Ensure the buffer is created (lazy initialization). */
    void ensure_created() const;

protected:
    BufferType type_;
    GLuint buffer_id_;
    size_t size_bytes_;
};
