#pragma once

#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>

#include "texture.hpp"


/**
 * @enum AttachmentType
 * @brief Specifies the type of attachment for framebuffer objects.
 */
enum class AttachmentType {
    COLOR_ATTACHMENT0 = GL_COLOR_ATTACHMENT0,
    COLOR_ATTACHMENT1 = GL_COLOR_ATTACHMENT1,
    COLOR_ATTACHMENT2 = GL_COLOR_ATTACHMENT2,
    COLOR_ATTACHMENT3 = GL_COLOR_ATTACHMENT3,
    COLOR_ATTACHMENT4 = GL_COLOR_ATTACHMENT4,
    COLOR_ATTACHMENT5 = GL_COLOR_ATTACHMENT5,
    COLOR_ATTACHMENT6 = GL_COLOR_ATTACHMENT6,
    COLOR_ATTACHMENT7 = GL_COLOR_ATTACHMENT7,
    DEPTH_ATTACHMENT = GL_DEPTH_ATTACHMENT,
    STENCIL_ATTACHMENT = GL_STENCIL_ATTACHMENT,
    DEPTH_STENCIL_ATTACHMENT = GL_DEPTH_STENCIL_ATTACHMENT
};

/**
 * @class Framebuffer
 * @brief Manages OpenGL framebuffer objects, including attachments, status, and clearing.
 */
class Framebuffer {
public: // Statics
    /**
     * @brief Create a framebuffer with color and depth attachments.
     * @param width Width of framebuffer.
     * @param height Height of framebuffer.
     * @param color_format Color texture format.
     * @param include_depth Whether to include depth attachment.
     * @return Unique pointer to Framebuffer.
     */
    static std::unique_ptr<Framebuffer> create_with_color_depth(int width, int height, 
        TextureFormat color_format = TextureFormat::RGBA, bool include_depth = true);

public: // Constructors
    /** @brief Construct a Framebuffer object. */
    Framebuffer();

    /** @brief Destructor. Cleans up resources. */
    ~Framebuffer();

public: // Methods
    /** @brief Bind the framebuffer. */
    void bind() const;

    /** @brief Unbind the framebuffer. */
    void unbind() const;

    /**
     * @brief Attach a texture to the framebuffer.
     * @param texture Shared pointer to texture.
     * @param attachment Attachment type.
     */
    void attach_texture(std::shared_ptr<Texture> texture, AttachmentType attachment);

    /**
     * @brief Attach a color texture to the framebuffer.
     * @param texture Shared pointer to texture.
     * @param color_attachment Color attachment index.
     */
    void attach_color_texture(std::shared_ptr<Texture> texture, int color_attachment = 0);

    /**
     * @brief Attach a depth texture to the framebuffer.
     * @param texture Shared pointer to texture.
     */
    void attach_depth_texture(std::shared_ptr<Texture> texture);

    /**
     * @brief Attach a depth-stencil texture to the framebuffer.
     * @param texture Shared pointer to texture.
     */
    void attach_depth_stencil_texture(std::shared_ptr<Texture> texture);

    /**
     * @brief Attach a depth renderbuffer to the framebuffer.
     * @param width Width of renderbuffer.
     * @param height Height of renderbuffer.
     */
    void attach_depth_renderbuffer(int width, int height);

    /**
     * @brief Attach a stencil renderbuffer to the framebuffer.
     * @param width Width of renderbuffer.
     * @param height Height of renderbuffer.
     */
    void attach_stencil_renderbuffer(int width, int height);

    /**
     * @brief Attach a depth-stencil renderbuffer to the framebuffer.
     * @param width Width of renderbuffer.
     * @param height Height of renderbuffer.
     */
    void attach_depth_stencil_renderbuffer(int width, int height);

    /**
     * @brief Set draw buffers for multiple render targets.
     * @param attachments Vector of attachment enums.
     */
    void set_draw_buffers(const std::vector<GLenum>& attachments);

    /** @brief Set draw buffer to none (for depth-only rendering). */
    void set_draw_buffer_none();

    /** @brief Check if the framebuffer is complete. */
    bool is_complete() const;

    /** @brief Get a string describing the framebuffer status. */
    std::string get_status_string() const;

    /**
     * @brief Clear the framebuffer with specified color and depth.
     * @param r Red value.
     * @param g Green value.
     * @param b Blue value.
     * @param a Alpha value.
     * @param depth Depth value.
     */
    void clear(float r = 0.0f, float g = 0.0f, float b = 0.0f, float a = 1.0f, float depth = 1.0f);

public: // Getters
    /** @brief Get the OpenGL framebuffer ID. */
    GLuint id() const { return framebuffer_id_; }

    /** @brief Check if the framebuffer is valid. */
    bool is_valid() const { return framebuffer_id_ != 0; }

    /** @brief Get the color textures attached to the framebuffer. */
    const std::vector<std::shared_ptr<Texture>>& color_textures() const { return color_textures_; }

    /** @brief Get the depth texture attached to the framebuffer. */
    std::shared_ptr<Texture> depth_texture() const { return depth_texture_; }

private: // Variables
    GLuint framebuffer_id_;
    GLuint depth_renderbuffer_id_;
    GLuint stencil_renderbuffer_id_;
    
    std::vector<std::shared_ptr<Texture>> color_textures_;
    std::shared_ptr<Texture> depth_texture_;
};
