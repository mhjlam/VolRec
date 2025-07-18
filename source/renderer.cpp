#include "renderer.hpp"

#include <ranges>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <opencv2/opencv.hpp>

#include "view.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "global.hpp"

#include "model/box.hpp"
#include "model/floor.hpp"
#include "model/frame.hpp"
#include "model/model.hpp"
#include "model/volume.hpp"
#include "model/frustum.hpp"
#include "model/checkers.hpp"

#include "render/mesh.hpp"
#include "render/texture.hpp"


/* Constructors */

Renderer::Renderer(int width, int height, std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera)
: show_box_{true}
, show_frame_{true}
, show_floor_{true}
, show_frustums_{true}
, show_checkers_{true}
, show_volume_{false}
, project_{std::make_shared<Project>()}
, scene_{scene}
, camera_{camera}
, aspect_ratio_{1.0f}
{
    // Set window dimensions and aspect ratio
    window_width_ = static_cast<float>(width);
    window_height_ = static_cast<float>(height);
    aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);

    // Set OpenGL viewport to match window size
    glViewport(0, 0, width, height);

    // Set up OpenGL state for modern rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS); // Standard depth testing
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    
    // Enable MSAA (Multisample Anti-Aliasing) for modern anti-aliasing
    glEnable(GL_MULTISAMPLE);
    
    glEnable(GL_PROGRAM_POINT_SIZE); // Allow vertex shader to set point size
    
    // Enable backface culling to improve cube rendering
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW); // Counter-clockwise front faces
    
    // Improve depth precision to reduce z-fighting
    glDepthRange(0.0, 1.0);
    glClearDepth(1.0);

    // Initialize shaders
    if (!initialize_shaders()) {
        std::cerr << "Failed to initialize shaders!" << std::endl;
    }

    // Set up image overlay system
    initialize_textures();
}


/* Public methods */

void Renderer::load_project(std::shared_ptr<Project> project) {
    project_ = project;

    // Set rendering flags
    show_box_ = true;
    show_floor_ = true;
    show_frame_ = true;
    show_volume_ = true;
    show_frustums_ = true;
    show_background_ = false;

    // Initialize shaders and textures resources
    initialize_shaders();
    initialize_textures();
}

void Renderer::unload_project() {
    project_ = std::make_shared<Project>();

    show_box_ = true;
    show_floor_ = true;
    show_frame_ = true;
    show_volume_ = false;
    show_frustums_ = false;
    show_background_ = false;

    background_texture_.reset();
    foreground_texture_.reset();
    overlay_resources_initialized_ = false;

    // Initialize shaders and textures resources
    initialize_shaders();
    initialize_textures();
}

void Renderer::resize(int width, int height) {
    window_width_ = static_cast<float>(width);
    window_height_ = static_cast<float>(height);
    aspect_ratio_ = static_cast<float>(width) / static_cast<float>(height);

    // Update OpenGL viewport
    glViewport(0, 0, width, height);
}

void Renderer::render() {
    // Safety check: don't render if no scene is set
    if (!scene_) {
        return;
    }

    // Update overlay textures before rendering
    if (show_background_) {
        update_overlay_textures();
    }

    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render image overlay first (behind 3D content)
    if (show_background_ && !camera_->current_view().bg.empty()) {
        render_image_overlay();
    }

    // Draw the bounding box of the volume (wireframe, should be on top)
    if (show_box_) {
        render_box();
    }

    // Draw the floor grid first (lowest priority)
    if (show_floor_) {
        render_floor();
    }

    // Draw the checkers (on the floor, after grid)
    if (show_checkers_) {
        render_checkers();
    }

    // Draw the volume (will write proper depth values)
    if (show_volume_) {
        render_volume();
    }
    
    // Draw the world axes (after volume, should be properly occluded by depth testing)
    if (show_frame_) {
        render_frame();
    }
    
    // Draw the camera frustums (after everything else)
    if (show_frustums_) {
        render_frustums();
    }
}

void Renderer::toggle_volume_render_mode() {
    auto volume = scene_->volume();
    if (volume) {
        switch (volume->render_mode()) {
            case VolumeRenderMode::POINT_CLOUD:
                volume->set_render_mode(VolumeRenderMode::VOXEL_CUBES);
                break;
                
            case VolumeRenderMode::VOXEL_CUBES:
                volume->set_render_mode(VolumeRenderMode::POINT_CLOUD);
                break;
        }
    }
}


/* Getters */

