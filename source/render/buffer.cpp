#include "buffer.hpp"

#include <iostream>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "global.hpp"
#include "vertex.hpp"


Buffer::~Buffer() {
    if (buffer_id_ != 0) {
        glDeleteBuffers(1, &buffer_id_);
    }
}


void Buffer::bind() const {
    ensure_created();
    if (buffer_id_ != 0) {
        glBindBuffer(static_cast<GLenum>(type_), buffer_id_);
        std::string err = check_gl_error("glBindBuffer");
        if (!err.empty()) {
            std::cerr << "Error binding buffer: " << err << std::endl;
        }
    }
    else {
        std::cerr << "Error: Cannot bind buffer - buffer creation failed" << std::endl;
    }
}

void Buffer::unbind() const {
    glBindBuffer(static_cast<GLenum>(type_), 0);
}

void Buffer::upload_data(const void* data, size_t size_bytes, BufferUsage usage) {
    if (data == nullptr) {
        std::cerr << "Error: Attempting to upload null data to buffer" << std::endl;
        return;
    }
    
    if (size_bytes == 0) {
        std::cerr << "Error: Attempting to upload zero bytes to buffer" << std::endl;
        return;
    }
    
    // Additional data validation to prevent driver crashes
    if (size_bytes > UINT32_MAX) {
        std::cerr << "Error: Buffer size too large: " << size_bytes << " bytes" << std::endl;
        return;
    }
    
    // Clear any existing errors first
    while (glGetError() != GL_NO_ERROR);
    
    // Ensure buffer is created and bound
    ensure_created();
    if (buffer_id_ == 0) {
        std::cerr << "Error: Buffer creation failed, cannot upload data" << std::endl;
        return;
    }
    
    // Additional validation before upload
    GLenum target = static_cast<GLenum>(type_);
    GLenum usage_enum = static_cast<GLenum>(usage);
    
    bind();
    
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error after bind: " << error << std::endl;
        return;
    }
    
    // For large buffers, use a safer upload strategy
    const size_t chunk_size = 1024 * 1024; // 1MB chunks
    if (size_bytes > chunk_size) {
        // First allocate the full buffer with null data
        glBufferData(target, static_cast<GLsizeiptr>(size_bytes), nullptr, usage_enum);
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "Error allocating buffer space: " << error << std::endl;
            return;
        }
        
        // Upload data in chunks
        const char* data_ptr = static_cast<const char*>(data);
        size_t offset = 0;
        while (offset < size_bytes) {
            size_t current_chunk = std::min(chunk_size, size_bytes - offset);
            std::cout << "Uploading chunk: offset=" << offset << ", size=" << current_chunk << std::endl;
            
            glBufferSubData(target, static_cast<GLintptr>(offset), 
                          static_cast<GLsizeiptr>(current_chunk), data_ptr + offset);
            error = glGetError();
            if (error != GL_NO_ERROR) {
                std::cerr << "Error uploading chunk at offset " << offset << ": " << error << std::endl;
                return;
            }
            
            offset += current_chunk;
        }
    }
    else {
        // Small buffer, use direct upload
        glBufferData(target, static_cast<GLsizeiptr>(size_bytes), data, usage_enum);
        
        error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "Error in glBufferData: ";
            switch (error) {
                case GL_INVALID_ENUM: std::cerr << "GL_INVALID_ENUM (target=" << target << ", usage=" << usage_enum << ")"; break;
                case GL_INVALID_VALUE: std::cerr << "GL_INVALID_VALUE (size=" << size_bytes << ")"; break;
                case GL_INVALID_OPERATION: std::cerr << "GL_INVALID_OPERATION"; break;
                case GL_OUT_OF_MEMORY: std::cerr << "GL_OUT_OF_MEMORY"; break;
                default: std::cerr << "Unknown error " << error; break;
            }
            std::cerr << std::endl;
            return;
        }
    }
    
    size_bytes_ = size_bytes;
}

