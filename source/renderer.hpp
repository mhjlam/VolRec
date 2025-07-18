#pragma once

#include <array>
#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

#include "camera.hpp"
#include "global.hpp"

#include "render/shader.hpp"
#include "render/texture.hpp"
#include "render/framebuffer.hpp"


class Mesh;
class Scene;
class Volume;


/**
 * @class Renderer
 * @brief Handles all rendering operations for the application, including scene, overlays, and camera interaction.
 */
class Renderer {
public: // Constructors
    /**
     * @brief Construct a Renderer object.
     * @param width Window width.
     * @param height Window height.
     * @param scene Shared pointer to the scene.
     * @param camera Shared pointer to the camera.
     */
    Renderer(int width, int height, std::shared_ptr<Scene> scene, std::shared_ptr<Camera> camera);


public: // Methods
    /**
     * @brief Load a project into the renderer.
     * @param project Shared pointer to the project.
     */
    void load_project(std::shared_ptr<Project> project);

    /** @brief Unload the current project from the renderer. */
    void unload_project();

    /**
     * @brief Resize the renderer window.
     * @param width New window width.
     * @param height New window height.
     */
    void resize(int width, int height);

    /** @brief Render the scene. */
    void render();

    /** @brief Toggle between point cloud and solid voxel render modes. */
    void toggle_volume_render_mode();

    /** @brief Toggle volume bounding box visibility. */
    void toggle_box() { show_box_ = !show_box_; }

    /** @brief Toggle world axes visibility. */
    void toggle_frame() { show_frame_ = !show_frame_; }

    /** @brief Toggle ground floor grid visibility. */
    void toggle_floor() { show_floor_ = !show_floor_; }

    /** @brief Toggle volume visibility. */
    void toggle_volume() { show_volume_ = !show_volume_; }

    /** @brief Toggle checker board visibility. */
    void toggle_checkers() { show_checkers_ = !show_checkers_; }

    /** @brief Toggle camera frustums visibility. */
    void toggle_frustums() { show_frustums_ = !show_frustums_; }

    /** @brief Toggle background overlay visibility. */
    void toggle_background() { show_background_ = !show_background_; };

public: // Getters
    /** @brief Returns true if volume bounding box is visible. */
    bool get_show_box() const { return show_box_; }
    /** @brief Returns true if world axes are visible. */

    bool get_show_frame() const { return show_frame_; }
    /** @brief Returns true if ground floor grid is visible. */

    bool get_show_floor() const { return show_floor_; }
    /** @brief Returns true if volume is visible. */

    bool get_show_volume() const { return show_volume_; }
    /** @brief Returns true if checker board is visible. */

    bool get_show_checkers() const { return show_checkers_; }
    /** @brief Returns true if camera frustums are visible. */
    bool get_show_frustums() const { return show_frustums_; }

    /** @brief Returns true if background overlay is visible. */
    bool get_show_background() const { return show_background_; }

    /**
     * @brief Retrieve a shader by its type.
     * @param type The type of shader to retrieve.
     * @return Shared pointer to the requested shader.
     */
    std::shared_ptr<Shader> get_shader(ShaderType type) const;

    /**
     * @brief Compute the Model-View-Projection (MVP) matrix.
     * @param model_matrix The model transformation matrix (defaults to identity).
     * @return The computed MVP matrix.
     */
    glm::mat4 mvp_matrix(const glm::mat4& model_matrix = glm::mat4(1.0f)) const;

private: // Methods
    /**
     * @brief Initialize all required shaders for rendering.
     * @return True if initialization was successful, false otherwise.
     */
    bool initialize_shaders();

    /** @brief Initialize textures required for overlays and rendering. */
    void initialize_textures();

    /** @brief Update overlay textures (background/foreground) as needed. */
    void update_overlay_textures();

    /**
     * @brief Calculate transformation parameters for the background overlay.
     * @param scale_x Output scale factor in X.
     * @param scale_y Output scale factor in Y.
     * @param offset_x Output offset in X.
     * @param offset_y Output offset in Y.
     */
    void calc_bg_transform(float& scale_x, float& scale_y, float& offset_x, float& offset_y) const;

    /** @brief Render the volume bounding box. */
    void render_box() const;

    /** @brief Render the ground floor grid. */
    void render_floor() const;

    /** @brief Render the world axes. */
    void render_frame() const;

    /** @brief Render the volume. */
    void render_volume() const;

    /** @brief Render the checker board. */
    void render_checkers() const;

    /** @brief Render the camera frustums. */
    void render_frustums() const;

    /** @brief Render the image overlay on top of the scene. */
    void render_image_overlay();

    /**
     * @brief Draw a mesh using OpenGL.
     * @param mesh The mesh to draw.
     */
    void draw_mesh(const Mesh& mesh) const;

    /**
     * @brief Draw multiple instances of a mesh using OpenGL instancing.
     * @param mesh The mesh to draw.
     * @param instance_count Number of instances to render.
     */
    void draw_mesh_instanced(const Mesh& mesh, unsigned int instance_count) const;

    /**
     * @brief Draw a volume using OpenGL.
     * @param volume The volume to draw.
     */
    void draw_volume(const Volume& volume) const;

    /**
     * @brief Draw a textured quad.
     * @param texture Shared pointer to the texture to use.
     */
    void draw_quad(const std::shared_ptr<Texture>& texture) const;

private: // Variables
    bool show_box_;							                // Volume bounding box
    bool show_frame_;						                // World axes
    bool show_floor_;						                // Ground floor grid
    bool show_volume_;                                      // Volume
    bool show_frustums_;					                // Camera frustums
    bool show_checkers_;                                    // Checker board
    bool show_background_ = false;                          // Background overlay disabled by default

    float window_width_ = static_cast<float>(VIEW_WIDTH);   // Window width
    float window_height_ = static_cast<float>(VIEW_HEIGHT); // Window height
    float aspect_ratio_ = 1.0f;                             // Aspect ratio for the projection matrix

    std::shared_ptr<Project> project_;

    std::shared_ptr<Scene> scene_;                          // Reference to scene (not owned)
    std::shared_ptr<Camera> camera_;                        // Single interactive camera
    std::unique_ptr<Framebuffer> overlay_buffer_;           // Framebuffer for image overlays
    std::unordered_map<ShaderType, std::shared_ptr<Shader>> shaders_;
    
    // Image overlay system
    GLuint overlay_vao_ = 0;
    GLuint overlay_vbo_ = 0;
    std::unique_ptr<class Texture> background_texture_;
    std::unique_ptr<class Texture> foreground_texture_;
    bool overlay_resources_initialized_ = false;
};
