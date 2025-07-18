#pragma once

#include <string>
#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>


/**
 * @enum ShaderType
 * @brief Specifies the type of shader for rendering.
 */
enum class ShaderType {
    LINES,      /**< Line rendering shader */
    POINTS,     /**< Point rendering shader */
    VOXELS,     /**< Voxel rendering shader */
    OVERLAY     /**< Overlay rendering shader */
};

/**
 * @class Shader
 * @brief Manages OpenGL shader programs, including compilation, linking, and uniform management.
 */
class Shader {
public: // Constructors
    /** @brief Construct a Shader object. */
    Shader() : program_id_(0) {}

    /** @brief Destructor. Cleans up resources. */
    ~Shader();

public: // Methods
    /**
     * @brief Load and compile vertex/fragment shaders from files.
     * @param vertex_path Path to vertex shader file.
     * @param fragment_path Path to fragment shader file.
     * @return True if successful.
     */
    bool load_from_file(const std::string& vertex_path, const std::string& fragment_path);

    /**
     * @brief Load and compile vertex/geometry/fragment shaders from files.
     * @param vertex_path Path to vertex shader file.
     * @param geometry_path Path to geometry shader file.
     * @param fragment_path Path to fragment shader file.
     * @return True if successful.
     */
    bool load_from_file(const std::string& vertex_path, const std::string& geometry_path, const std::string& fragment_path);

    /**
     * @brief Load and compile vertex/fragment shaders from source strings.
     * @param vertex_source Vertex shader source.
     * @param fragment_source Fragment shader source.
     * @return True if successful.
     */
    bool load_from_source(const std::string& vertex_source, const std::string& fragment_source);

    /**
     * @brief Load and compile vertex/geometry/fragment shaders from source strings.
     * @param vertex_source Vertex shader source.
     * @param geometry_source Geometry shader source.
     * @param fragment_source Fragment shader source.
     * @return True if successful.
     */
    bool load_from_source(const std::string& vertex_source, const std::string& geometry_source, const std::string& fragment_source);

    /** @brief Use the shader program. */
    void use() const;

    /** @brief Unuse the shader program. */
    void unuse() const;

    /**
     * @brief Set a boolean uniform value.
     * @param name Uniform name.
     * @param value Boolean value.
     */
    void set_uniform(const std::string& name, bool value);

    /**
     * @brief Set an integer uniform value.
     * @param name Uniform name.
     * @param value Integer value.
     */
    void set_uniform(const std::string& name, int value);

    /**
     * @brief Set a float uniform value.
     * @param name Uniform name.
     * @param value Float value.
     */
    void set_uniform(const std::string& name, float value);

    /**
     * @brief Set a vec2 uniform value.
     * @param name Uniform name.
     * @param value glm::vec2 value.
     */
    void set_uniform(const std::string& name, const glm::vec2& value);

    /**
     * @brief Set a vec3 uniform value.
     * @param name Uniform name.
     * @param value glm::vec3 value.
     */
    void set_uniform(const std::string& name, const glm::vec3& value);

    /**
     * @brief Set a vec4 uniform value.
     * @param name Uniform name.
     * @param value glm::vec4 value.
     */
    void set_uniform(const std::string& name, const glm::vec4& value);

    /**
     * @brief Set a mat3 uniform value.
     * @param name Uniform name.
     * @param value glm::mat3 value.
     */
    void set_uniform(const std::string& name, const glm::mat3& value);

    /**
     * @brief Set a mat4 uniform value.
     * @param name Uniform name.
     * @param value glm::mat4 value.
     */
    void set_uniform(const std::string& name, const glm::mat4& value);

    /**
     * @brief Bind a uniform buffer block to a binding point.
     * @param block_name Block name.
     * @param binding_point Binding point index.
     */
    void bind_uniform_buffer(const std::string& block_name, GLuint binding_point);
    
public: // Getters
    /** @brief Check if the shader program is valid. */
    bool is_valid() const { return program_id_ != 0; }

    /** @brief Get the OpenGL program ID. */
    GLuint id() const { return program_id_; }

    /**
     * @brief Get the location of a vertex attribute.
     * @param name Attribute name.
     * @return Location index.
     */
    GLint get_attribute_location(const std::string& name) const;

private: // Methods
    /**
     * @brief Compile a shader from source.
     * @param source Shader source code.
     * @param type Shader type (GL_VERTEX_SHADER, etc.).
     * @return Shader object ID.
     */
    GLuint compile_shader(const std::string& source, GLenum type);

    /**
     * @brief Link vertex, fragment, and optional geometry shaders into a program.
     * @param vertex Vertex shader ID.
     * @param fragment Fragment shader ID.
     * @param geometry Geometry shader ID (optional).
     * @return True if successful.
     */
    bool link_program(GLuint vertex, GLuint fragment, GLuint geometry = 0);

    /**
     * @brief Read shader source from a file.
     * @param filepath Path to file.
     * @return Source code string.
     */
    std::string read_file(const std::string& filepath);

    /**
     * @brief Check and print shader compile errors.
     * @param shader Shader object ID.
     * @param type Shader type string.
     */
    void check_compile_errors(GLuint shader, const std::string& type);

    /**
     * @brief Get the location of a uniform variable.
     * @param name Uniform name.
     * @return Location index.
     */
    GLint get_uniform_location(const std::string& name) const;

private: // Variables
    GLuint program_id_;
    mutable std::unordered_map<std::string, GLint> uniform_cache_;
};