std::shared_ptr<Shader> Renderer::get_shader(ShaderType type) const {
    if (auto it = shaders_.find(type); it != shaders_.end()) {
        return it->second;
    }
    return nullptr;
}

glm::mat4 Renderer::mvp_matrix(const glm::mat4& model_matrix) const {
    auto eye = camera_->eye();
    auto center = camera_->at();
    auto up = camera_->up();
    
    return camera_->proj_matrix() * glm::lookAt(eye, center, up) * model_matrix;
}


/* Private methods */

bool Renderer::initialize_shaders() {
    bool success = true;

    // Get the directory where the executable is located
    std::string exe_dir = get_executable_dir();

    // Helper lambda to build shader paths
    auto shader_path = [&](const std::string& rel_path) {
        return exe_dir + "/" + rel_path;
    };

    // Load unified shader
    auto lines_shader = std::make_shared<Shader>();
    if (lines_shader->load_from_file(shader_path("shaders/lines.vert"), shader_path("shaders/lines.frag"))) {
        shaders_[ShaderType::LINES] = lines_shader;
    }
    else {
        std::cerr << "Failed to load lines shader" << std::endl;
        success = false;
    }

    // Load point cloud shader
    auto points_shader = std::make_shared<Shader>();
    if (points_shader->load_from_file(shader_path("shaders/points.vert"), shader_path("shaders/points.frag"))) {
        shaders_[ShaderType::POINTS] = points_shader;
    }
    else {
        std::cerr << "Failed to load points shader" << std::endl;
        success = false;
    }

    // Load volume voxel instanced shader
    auto voxels_shader = std::make_shared<Shader>();
    if (voxels_shader->load_from_file(shader_path("shaders/voxels.vert"), shader_path("shaders/voxels.frag"))) {
        shaders_[ShaderType::VOXELS] = voxels_shader;
    }
    else {
        std::cerr << "Failed to load voxels shader" << std::endl;
        success = false;
    }

    // Load image overlay shader
    auto overlay_shader = std::make_shared<Shader>();
    if (overlay_shader->load_from_file(shader_path("shaders/overlay.vert"), shader_path("shaders/overlay.frag"))) {
        shaders_[ShaderType::OVERLAY] = overlay_shader;
    }
    else {
        std::cerr << "Failed to load overlay shader" << std::endl;
        success = false;
    }

    return success;
}

