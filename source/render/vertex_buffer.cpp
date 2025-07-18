#include "vertex_buffer.hpp"

#include "vertex.hpp"


// Explicit template instantiations for common vertex types
template void VertexBuffer::upload_vertices<Vertex>(const std::vector<Vertex>&, BufferUsage);
template void VertexBuffer::update_vertices<Vertex>(const std::vector<Vertex>&, size_t);
template size_t VertexBuffer::vertex_count<Vertex>() const;

// Template instantiations for common primitive types
template void VertexBuffer::upload_vertices<float>(const std::vector<float>&, BufferUsage);
template void VertexBuffer::upload_vertices<glm::vec3>(const std::vector<glm::vec3>&, BufferUsage);
template void VertexBuffer::upload_vertices<glm::vec4>(const std::vector<glm::vec4>&, BufferUsage);
template void VertexBuffer::update_vertices<float>(const std::vector<float>&, size_t);
template void VertexBuffer::update_vertices<glm::vec3>(const std::vector<glm::vec3>&, size_t);
template void VertexBuffer::update_vertices<glm::vec4>(const std::vector<glm::vec4>&, size_t);
template size_t VertexBuffer::vertex_count<float>() const;
template size_t VertexBuffer::vertex_count<glm::vec3>() const;
template size_t VertexBuffer::vertex_count<glm::vec4>() const;
