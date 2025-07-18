#include "index_buffer.hpp"


void IndexBuffer::upload_indices(const std::vector<unsigned int>& indices, BufferUsage usage) {
    upload_data(indices.data(), indices.size(), usage);
    index_type_ = GL_UNSIGNED_INT;
}

void IndexBuffer::upload_indices(const std::vector<unsigned short>& indices, BufferUsage usage) {
    upload_data(indices.data(), indices.size(), usage);
    index_type_ = GL_UNSIGNED_SHORT;
}

void IndexBuffer::update_indices(const std::vector<unsigned int>& indices, size_t offset) {
    update_data(indices.data(), indices.size(), offset);
    index_type_ = GL_UNSIGNED_INT;
}

void IndexBuffer::update_indices(const std::vector<unsigned short>& indices, size_t offset) {
    update_data(indices.data(), indices.size(), offset);
    index_type_ = GL_UNSIGNED_SHORT;
}


size_t IndexBuffer::index_count() const {
    switch (index_type_) {
        case GL_UNSIGNED_INT: return size() / sizeof(unsigned int);
        case GL_UNSIGNED_SHORT: return size() / sizeof(unsigned short);
        case GL_UNSIGNED_BYTE: return size() / sizeof(unsigned char);
        default: return 0;
    }
}
