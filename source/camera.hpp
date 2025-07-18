#pragma once

#include <array>
#include <memory>

#include <glm/glm.hpp>

#include "view.hpp"
#include "project.hpp"


/**
 * @class Camera
 * @brief Manages camera state, calibration, and view switching.
 */
class Camera {
public: // Constructors
    /** @brief Construct a new Camera object. */
    Camera();

public: // Methods
    /**
     * @brief Load a project and initialize camera parameters.
     * @param project Shared pointer to the project to load.
     */
    void load_project(std::shared_ptr<Project> project);

    /** @brief Unload the current project and reset camera state. */
    void unload_project();

    /**
     * @brief Switch to static view mode for the given view index.
     * @param view_index Index of the static view to activate.
     */
    void set_view(int view_index);

    /**
     * @brief Rotate the camera by the given x and y offsets.
     * @param x Horizontal rotation offset.
     * @param y Vertical rotation offset.
     */
    void rotate(int x, int y);

    /**
     * @brief Zoom the camera by the given delta.
     * @param delta Zoom amount.
     */
    void zoom(int delta);

    /**
     * @brief Resize the camera viewport.
     * @param width New window width.
     * @param height New window height.
     */
    void resize(int width, int height);

public: // Methods
    /**
     * @brief Get the current camera view (OpenGL coordinate system).
     * @return Reference to the current View.
     */
    const View& current_view() const { return current_view_; }

    /** @brief Get the camera eye position. */
    glm::vec3 eye() const { return current_view_.eye; }

    /** @brief Get the camera look-at center. */
    glm::vec3 at() const { return current_view_.at; }

    /** @brief Get the camera up direction. */
    glm::vec3 up() const { return current_view_.up; }

    /** @brief Get the camera field of view. */
    float fov() const { return current_view_.fov; }

    /** @brief Get the camera projection matrix. */
    glm::mat4 proj_matrix() const { return current_view_.proj; }

    /**
     * @brief Check if currently in static view mode.
     * @return True if in static view mode.
     */
    bool in_static_view() const { return current_view_index_ >= 0; }

    /**
     * @brief Get the current static view index.
     * @return Index of the current static view.
     */
    int get_current_view_index() const { return current_view_index_; }

private: // Methods
    /**
     * @brief Create a free-form camera view.
     * @param eye Camera position.
     * @return View object representing the free-form view.
     */
    View freeform_view(glm::vec3 eye = DEFAULT_EYE);

    /** @brief Calibrate the camera using chessboard images from multiple views. */
    void run_calibration();

    /** @brief Load camera parameters from YAML files for each view. */
    bool read_calibration();

    /** @brief Save camera parameters to YAML files for each view that needs calibration. */
    void write_calibration();

    /**
     * @brief Compute the mask and OpenGL transformation for a given view.
     * @param view View to calibrate.
     */
    void calibrate_view(View& view) const;

    /**
     * @brief Calculate the mask from foreground and background images.
     * @param fg_img Foreground image.
     * @param bg_img Background image.
     * @return Mask image.
     */
    cv::Mat calc_mask(const cv::Mat& fg_img, const cv::Mat& bg_img) const;

    /**
     * @brief Calculate the field of view based on focal length and view width.
     * @param focal_length Focal length.
     * @param view_width View width.
     * @return Field of view in degrees.
     */
    float calc_fov(float focal_length, float view_width) const;

    /**
     * @brief Calculate the projection matrix based on OpenCV intrinsics.
     * @param intrinsic Intrinsic camera matrix.
     * @param width View width.
     * @param height View height.
     * @param near_plane Near clipping plane.
     * @param far_plane Far clipping plane.
     * @return Projection matrix.
     */
    glm::mat4 calc_proj(cv::Mat intrinsic, float width, float height, 
        float near_plane = DEFAULT_NEAR, float far_plane = DEFAULT_FAR) const;

private: // Variables
    int current_view_index_ = -1;               // Current static view index (-1 if not in static view)
    std::shared_ptr<Project> project_;          // Shared pointer to the current project

    View freeform_view_;                        // Free-form camera state (used when not in static view mode)
    View& current_view_ = freeform_view_;       // Current camera state
};