void Buffer::update_data(const void* data, size_t size_bytes, size_t offset) {
    bind();
    glBufferSubData(static_cast<GLenum>(type_), offset, size_bytes, data);
    std::string err = check_gl_error("Buffer::update_data");
    if (!err.empty()) {
        std::cerr << "Error updating buffer data: " << err << std::endl;
        return;
    }
}


void Buffer::ensure_created() const {
    if (buffer_id_ == 0) {
        // Clear any existing errors first
        while (glGetError() != GL_NO_ERROR) { /* clear error queue */ }
        
        // Check if we have a valid OpenGL context
        if (!glewIsSupported("GL_VERSION_3_0")) {
            std::cerr << "Error: OpenGL 3.0 not supported when creating buffer" << std::endl;
            return;
        }
        
        // Check current OpenGL context
        if (glGetString(GL_VERSION) == nullptr) {
            std::cerr << "Error: No valid OpenGL context when creating buffer" << std::endl;
            return;
        }
        
        glGenBuffers(1, &const_cast<Buffer*>(this)->buffer_id_);
        
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "Error in glGenBuffers: " << error << std::endl;
            const_cast<Buffer*>(this)->buffer_id_ = 0;
            return;
        }
        
        if (buffer_id_ == 0) {
            std::cerr << "Error: glGenBuffers returned buffer ID 0" << std::endl;
            return;
        }
    }
}


// Explicit template instantiations for common types
template void Buffer::upload_data<float>(const std::vector<float>&, BufferUsage);
template void Buffer::upload_data<int>(const std::vector<int>&, BufferUsage);
template void Buffer::upload_data<unsigned int>(const std::vector<unsigned int>&, BufferUsage);
template void Buffer::upload_data<float>(const float*, size_t, BufferUsage);
template void Buffer::upload_data<int>(const int*, size_t, BufferUsage);
template void Buffer::upload_data<unsigned int>(const unsigned int*, size_t, BufferUsage);

template void Buffer::update_data<float>(const std::vector<float>&, size_t);
template void Buffer::update_data<int>(const std::vector<int>&, size_t);
template void Buffer::update_data<unsigned int>(const std::vector<unsigned int>&, size_t);
template void Buffer::update_data<float>(const float*, size_t, size_t);
template void Buffer::update_data<int>(const int*, size_t, size_t);
template void Buffer::update_data<unsigned int>(const unsigned int*, size_t, size_t);

// Additional explicit template instantiations for types used in the application
template void Buffer::upload_data<glm::vec3>(const std::vector<glm::vec3>&, BufferUsage);
template void Buffer::upload_data<glm::vec4>(const std::vector<glm::vec4>&, BufferUsage);
template void Buffer::upload_data<unsigned short>(const std::vector<unsigned short>&, BufferUsage);
template void Buffer::upload_data<Vertex>(const std::vector<Vertex>&, BufferUsage);

template void Buffer::upload_data<glm::vec3>(const glm::vec3*, size_t, BufferUsage);
template void Buffer::upload_data<glm::vec4>(const glm::vec4*, size_t, BufferUsage);
template void Buffer::upload_data<unsigned short>(const unsigned short*, size_t, BufferUsage);
template void Buffer::upload_data<Vertex>(const Vertex*, size_t, BufferUsage);

template void Buffer::update_data<glm::vec3>(const std::vector<glm::vec3>&, size_t);
template void Buffer::update_data<glm::vec4>(const std::vector<glm::vec4>&, size_t);
template void Buffer::update_data<unsigned short>(const std::vector<unsigned short>&, size_t);
template void Buffer::update_data<Vertex>(const std::vector<Vertex>&, size_t);

template void Buffer::update_data<glm::vec3>(const glm::vec3*, size_t, size_t);
template void Buffer::update_data<glm::vec4>(const glm::vec4*, size_t, size_t);
template void Buffer::update_data<unsigned short>(const unsigned short*, size_t, size_t);
template void Buffer::update_data<Vertex>(const Vertex*, size_t, size_t);