void Renderer::initialize_textures() {
    if (overlay_resources_initialized_) { return; }
    
    // Create textures for background and foreground images
    background_texture_ = std::make_unique<Texture>();
    foreground_texture_ = std::make_unique<Texture>();
    
    // Create a full-screen quad for rendering the overlay (using triangles)
    float vertices[] = {
        // positions     // texture coords (flipped Y to fix upside-down texture)
        -1.0f, -1.0f,     0.0f, 1.0f,   // bottom left
         1.0f, -1.0f,     1.0f, 1.0f,   // bottom right
         1.0f,  1.0f,     1.0f, 0.0f,   // top right
        
        -1.0f, -1.0f,     0.0f, 1.0f,   // bottom left  
         1.0f,  1.0f,     1.0f, 0.0f,   // top right
        -1.0f,  1.0f,     0.0f, 0.0f    // top left
    };
    
    // Generate vertex array and buffer objects (no EBO needed for triangle list)
    glGenVertexArrays(1, &overlay_vao_);
    glGenBuffers(1, &overlay_vbo_);
    
    glBindVertexArray(overlay_vao_);
    
    glBindBuffer(GL_ARRAY_BUFFER, overlay_vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
    
    overlay_resources_initialized_ = true;
}

void Renderer::update_overlay_textures() {
    if (!overlay_resources_initialized_) {
        return;
    }
    
    // Only update textures if we're in static view mode
    if (!camera_->in_static_view()) {
        return;
    }

    const View& view = camera_->current_view();
    
    // Update background texture if overlay is enabled
    if (show_background_ && !view.bg.empty()) {
        background_texture_->upload_from_mat(view.bg);
    }
}

void Renderer::calc_bg_transform(float& scale_x, float& scale_y, float& offset_x, float& offset_y) const {
    scale_x = scale_y = 1.0f;
    offset_x = offset_y = 0.0f;
    
    // Get current image dimensions for background overlay
    if (!camera_->in_static_view()) {
        return;
    }
    
    const View& view = camera_->current_view();
    
    if (!show_background_ || view.bg.empty()) {
        return; // No valid image
    }
    
    // Calculate aspect ratios
    float image_aspect = static_cast<float>(view.bg.cols) / static_cast<float>(view.bg.rows);
    float window_aspect = window_width_ / window_height_;
    
    // Scale to fit image within window while maintaining aspect ratio
    if (image_aspect > window_aspect) {
        // Image is wider than window - fit to width, letterbox top/bottom
        scale_x = 1.0f;
        scale_y = window_aspect / image_aspect;
    }
    else {
        // Image is taller than window - fit to height, pillarbox left/right  
        scale_x = image_aspect / window_aspect;
        scale_y = 1.0f;
    }
}

void Renderer::render_box() const {
    auto box = scene_->box();
    if (!box || !box->is_visible() || !box->is_ready_to_render()) { return; }

    // Use unified shader for box rendering
    auto shader = get_shader(ShaderType::LINES);
    if (!shader || !shader->is_valid()) { return; }
    
    shader->use();
    
    // Set uniforms
    shader->set_uniform("mvp_matrix", mvp_matrix());
    shader->set_uniform("model_matrix", box->transform());
    shader->set_uniform("model_color", box->color());
    shader->set_uniform("depth_bias", 0.0f); // No depth bias for box
    shader->set_uniform("use_chessboard_layout", false); // Use standard color layout
    
    // Render all meshes
    const auto& meshes = box->meshes();
    for (const auto& mesh : meshes) {
        if (mesh) {
            mesh->bind();
            draw_mesh(*mesh);
            mesh->unbind();
        }
    }
    
    shader->unuse();
}

void Renderer::render_floor() const {
    auto floor = scene_->floor();
    if (!floor || !floor->is_visible() || !floor->is_ready_to_render()) { return; }

    // Use unified shader for floor grid rendering
    auto shader = get_shader(ShaderType::LINES);
    if (!shader || !shader->is_valid()) { return; }
    
    shader->use();
    
    // Set uniforms
    shader->set_uniform("mvp_matrix", mvp_matrix());
    shader->set_uniform("model_matrix", floor->transform());
    shader->set_uniform("model_color", floor->color());
    shader->set_uniform("depth_bias", 0.0f); // No depth bias for floor
    shader->set_uniform("use_chessboard_layout", false); // Use standard color layout
    
    // Render all meshes
    const auto& meshes = floor->meshes();
    for (const auto& mesh : meshes) {
        if (mesh) {
            mesh->bind();
            draw_mesh(*mesh);
            mesh->unbind();
        }
    }
    
    shader->unuse();
}

void Renderer::render_frame() const {
    auto axes = scene_->frame();
    if (!axes || !axes->is_visible() || !axes->is_ready_to_render()) { return; }

    // Use unified shader for axes rendering (each axis has its own color)
    auto shader = get_shader(ShaderType::LINES);  
    if (!shader || !shader->is_valid()) { return; }
    
    // Ensure proper depth testing state for axes
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_FALSE); // Don't write to depth buffer - just test against existing depth
    
    // Ensure line rendering doesn't interfere with depth testing
    glLineWidth(1.0f);
    glDisable(GL_LINE_SMOOTH); // Line smoothing can interfere with depth
    
    shader->use();
    
    // Set uniforms
    shader->set_uniform("mvp_matrix", mvp_matrix());
    shader->set_uniform("model_matrix", axes->transform());
    shader->set_uniform("model_color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // Default color (will be overridden by vertex colors)
    shader->set_uniform("depth_bias", 0.00001f); // Small depth bias to win Z-fighting against floor
    shader->set_uniform("use_chessboard_layout", false); // Use standard color layout
    
    // Render all meshes (each axis as separate mesh with colors)
    const auto& meshes = axes->meshes();
    for (const auto& mesh : meshes) {
        if (mesh) {
            mesh->bind();
            draw_mesh(*mesh);
            mesh->unbind();
        }
    }
    
    shader->unuse();
    
    // Ensure consistent OpenGL state after axes rendering
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
}

void Renderer::render_volume() const {
    auto volume = scene_->volume();
    if (!volume || !volume->is_visible() || !volume->is_ready_to_render()) { return; }

    size_t active_count = volume->active_voxel_count();
    if (active_count == 0) { return; }

    // Choose rendering mode - Volume has its own render mode
    if (volume->render_mode() == VolumeRenderMode::POINT_CLOUD) {
        // Render as point cloud - modern shader-based approach
        auto shader = get_shader(ShaderType::POINTS);
        if (shader && shader->is_valid()) {
            shader->use();
            
            // Set uniforms - apply volume's transform
            auto mvp = mvp_matrix(volume->transform());
            shader->set_uniform("mvp_matrix", mvp);
            shader->set_uniform("model_matrix", volume->transform());
            shader->set_uniform("point_size", 2.0f); // Reasonable base size multiplier
            shader->set_uniform("voxel_size", static_cast<float>(VOLUME_VOXEL_SIZE)); // Actual voxel size from global constants
            
            // Use volume's modern OpenGL rendering
            volume->bind();
            draw_volume(*volume);
            volume->unbind();
            
            shader->unuse();
        }
        else {
            static bool once = false;
            if (!once) {
                std::cerr << "ERROR: Point cloud shader not found or invalid!" << std::endl;
                once = true;
            }
        }
    }
    else if (volume->render_mode() == VolumeRenderMode::VOXEL_CUBES) {
        // Render as instanced solid voxels
        auto shader = get_shader(ShaderType::VOXELS);
        if (shader && shader->is_valid()) {
            shader->use();
            
            // Ensure proper depth state for voxel rendering
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glDepthMask(GL_TRUE);
            glDisable(GL_BLEND); // Solid voxels don't need blending
            
            // Disable backface culling for voxel cubes to ensure all faces render
            glDisable(GL_CULL_FACE);
            
            // Set uniforms with visible colors
            shader->set_uniform("mvp_matrix", mvp_matrix());
            shader->set_uniform("model_matrix", volume->transform());
            shader->set_uniform("normal_matrix", glm::transpose(glm::inverse(volume->transform())));
            shader->set_uniform("model_color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // Bright red for instanced cubes
            shader->set_uniform("near_plane", DEFAULT_NEAR);
            shader->set_uniform("far_plane", DEFAULT_FAR);
            
            // Use volume's draw method
            volume->bind();
            draw_volume(*volume);
            volume->unbind();
            
            // Restore backface culling state
            glEnable(GL_CULL_FACE);
            
            shader->unuse();
        }
        else {
            std::cerr << "ERROR: Volume voxel instanced shader not found or invalid!" << std::endl;
        }
    }
}

void Renderer::render_checkers() const {
    auto checkers = scene_->checkers();
    if (!checkers || !checkers->is_ready_to_render()) { return; }

    // Use unified shader for checkers rendering
    auto shader = get_shader(ShaderType::LINES);
    if (!shader || !shader->is_valid()) { return; }

    shader->use();
    shader->set_uniform("mvp_matrix", mvp_matrix());
    shader->set_uniform("model_matrix", checkers->transform());
    shader->set_uniform("model_color", checkers->color());
    shader->set_uniform("depth_bias", 0.0f); // No depth bias for checkers (polygon offset is used instead)
    shader->set_uniform("use_chessboard_layout", true); // Enable chessboard color layout

    // Enable polygon offset so chessboard always renders over the floor
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(-1.0f, -1.0f); // Negative values pull chessboard closer to camera

    // Render all meshes
    const auto& meshes = checkers->meshes();
    for (const auto& mesh : meshes) {
        if (mesh) {
            mesh->bind();
            draw_mesh(*mesh);
            mesh->unbind();
        }
    }

    // Disable polygon offset after rendering chessboard
    glDisable(GL_POLYGON_OFFSET_FILL);

    shader->unuse();
}

void Renderer::render_frustums() const {
    const auto& frustums = scene_->frustums();
    
    // Use unified shader for frustum rendering
    auto shader = get_shader(ShaderType::LINES);
    if (!shader || !shader->is_valid()) { return; }
    
    shader->use();
    
    for (const auto& frustum : frustums) {
        if (!frustum || !frustum->is_visible() || !frustum->is_ready_to_render()) { continue; }

        // Set uniforms for this frustum
        shader->set_uniform("mvp_matrix", mvp_matrix());
        shader->set_uniform("model_matrix", frustum->transform());
        shader->set_uniform("model_color", frustum->color());
        shader->set_uniform("depth_bias", 0.0f); // No depth bias for frustums
        shader->set_uniform("use_chessboard_layout", false); // Use standard color layout
        
        // Render all meshes for this frustum
        const auto& meshes = frustum->meshes();
        for (const auto& mesh : meshes) {
            if (mesh) {
                mesh->bind();
                draw_mesh(*mesh);
                mesh->unbind();
            }
        }
    }
    
    shader->unuse();
}

void Renderer::render_image_overlay() {
    // Only render if overlay resources are initialized, we are in static view mode, and background overlay is enabled
    if (!overlay_resources_initialized_) { return; }
    if (!camera_->in_static_view()) { return; }
    if (!show_background_) { return; }
    
    auto shader = get_shader(ShaderType::OVERLAY);
    if (!shader) { return; }
    
    // Save current OpenGL state
    GLboolean depth_test_enabled = glIsEnabled(GL_DEPTH_TEST);
    GLboolean blend_enabled = glIsEnabled(GL_BLEND);
    
    // Disable depth testing for overlay rendering
    glDisable(GL_DEPTH_TEST);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    shader->use();
    
    // Calculate aspect ratio correction
    float scale_x, scale_y, offset_x, offset_y;
    calc_bg_transform(scale_x, scale_y, offset_x, offset_y);
    shader->set_uniform("scale", glm::vec2(scale_x, scale_y));
    shader->set_uniform("offset", glm::vec2(offset_x, offset_y));
    
    glBindVertexArray(overlay_vao_);
    
    // Render the background overlay
    if (background_texture_ && background_texture_->is_valid()) {
        background_texture_->bind(0);
        shader->set_uniform("image_texture", 0);
        shader->set_uniform("alpha", 1.0f); // Full opacity for background
        glDrawArrays(GL_TRIANGLES, 0, 6);
        background_texture_->unbind();
    }
    
    glBindVertexArray(0);
    shader->unuse();
    
    // Restore previous OpenGL state
    if (depth_test_enabled) { glEnable(GL_DEPTH_TEST); }
    if (!blend_enabled) { glDisable(GL_BLEND); }
}

void Renderer::draw_mesh(const Mesh& mesh) const {
    if (!mesh.indices().empty()) {
        glDrawElements(static_cast<GLenum>(mesh.primitive_type()), static_cast<GLsizei>(mesh.indices().size()), GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(static_cast<GLenum>(mesh.primitive_type()), 0, static_cast<GLsizei>(mesh.vertices().size()));
    }
}

void Renderer::draw_mesh_instanced(const Mesh& mesh, unsigned int instance_count) const {
    if (!mesh.indices().empty()) {
        glDrawElementsInstanced(static_cast<GLenum>(mesh.primitive_type()), 
            static_cast<GLsizei>(mesh.indices().size()), GL_UNSIGNED_INT, 0, instance_count);
    }
    else {
        glDrawArraysInstanced(static_cast<GLenum>(mesh.primitive_type()), 0, 
            static_cast<GLsizei>(mesh.vertices().size()), instance_count);
    }
}

void Renderer::draw_volume(const Volume& volume) const {
    switch (volume.render_mode()) {
        case VolumeRenderMode::POINT_CLOUD: {
            size_t rendered_count = volume.rendered_voxel_count();
            
            static bool once = false;
            if (!once) {
                once = true;
            }
            
            if (rendered_count > 0) {                
                // Enable point size modification in shaders
                glEnable(GL_PROGRAM_POINT_SIZE);
                
                // Disable blending to ensure proper depth buffer writes
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glDepthMask(GL_TRUE); // Critical: ensure depth writes are enabled for volume
                
                // Check for OpenGL errors before draw call
                GLenum error = glGetError();
                if (error != GL_NO_ERROR) {
                    std::cerr << "OpenGL error before draw: " << error << std::endl;
                }
                
                glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(rendered_count));
                
                // Check for OpenGL errors after draw call
                error = glGetError();
                if (error != GL_NO_ERROR) {
                    std::cerr << "OpenGL error after draw: " << error << std::endl;
                }
                
                // Restore OpenGL state
                glDisable(GL_BLEND);
                glDisable(GL_PROGRAM_POINT_SIZE);
            }
            else {
                std::cerr << "No points to render (rendered_count = 0)" << std::endl;
            }
            break;
        }

        case VolumeRenderMode::VOXEL_CUBES: {
            size_t instance_count = volume.rendered_voxel_count();
            if (instance_count > 0) {
                // Draw instanced cubes using indexed rendering (much more efficient)
                // 36 indices per cube (12 triangles × 3 indices each)
                glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, static_cast<GLsizei>(instance_count));
            }
            break;
        }
    }
}

void Renderer::draw_quad(const std::shared_ptr<Texture>& texture) const {
    if (!texture) { return; }

    // Use a simple shader for textured quad (create if needed)
    auto shader = get_shader(ShaderType::OVERLAY);
    if (!shader) { return; }

    shader->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture->id());
    shader->set_uniform("tex", 0);

    glBindVertexArray(overlay_vao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);
}
