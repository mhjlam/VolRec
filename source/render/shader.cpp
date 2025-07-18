#include "shader.hpp"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

#ifdef _WIN32
#include <windows.h>
#include <string>
#endif


/* Constructors */

Shader::~Shader() {
    if (program_id_ != 0) {
        glDeleteProgram(program_id_);
    }
}


/* Public methods */

bool Shader::load_from_file(const std::string& vertex_path, const std::string& fragment_path) {
    std::string vertex_source = read_file(vertex_path);
    std::string fragment_source = read_file(fragment_path);
    
    if (vertex_source.empty() || fragment_source.empty()) {
        return false;
    }
    
    return load_from_source(vertex_source, fragment_source);
}

bool Shader::load_from_file(const std::string& vertex_path, 
                            const std::string& geometry_path, 
                            const std::string& fragment_path) {
    std::string vertex_source = read_file(vertex_path);
    std::string geometry_source = read_file(geometry_path);
    std::string fragment_source = read_file(fragment_path);
    
    if (vertex_source.empty() || geometry_source.empty() || fragment_source.empty()) {
        return false;
    }
    
    return load_from_source(vertex_source, geometry_source, fragment_source);
}

bool Shader::load_from_source(const std::string& vertex_source, const std::string& fragment_source) {
    GLuint vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
    
    if (vertex_shader == 0 || fragment_shader == 0) {
        if (vertex_shader) { glDeleteShader(vertex_shader); }
        if (fragment_shader) { glDeleteShader(fragment_shader); }
        return false;
    }
    
    bool success = link_program(vertex_shader, fragment_shader);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return success;
}

bool Shader::load_from_source(const std::string& vertex_source, const std::string& geometry_source, const std::string& fragment_source) {
    GLuint vertex_shader = compile_shader(vertex_source, GL_VERTEX_SHADER);
    GLuint geometry_shader = compile_shader(geometry_source, GL_GEOMETRY_SHADER);
    GLuint fragment_shader = compile_shader(fragment_source, GL_FRAGMENT_SHADER);
    
    if (vertex_shader == 0 || geometry_shader == 0 || fragment_shader == 0) {
        if (vertex_shader) { glDeleteShader(vertex_shader); }
        if (geometry_shader) { glDeleteShader(geometry_shader); }
        if (fragment_shader) { glDeleteShader(fragment_shader); }
        return false;
    }
    
    bool success = link_program(vertex_shader, fragment_shader, geometry_shader);
    
    glDeleteShader(vertex_shader);
    glDeleteShader(geometry_shader);
    glDeleteShader(fragment_shader);
    
    return success;
}

void Shader::use() const {
    if (program_id_ != 0) {
        glUseProgram(program_id_);
    }
}

void Shader::unuse() const {
    glUseProgram(0);
}

void Shader::set_uniform(const std::string& name, bool value) {
    glUniform1i(get_uniform_location(name), static_cast<int>(value));
}

void Shader::set_uniform(const std::string& name, int value) {
    glUniform1i(get_uniform_location(name), value);
}

void Shader::set_uniform(const std::string& name, float value) {
    glUniform1f(get_uniform_location(name), value);
}

void Shader::set_uniform(const std::string& name, const glm::vec2& value) {
    glUniform2fv(get_uniform_location(name), 1, glm::value_ptr(value));
}

void Shader::set_uniform(const std::string& name, const glm::vec3& value) {
    glUniform3fv(get_uniform_location(name), 1, glm::value_ptr(value));
}

void Shader::set_uniform(const std::string& name, const glm::vec4& value) {
    glUniform4fv(get_uniform_location(name), 1, glm::value_ptr(value));
}

void Shader::set_uniform(const std::string& name, const glm::mat3& value) {
    glUniformMatrix3fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::set_uniform(const std::string& name, const glm::mat4& value) {
    glUniformMatrix4fv(get_uniform_location(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::bind_uniform_buffer(const std::string& block_name, GLuint binding_point) {
    if (program_id_ == 0) { return; }
    GLuint block_index = glGetUniformBlockIndex(program_id_, block_name.c_str());
    if (block_index != GL_INVALID_INDEX) {
        glUniformBlockBinding(program_id_, block_index, binding_point);
    }
}


/* Getters */

GLint Shader::get_attribute_location(const std::string& name) const {
    if (program_id_ == 0) {
        return -1;
    }
    return glGetAttribLocation(program_id_, name.c_str());
}


/* Private methods */

GLuint Shader::compile_shader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* source_cstr = source.c_str();
    glShaderSource(shader, 1, &source_cstr, nullptr);
    glCompileShader(shader);
    
    std::string type_string;
    switch (type) {
        case GL_VERTEX_SHADER: type_string = "VERTEX"; break;
        case GL_FRAGMENT_SHADER: type_string = "FRAGMENT"; break;
        case GL_GEOMETRY_SHADER: type_string = "GEOMETRY"; break;
        default: type_string = "UNKNOWN"; break;
    }
    
    check_compile_errors(shader, type_string);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool Shader::link_program(GLuint vertex, GLuint fragment, GLuint geometry) {
    if (program_id_ != 0) {
        glDeleteProgram(program_id_);
    }
    
    program_id_ = glCreateProgram();
    glAttachShader(program_id_, vertex);
    glAttachShader(program_id_, fragment);
    if (geometry != 0) {
        glAttachShader(program_id_, geometry);
    }
    
    glLinkProgram(program_id_);
    check_compile_errors(program_id_, "PROGRAM");
    
    GLint success;
    glGetProgramiv(program_id_, GL_LINK_STATUS, &success);
    if (!success) {
        glDeleteProgram(program_id_);
        program_id_ = 0;
        return false;
    }
    
    uniform_cache_.clear();
    return true;
}

std::string Shader::read_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filepath << std::endl;
        return "";
    }
    
    std::stringstream stream;
    stream << file.rdbuf();
    file.close();
    
    return stream.str();
}

void Shader::check_compile_errors(GLuint shader, const std::string& type) {
    GLint success;
    GLchar info_log[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, info_log);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" 
                      << info_log << "\n -- --------------------------------------------------- --" << std::endl;
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, info_log);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" 
                      << info_log << "\n -- --------------------------------------------------- --" << std::endl;
        }
    }
}

GLint Shader::get_uniform_location(const std::string& name) const {
    if (program_id_ == 0) {
        return -1;
    }
    
    auto it = uniform_cache_.find(name);
    if (it != uniform_cache_.end()) {
        return it->second;
    }
    
    GLint location = glGetUniformLocation(program_id_, name.c_str());
    uniform_cache_[name] = location;
    return location;
}
