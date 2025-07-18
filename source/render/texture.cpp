#include "texture.hpp"

#include <iostream>


/* Constructors */

Texture::Texture(GLenum target) : texture_id_(0), target_(target), width_(0), height_(0) {
    glGenTextures(1, &texture_id_);
}

Texture::~Texture() {
    if (texture_id_ != 0) {
        glDeleteTextures(1, &texture_id_);
    }
}


/* Public methods */

void Texture::bind(GLuint texture_unit) const {
    if (texture_id_ != 0) {
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        glBindTexture(target_, texture_id_);
    }
}

void Texture::unbind() const {
    glBindTexture(target_, 0);
}

void Texture::create_2d(int width, int height, TextureFormat internal_format, GLenum format, GLenum type) {
    width_ = width;
    height_ = height;
    
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, static_cast<GLenum>(internal_format), width, height, 0, format, type, nullptr);
    
    // Set default parameters
    set_filter(TextureFilter::LINEAR, TextureFilter::LINEAR);
    set_wrap(TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE);
}

void Texture::set_filter(TextureFilter min_filter, TextureFilter mag_filter) {
    bind();
    glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, static_cast<GLenum>(min_filter));
    glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, static_cast<GLenum>(mag_filter));
}

void Texture::set_wrap(TextureWrap wrap_s, TextureWrap wrap_t) {
    bind();
    glTexParameteri(target_, GL_TEXTURE_WRAP_S, static_cast<GLenum>(wrap_s));
    glTexParameteri(target_, GL_TEXTURE_WRAP_T, static_cast<GLenum>(wrap_t));
}

void Texture::set_border_color(float r, float g, float b, float a) {
    float border_color[] = {r, g, b, a};
    bind();
    glTexParameterfv(target_, GL_TEXTURE_BORDER_COLOR, border_color);
}

void Texture::generate_mipmaps() {
    bind();
    glGenerateMipmap(target_);
}

void Texture::upload_from_mat(const cv::Mat& mat) {
    if (mat.empty()) {
        std::cerr << "Cannot upload empty Mat to texture" << std::endl;
        return;
    }

    width_ = mat.cols;
    height_ = mat.rows;
    
    bind();
    
    // Convert OpenCV format to OpenGL format
    GLenum format;
    GLenum internal_format;
    GLenum type = GL_UNSIGNED_BYTE;
    
    switch (mat.channels()) {
        case 1:
            format = GL_RED;
            internal_format = GL_R8;
            break;

        case 3:
            format = GL_BGR; // OpenCV uses BGR by default
            internal_format = GL_RGB8;
            break;

        case 4:
            format = GL_BGRA; // OpenCV uses BGRA by default
            internal_format = GL_RGBA8;
            break;
            
        default:
            std::cerr << "Unsupported number of channels: " << mat.channels() << std::endl;
            return;
    }
    
    // Ensure the data is continuous
    cv::Mat continuous_mat;
    if (mat.isContinuous()) {
        continuous_mat = mat;
    }
    else {
        mat.copyTo(continuous_mat);
    }
    
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width_, height_, 0, format, type, continuous_mat.data);
    
    // Set default parameters
    set_filter(TextureFilter::LINEAR, TextureFilter::LINEAR);
    set_wrap(TextureWrap::CLAMP_TO_EDGE, TextureWrap::CLAMP_TO_EDGE);
}

void Texture::upload_sub_data(int x, int y, int width, int height, GLenum format, GLenum type, const void* data) {
    bind();
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, format, type, data);
}
