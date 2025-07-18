#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include <GL/glew.h>
#include <GLFW/glfw3.h>


class Scene;
class Camera;
class Overlay;
class Renderer;


/**
 * @class Input
 * @brief Handles user input events and state for the application.
 */
class Input {
public: // Constructors
    /**
     * @brief Construct a new Input object.
     * @param scene Shared pointer to the scene.
     * @param renderer Shared pointer to the renderer.
     * @param camera Shared pointer to the camera.
     * @param overlay Shared pointer to the overlay.
     */
    Input(std::shared_ptr<Scene> scene, std::shared_ptr<Renderer> renderer, 
          std::shared_ptr<Camera> camera, std::shared_ptr<Overlay> overlay);

public: // Methods
    /**
     * @brief Handle keyboard input events.
     * @param window GLFW window pointer.
     * @param key Key code.
     * @param scancode Scan code.
     * @param action Key action.
     * @param mods Modifier keys.
     */
    void on_key(GLFWwindow* window, int key, int scancode, int action, int mods);

    /**
     * @brief Handle mouse button input events.
     * @param window GLFW window pointer.
     * @param button Mouse button.
     * @param action Button action.
     * @param mods Modifier keys.
     */
    void on_mouse_button(GLFWwindow* window, int button, int action, int mods);

    /**
     * @brief Handle mouse cursor position events.
     * @param window GLFW window pointer.
     * @param pos_x Cursor X position.
     * @param pos_y Cursor Y position.
     */
    void on_cursor_pos(GLFWwindow* window, double pos_x, double pos_y);
    
    /**
     * @brief Set the mouse position.
     * @param x Mouse X position.
     * @param y Mouse Y position.
     */
    void set_mouse(int x, int y) { mouse_x_ = x; mouse_y_ = y; }

    /** @brief Set left mouse button down state. */
    void set_lmb_down(bool down) { lmb_down_ = down; }

    /** @brief Set right mouse button down state. */
    void set_rmb_down(bool down) { rmb_down_ = down; }

public: // Getters

    /** @brief Get the mouse X position. */
    int mouse_x() const { return mouse_x_; }

    /** @brief Get the mouse Y position. */
    int mouse_y() const { return mouse_y_; }

    /** @brief Get left mouse button down state. */
    bool lmb_down() const { return lmb_down_; }

    /** @brief Get right mouse button down state. */
    bool rmb_down() const { return rmb_down_; }

private: // Variables
    int mouse_x_ = 0;
    int mouse_y_ = 0;
    bool lmb_down_ = false;
    bool rmb_down_ = false;

    std::shared_ptr<Scene> scene_;
    std::shared_ptr<Camera> camera_;
    std::shared_ptr<Overlay> overlay_;
    std::shared_ptr<Renderer> renderer_;
};
