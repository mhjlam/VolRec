#pragma once

#include <GL/glew.h>
#include <opencv2/opencv.hpp>

/**
 * @enum TextureFormat
 * @brief Specifies the internal format for OpenGL textures.
 */
enum class TextureFormat {
    RGB = GL_RGB8,           /**< 8-bit RGB */
    RGBA = GL_RGBA8,         /**< 8-bit RGBA */
    RGB16F = GL_RGB16F,      /**< 16-bit float RGB */
    RGBA16F = GL_RGBA16F,    /**< 16-bit float RGBA */
    RGB32F = GL_RGB32F,      /**< 32-bit float RGB */
    RGBA32F = GL_RGBA32F,    /**< 32-bit float RGBA */
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT, /**< Depth component */
    DEPTH_COMPONENT16 = GL_DEPTH_COMPONENT16,
    DEPTH_COMPONENT24 = GL_DEPTH_COMPONENT24,
    DEPTH_COMPONENT32F = GL_DEPTH_COMPONENT32F,
    DEPTH24_STENCIL8 = GL_DEPTH24_STENCIL8,
    DEPTH32F_STENCIL8 = GL_DEPTH32F_STENCIL8
};

/**
 * @enum TextureFilter
 * @brief Specifies the filtering mode for OpenGL textures.
 */
enum class TextureFilter {
    NEAREST = GL_NEAREST,    /**< Nearest neighbor filtering */
    LINEAR = GL_LINEAR       /**< Linear filtering */
};

/**
 * @enum TextureWrap
 * @brief Specifies the wrapping mode for OpenGL textures.
 */
enum class TextureWrap {
    CLAMP_TO_EDGE = GL_CLAMP_TO_EDGE,         /**< Clamp to edge */
    CLAMP_TO_BORDER = GL_CLAMP_TO_BORDER,     /**< Clamp to border */
    REPEAT = GL_REPEAT,                       /**< Repeat */
    MIRRORED_REPEAT = GL_MIRRORED_REPEAT      /**< Mirrored repeat */
};

/**
 * @class Texture
 * @brief Manages OpenGL texture objects, including creation, parameter setting, and data upload.
 */
class Texture {
public: // Constructors
    /**
     * @brief Construct a Texture object.
     * @param target OpenGL texture target (default GL_TEXTURE_2D).
     */
    Texture(GLenum target = GL_TEXTURE_2D);

    /** @brief Destructor. Cleans up resources. */
    ~Texture();

public: // Methods
    /**
     * @brief Bind the texture to a texture unit.
     * @param texture_unit Texture unit index.
     */
    void bind(GLuint texture_unit = 0) const;

    /** @brief Unbind the texture. */
    void unbind() const;

    /**
     * @brief Create 2D texture storage.
     * @param width Texture width.
     * @param height Texture height.
     * @param internal_format Internal format enum.
     * @param format Format (default GL_RGBA).
     * @param type Data type (default GL_UNSIGNED_BYTE).
     */
    void create_2d(int width, int height, TextureFormat internal_format, GLenum format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

    /**
     * @brief Set texture filtering parameters.
     * @param min_filter Minification filter.
     * @param mag_filter Magnification filter.
     */
    void set_filter(TextureFilter min_filter, TextureFilter mag_filter);

    /**
     * @brief Set texture wrapping parameters.
     * @param wrap_s S axis wrap mode.
     * @param wrap_t T axis wrap mode.
     */
    void set_wrap(TextureWrap wrap_s, TextureWrap wrap_t);

    /**
     * @brief Set border color for the texture.
     * @param r Red value.
     * @param g Green value.
     * @param b Blue value.
     * @param a Alpha value (default 1.0f).
     */
    void set_border_color(float r, float g, float b, float a = 1.0f);

    /** @brief Generate mipmaps for the texture. */
    void generate_mipmaps();

    /**
     * @brief Upload data from an OpenCV Mat to the texture.
     * @param mat OpenCV matrix.
     */
    void upload_from_mat(const cv::Mat& mat);

    /**
     * @brief Upload sub-region data to the texture.
     * @param x X offset.
     * @param y Y offset.
     * @param width Width of region.
     * @param height Height of region.
     * @param format Data format.
     * @param type Data type.
     * @param data Pointer to data.
     */
    void upload_sub_data(int x, int y, int width, int height, GLenum format, GLenum type, const void* data);

public: // Getters
    /** @brief Get the OpenGL texture ID. */
    GLuint id() const { return texture_id_; }

    /** @brief Get the OpenGL texture target. */
    GLenum target() const { return target_; }

    /** @brief Get the texture width. */
    int width() const { return width_; }

    /** @brief Get the texture height. */
    int height() const { return height_; }

    /** @brief Check if the texture is valid. */
    bool is_valid() const { return texture_id_ != 0; }

private: // Variables
    int width_;
    int height_;

    GLenum target_;
    GLuint texture_id_;
};
