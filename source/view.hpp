#pragma once

#include <filesystem>

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>


constexpr const float DEFAULT_CAM_DIST = 2000.0f;

constexpr const float DEFAULT_CAM_X = DEFAULT_CAM_DIST * 0.75f;
constexpr const float DEFAULT_CAM_Y = DEFAULT_CAM_DIST * 0.75f;
constexpr const float DEFAULT_CAM_Z = DEFAULT_CAM_DIST * 0.75f;

constexpr const glm::vec3 DEFAULT_EYE(DEFAULT_CAM_X, DEFAULT_CAM_Y, DEFAULT_CAM_Z);
constexpr const glm::vec3 DEFAULT_AT(0.0f, 0.0f, 0.0f);
constexpr const glm::vec3 DEFAULT_UP(0.0f, 1.0f, 0.0f);

constexpr const float DEFAULT_FOV = 60.0f;
constexpr const float DEFAULT_NEAR = 0.1f;
constexpr const float DEFAULT_FAR = 10000.0f;


/**
 * @struct View
 * @brief Stores camera parameters, calibration data, and image resources for a single view.
 *
 * This struct encapsulates all data needed to represent a camera view in the application, including:
 * - OpenGL rendering parameters (position, orientation, projection)
 * - OpenCV calibration matrices (intrinsic, distortion, rotation, translation)
 * - Image resources for overlays (background, foreground, mask)
 * - File paths for image and calibration data
 *
 * Members:
 * - eye, at, up: Camera position and orientation vectors
 * - fov, near, far: Field of view and clipping planes
 * - forward, upward, right: Derived orientation vectors
 * - proj: Projection matrix
 * - intrinsic, distortion, rvec, tvec, tvec_proj, focal_length, principal_point: Calibration matrices
 * - fg, bg, mask: Foreground, background, and mask images
 * - bg_path, fg_path, cb_path: Paths to image and calibration files
 */
struct View {
    // OpenGL rendering data
    glm::vec3 eye = DEFAULT_EYE;                                    // Camera position
    glm::vec3 at  = DEFAULT_AT;                                     // Look-at center
    glm::vec3 up  = DEFAULT_UP;                                     // Up direction
    
    float fov = DEFAULT_FOV;                                        // Horizontal field of view in degrees
    float near = DEFAULT_NEAR;                                      // Near clipping plane
    float far = DEFAULT_FAR;                                        // Far clipping plane

    glm::vec3 forward = glm::normalize(DEFAULT_AT - DEFAULT_EYE);   // Forward direction (-Z in OpenGL)
    glm::vec3 upward = DEFAULT_UP;                                  // Up direction (+Y in OpenGL)
    glm::vec3 right = glm::normalize(glm::cross(forward, upward));  // Right direction (+X in OpenGL)

    glm::mat4 proj = glm::mat4(1.0f);                               // Projection matrix (identity by default)

    // OpenCV calibration data
    cv::Mat intrinsic = cv::Mat::zeros(3, 3, CV_64F);               // Intrinsic camera matrix as 3x3 double matrix
    cv::Mat distortion = cv::Mat::zeros(1, 5, CV_64F);              // Distortion coefficients as 1x5 double matrix
    cv::Mat rvec = cv::Mat::zeros(3, 1, CV_64F);                    // Rotation vector (OpenCV)
    cv::Mat tvec = cv::Mat::zeros(3, 1, CV_64F);                    // Translation vector (OpenCV)
    cv::Mat tvec_proj = cv::Mat::zeros(3, 1, CV_64F);               // Precomputed translation vector for projection
    cv::Mat focal_length = cv::Mat::zeros(2, 1, CV_64F);            // Focal length (fx, fy) as 2x1 double matrix
    cv::Mat principal_point = cv::Mat::zeros(2, 1, CV_64F);         // Principal point (cx, cy) as 2x1 double matrix
    
    // Background, foreground, mask images
    cv::Mat fg;                                                     // Foreground image
    cv::Mat bg;                                                     // Background image
    cv::Mat mask;                                                   // Mask image

    // Paths for background, foreground, and calibration files
    std::filesystem::path bg_path;                                  // Background image path
    std::filesystem::path fg_path;                                  // Foreground image path
    std::filesystem::path cb_path;                                  // Calibration file path
};
