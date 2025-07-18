#include "input.hpp"

#include <format>
#include <limits>
#include <iostream>

#include "scene.hpp"
#include "camera.hpp"
#include "overlay.hpp"
#include "renderer.hpp"


/* Constructors */

Input::Input(std::shared_ptr<Scene> scene, std::shared_ptr<Renderer> renderer, 
             std::shared_ptr<Camera> camera, std::shared_ptr<Overlay> overlay)
: scene_{scene}
, camera_{camera}
, overlay_{overlay} 
, renderer_{renderer} {}


/* Public methods */

void Input::on_key(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Define important shortcuts that should work even when ImGui is active
    bool is_passthrough_key = false;
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_1:
            case GLFW_KEY_2:
            case GLFW_KEY_3:
            case GLFW_KEY_4:
            case GLFW_KEY_A:  // Toggle world axes
            case GLFW_KEY_B:  // Toggle volume box
            case GLFW_KEY_C:  // Toggle camera wireframes
            case GLFW_KEY_F:  // Toggle floor grid
            case GLFW_KEY_G:  // Toggle background window
            case GLFW_KEY_H:  // Toggle foreground window
            case GLFW_KEY_V:  // Volume render mode toggle
            case GLFW_KEY_ESCAPE:
                is_passthrough_key = true;
                break;
        }
    }
    
    // Check if ImGui wants to capture keyboard input, but allow passthrough keys
    if (overlay_ && overlay_->wants_capture_keyboard() && !is_passthrough_key) {
        return;
    }

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_B: 
                if (renderer_) { renderer_->toggle_box(); } 
                if (overlay_) { overlay_->toggle_show_box(); }
                break;

            case GLFW_KEY_A: 
                if (renderer_) { renderer_->toggle_frame(); } 
                if (overlay_) { overlay_->toggle_show_frame(); }
                break;

            case GLFW_KEY_F: 
                if (renderer_) { renderer_->toggle_floor(); } 
                if (overlay_) { overlay_->toggle_show_floor(); }
                break;

            case GLFW_KEY_C: 
                if (renderer_) { renderer_->toggle_frustums(); } 
                if (overlay_) { overlay_->toggle_show_frustums(); }
                break;

            case GLFW_KEY_V: 
                if (renderer_) { renderer_->toggle_volume(); }  // Toggle volume visibility, not render mode
                if (overlay_) { overlay_->toggle_show_volume(); }
                break;

            case GLFW_KEY_G: 
                if (renderer_) { renderer_->toggle_background(); }  // Toggle background overlay (only works in static views)
                if (overlay_) { overlay_->sync_with_renderer(); }
                break;

            case GLFW_KEY_1: 
                if (camera_) { camera_->set_view(0); }
                if (overlay_) { overlay_->set_current_camera_view(0); }
                break;

            case GLFW_KEY_2: 
                if (camera_) { camera_->set_view(1); }
                if (overlay_) { overlay_->set_current_camera_view(1); }
                break;

            case GLFW_KEY_3: 
                if (camera_) { camera_->set_view(2); }
                if (overlay_) { overlay_->set_current_camera_view(2); }
                break;

            case GLFW_KEY_4: 
                if (camera_) { camera_->set_view(3); }
                if (overlay_) { overlay_->set_current_camera_view(3); }
                break;
            
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window ? window : glfwGetCurrentContext(), GLFW_TRUE); 
                break;
        }
    }
}

void Input::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
    // Check if ImGui wants to capture mouse input
    if (overlay_ && overlay_->wants_capture_mouse()) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        lmb_down_ = (action == GLFW_PRESS);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        rmb_down_ = (action == GLFW_PRESS);
    }
}

void Input::on_cursor_pos(GLFWwindow* window, double pos_x, double pos_y) {
    int x = static_cast<int>(pos_x);
    int y = static_cast<int>(pos_y);

    // Check if ImGui wants to capture mouse input
    if (overlay_ && overlay_->wants_capture_mouse()) {
        mouse_x_ = x;
        mouse_y_ = y;
        return;
    }

    if (lmb_down_) {
        overlay_->set_current_camera_view(-1);
        camera_->rotate(x - mouse_x_, y - mouse_y_);
    }

    if (rmb_down_) {
        overlay_->set_current_camera_view(-1);
        camera_->zoom(-(y - mouse_y_) / 2);
    }

    mouse_x_ = x;
    mouse_y_ = y;
}
