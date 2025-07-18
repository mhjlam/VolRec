#include "framebuffer.hpp"

#include <iostream>

#include "texture.hpp"


std::unique_ptr<Framebuffer> Framebuffer::create_with_color_depth(int width, int height, TextureFormat color_format, bool include_depth) {
    auto framebuffer = std::make_unique<Framebuffer>();
    
    // Create color texture
    auto color_texture = std::make_shared<Texture>();
    color_texture->create_2d(width, height, color_format);
    framebuffer->attach_color_texture(color_texture, 0);

    if (include_depth) {
        // Create depth texture or renderbuffer
        auto depth_texture = std::make_shared<Texture>();
        depth_texture->create_2d(width, height, TextureFormat::DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT);
        framebuffer->attach_depth_texture(depth_texture);
    }
    
    // Set draw buffer
    std::vector<GLenum> draw_buffers = { GL_COLOR_ATTACHMENT0 };
    framebuffer->set_draw_buffers(draw_buffers);
    
    if (!framebuffer->is_complete()) {
        std::cerr << "Framebuffer incomplete: " << framebuffer->get_status_string() << std::endl;
        return nullptr;
    }
    
    return framebuffer;
}


Framebuffer::Framebuffer()
: framebuffer_id_(0)
, depth_renderbuffer_id_(0)
, stencil_renderbuffer_id_(0) {
    glGenFramebuffers(1, &framebuffer_id_);
}

Framebuffer::~Framebuffer() {
    if (depth_renderbuffer_id_ != 0) {
        glDeleteRenderbuffers(1, &depth_renderbuffer_id_);
    }
    if (stencil_renderbuffer_id_ != 0) {
        glDeleteRenderbuffers(1, &stencil_renderbuffer_id_);
    }
    if (framebuffer_id_ != 0) {
        glDeleteFramebuffers(1, &framebuffer_id_);
    }
}


void Framebuffer::bind() const {
    if (framebuffer_id_ != 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id_);
    }
}

void Framebuffer::unbind() const {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::attach_texture(std::shared_ptr<Texture> texture, AttachmentType attachment) {
    if (!texture || framebuffer_id_ == 0) { return; }

    bind();
    
    GLenum attach_point = static_cast<GLenum>(attachment);
    glFramebufferTexture2D(GL_FRAMEBUFFER, attach_point, GL_TEXTURE_2D, texture->id(), 0);
    
    // Store texture reference based on attachment type
    if (attach_point >= GL_COLOR_ATTACHMENT0 && attach_point <= GL_COLOR_ATTACHMENT7) {
        int index = attach_point - GL_COLOR_ATTACHMENT0;
        if (index >= static_cast<int>(color_textures_.size())) {
            color_textures_.resize(index + 1);
        }
        color_textures_[index] = texture;
    }
    else if (attach_point == GL_DEPTH_ATTACHMENT || attach_point == GL_DEPTH_STENCIL_ATTACHMENT) {
        depth_texture_ = texture;
    }
}

void Framebuffer::attach_color_texture(std::shared_ptr<Texture> texture, int color_attachment) {
    if (color_attachment < 0 || color_attachment > 7) {
        std::cerr << "Invalid color attachment index: " << color_attachment << std::endl;
        return;
    }
    
    AttachmentType attachment = static_cast<AttachmentType>(GL_COLOR_ATTACHMENT0 + color_attachment);
    attach_texture(texture, attachment);
}

void Framebuffer::attach_depth_texture(std::shared_ptr<Texture> texture) {
    attach_texture(texture, AttachmentType::DEPTH_ATTACHMENT);
}

void Framebuffer::attach_depth_stencil_texture(std::shared_ptr<Texture> texture) {
    attach_texture(texture, AttachmentType::DEPTH_STENCIL_ATTACHMENT);
}

void Framebuffer::attach_depth_renderbuffer(int width, int height) {
    if (framebuffer_id_ == 0) { return; }

    if (depth_renderbuffer_id_ == 0) {
        glGenRenderbuffers(1, &depth_renderbuffer_id_);
    }

    bind();
    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_id_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer_id_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::attach_stencil_renderbuffer(int width, int height) {
    if (framebuffer_id_ == 0) { return; }

    if (stencil_renderbuffer_id_ == 0) {
        glGenRenderbuffers(1, &stencil_renderbuffer_id_);
    }

    bind();
    glBindRenderbuffer(GL_RENDERBUFFER, stencil_renderbuffer_id_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencil_renderbuffer_id_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::attach_depth_stencil_renderbuffer(int width, int height) {
    if (framebuffer_id_ == 0) { return; }

    if (depth_renderbuffer_id_ == 0) {
        glGenRenderbuffers(1, &depth_renderbuffer_id_);
    }

    bind();
    glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer_id_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer_id_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

void Framebuffer::set_draw_buffers(const std::vector<GLenum>& attachments) {
    if (framebuffer_id_ == 0) { return; }
    
    bind();
    glDrawBuffers(static_cast<GLsizei>(attachments.size()), attachments.data());
}

void Framebuffer::set_draw_buffer_none() {
    if (framebuffer_id_ == 0) { return; }
    
    bind();
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
}

bool Framebuffer::is_complete() const {
    if (framebuffer_id_ == 0) {
        return false;
    }
    
    bind();
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    return status == GL_FRAMEBUFFER_COMPLETE;
}

std::string Framebuffer::get_status_string() const {
    if (framebuffer_id_ == 0) {
        return "Invalid framebuffer";
    }
    
    bind();
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            return "Complete";

        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            return "Incomplete attachment";

        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            return "Missing attachment";

        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            return "Incomplete draw buffer";

        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            return "Incomplete read buffer";

        case GL_FRAMEBUFFER_UNSUPPORTED:
            return "Unsupported";

        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            return "Incomplete multisample";
            
        default:
            return "Unknown error";
    }
}

void Framebuffer::clear(float r, float g, float b, float a, float depth) {
    bind();
    
    glClearColor(r, g, b, a);
    glClearDepth(depth);
    
    GLbitfield mask = GL_COLOR_BUFFER_BIT;
    if (depth_texture_ || depth_renderbuffer_id_ != 0) {
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (stencil_renderbuffer_id_ != 0) {
        mask |= GL_STENCIL_BUFFER_BIT;
    }
    
    glClear(mask);
}
