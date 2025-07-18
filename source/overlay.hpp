#pragma once

#include <memory>
#include <string>
#include <functional>

#include <imgui.h>

#include "project.hpp"


class Scene;
class Camera;
class Renderer;
struct GLFWwindow;


constexpr const int DEFAULT_CAMERA_VIEW = -1;
constexpr const int DEFAULT_VOLUME_RENDER_MODE = 1;


/**
 * @class Overlay
 * @brief Manages the ImGui-based user interface and UI state.
 */
class Overlay {
public: // Constructors
    /**
     * @brief Construct a new Overlay object.
     * @param window GLFW window pointer.
     * @param scene Shared pointer to the scene.
     * @param renderer Shared pointer to the renderer.
     * @param camera Shared pointer to the camera.
     * @param project_load_cb Callback for project load.
     * @param project_close_cb Callback for project close.
     */
    Overlay(GLFWwindow* window, std::shared_ptr<Scene> scene, 
            std::shared_ptr<Renderer> renderer, std::shared_ptr<Camera> camera,
            std::function<void(std::shared_ptr<Project>)> project_load_cb, std::function<void()> project_close_cb);

    /** @brief Destructor. Cleans up ImGui resources. */
    ~Overlay();

public: // Methods
    /**
     * @brief Load a project and update UI state.
     * @param project Shared pointer to the project to load.
     */
    void load_project(std::shared_ptr<Project> project);

    /** @brief Unload the current project and reset UI state. */
    void unload_project();

    /**
     * @brief Set the current camera view in the UI.
     * @param view Camera view index.
     */
    void set_current_camera_view(int view);

    /** @brief Start a new frame for ImGui rendering. */
    void new_frame();

    /** @brief Render the ImGui interface. */
    void render();

    /** @brief End frame and render ImGui draw data. */
    void end_frame();

    /**
     * @brief Check if ImGui wants to capture mouse input.
     * @return True if ImGui wants to capture mouse.
     */
    bool wants_capture_mouse() const;

    /**
     * @brief Check if ImGui wants to capture keyboard input.
     * @return True if ImGui wants to capture keyboard.
     */
    bool wants_capture_keyboard() const;

    /** @brief Sync UI state with renderer. */
    void sync_with_renderer();

    /** @brief Toggle the visibility of the volume bounding box in UI. */
    void toggle_show_box() { show_box_ = !show_box_; }

    /** @brief Toggle the visibility of the ground floor grid in UI. */
    void toggle_show_floor() { show_floor_ = !show_floor_; }

    /** @brief Toggle the visibility of the world axes in UI. */
    void toggle_show_frame() { show_frame_ = !show_frame_; }

    /** @brief Toggle the visibility of the volume in UI. */
    void toggle_show_volume() { show_volume_ = !show_volume_; }

    /** @brief Toggle the visibility of the camera frustums in UI. */
    void toggle_show_frustums() { show_frustums_ = !show_frustums_; }

    /** @brief Toggle the visibility of the checker board in UI. */
    void toggle_show_chessboard() { show_checkers_ = !show_checkers_; }

    /**
     * @brief Show an error popup with the given message.
     * @param message Error message to display.
     */
    void show_error_popup(const std::string& message);

    /**
     * @brief Check if the error popup is open.
     * @return True if error popup is open.
     */
    bool is_error_popup_open() const { return error_popup_open_; }

private: // Static functions
    /**
     * @brief Helper for ImGui::BeginDisabled if condition is true.
     * @param condition Condition to disable UI.
     */
    static inline void BeginDisabledIf(bool condition) {
        if (condition) { ImGui::BeginDisabled(true); }
    }
    /**
     * @brief Helper for ImGui::EndDisabled if condition is true.
     * @param condition Condition to enable UI.
     */
    static inline void EndDisabledIf(bool condition) {
        if (condition) { ImGui::EndDisabled(); }
    }

private: // Methods
    /** @brief Render the main control panel UI. */
    void render_menu();

    /** @brief Render the main menu bar. */
    void render_main_menu_bar();

    /** @brief Render the file menu. */
    void render_file_menu();
    
    /** @brief Render the project name in the menu bar. */
    void render_project_name(float menu_bar_width);

    /** @brief Render the help tooltip in the menu bar. */
    void render_help_tooltip(float menu_bar_width, float help_width);

    /** @brief Render the control window. */
    void render_control_window();

    /** @brief Render the camera selection UI. */
    void render_camera_selection();

    /** @brief Render the background toggle UI. */
    void render_background_toggle();

    /** @brief Render the volume render mode UI. */
    void render_volume_render_mode();

    /** @brief Render the scene models UI. */
    void render_scene_models();

    /**
     * @brief Reset UI state when project is loaded or unloaded.
     * @param project_loaded True if project is loaded.
     */
    void reset_ui_state(bool project_loaded);

    /**
     * @brief Set the error popup message.
     * @param message Error message to display.
     */
    void set_error_popup(const std::string& message);

private: // Structs
    struct Toggleable {
        const char* label;
        bool* state;
        std::function<void()> toggle_func;
    };

private: // Variables
    GLFWwindow* window_;
    std::shared_ptr<Project> project_;

    std::shared_ptr<Scene> scene_;
    std::shared_ptr<Camera> camera_;
    std::shared_ptr<Renderer> renderer_;

    std::function<void(std::shared_ptr<Project> project)> project_open_callback_;
    std::function<void()> project_close_callback_;

    // UI state
    bool show_box_ = true;
    bool show_floor_ = true;
    bool show_frame_ = true;
    bool show_volume_ = false;
    bool show_frustums_ = true;
    bool show_checkers_ = true;

    int active_camera_view_ = DEFAULT_CAMERA_VIEW;
    int volume_render_mode_ = DEFAULT_VOLUME_RENDER_MODE;

    // Background overlay state
    bool show_background_ = false;

    // Deferred project actions to avoid ImGui state invalidation
    bool pending_project_load_ = false;
    bool pending_project_close_ = false;
    std::string pending_project_path_;

    // Error popup state
    bool error_popup_open_ = false;
    std::string error_popup_message_;

    // Toggle items for scene models
    std::vector<Toggleable> toggle_items_;
};
