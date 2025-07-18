#include "camera.hpp"

#include <limits>
#include <numbers>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <opencv2/opencv.hpp>

#include "global.hpp"


// Mask thresholding values
constexpr const int THRESHOLD_H = 20;
constexpr const int THRESHOLD_S = 20;
constexpr const int THRESHOLD_V = 40;

// Camera configuration constants
constexpr const float FRUSTUM_DISTANCE = 100.0f;
constexpr const float LOOK_AT_DISTANCE = 1000.0f;

// Camera rotation constants
constexpr const float ROTATION_SPEED_CLOSE = 100.0f;
constexpr const float ROTATION_SPEED_FAR = 200.0f;
constexpr const float ROTATION_DISTANCE_THRESHOLD = 1000.0f;

// Camera zoom constants
constexpr const float ZOOM_STEP_SIZE = 100.0f;
constexpr const float MIN_ZOOM_DISTANCE = 200.0f;   // Minimum distance from center
constexpr const float MAX_ZOOM_DISTANCE = 15000.0f; // Maximum distance from center


/* Constructors */

Camera::Camera()
: current_view_index_{-1}
, project_{std::make_shared<Project>()}
, freeform_view_{}
, current_view_{freeform_view_}
{
    freeform_view_ = freeform_view();
}


/* Public methods */

void Camera::load_project(std::shared_ptr<Project> project) {
    project_ = project;

    if (project_->needs_calibration) {
        run_calibration();
        write_calibration();
    }
    else {
        read_calibration();
    }

    // Ensure all views have valid calibration data
    for (auto& view : project_->views) {
        calibrate_view(view);
    }

    // Mark the project as initialized
    project_->empty = false;
    project_->initialized = true;
    project_->needs_calibration = false;

    // Set freeform view to first camera's position
    if (project_->views.size() > 0) {
        freeform_view_ = freeform_view(project_->views.front().eye);
    }

    // Start in freeform mode
    current_view_index_ = -1;
    current_view_ = freeform_view_;
}

void Camera::unload_project() {
    project_ = std::make_shared<Project>();

    // Set current view to freeform view
    current_view_index_ = -1;
    current_view_ = freeform_view_;
}

void Camera::set_view(int view_index) {
    // Save the current freeform view before switching
    if (current_view_index_ == -1) {
        freeform_view_ = current_view_;
    }
    
    // Switch to the specified static view
    if (view_index >= 0 && view_index < project_->views.size()) {
        current_view_index_ = view_index;
        current_view_ = project_->views[view_index];
    }
}

void Camera::rotate(int x, int y) {
    if (x == 0 && y == 0) { 
        return;
    }

    // Restore freeform mode on rotation interaction
    if (current_view_index_ >= 0) {
        current_view_index_ = -1;
        freeform_view_ = freeform_view(current_view_.eye);
        current_view_ = freeform_view_;
    }

    // Calculate vector from look-at point to eye (camera position)
    glm::vec3 offset = current_view_.eye - current_view_.at;
    float radius = glm::length(offset);
    if (radius < EPSILON) {
        std::cerr << "Error: Camera radius too small!" << std::endl;
        return;
    }

    // Convert to spherical coordinates
    float azimuth = std::atan2(offset.z, offset.x);
    float elevation = std::asin(glm::clamp(offset.y / radius, -1.0f, 1.0f));

    float speed = (radius > ROTATION_DISTANCE_THRESHOLD) ? ROTATION_SPEED_FAR : ROTATION_SPEED_CLOSE;
    azimuth += static_cast<float>(x) / speed;
    elevation += static_cast<float>(y) / speed;
    elevation = std::clamp(elevation, -HALF_PI + EPSILON, HALF_PI - EPSILON);

    // Convert back to cartesian coordinates
    float cos_elev = std::cos(elevation);
    offset = radius * glm::vec3(
        std::cos(azimuth) * cos_elev,
        std::sin(elevation),
        std::sin(azimuth) * cos_elev
    );

    current_view_.eye = current_view_.at + offset;
}

void Camera::zoom(int delta) {
    if (delta == 0) { 
        return;
    }

    // Restore freeform mode on zoom interaction
    if (current_view_index_ >= 0) {
        current_view_index_ = -1;
        freeform_view_ = freeform_view(current_view_.eye);
        current_view_ = freeform_view_;
    }

    float mag = glm::length(current_view_.eye);
    if (mag < EPSILON) { return; }
    float new_mag = glm::clamp(mag - ZOOM_STEP_SIZE * glm::sign(float(delta)), MIN_ZOOM_DISTANCE, MAX_ZOOM_DISTANCE);
    current_view_.eye *= new_mag / mag;
}

void Camera::resize(int width, int height) {
    // Resize the freeform view's projection matrix
    float width_f = static_cast<float>(width);
    float height_f = static_cast<float>(height);

    freeform_view_.fov = calc_fov(DEFAULT_FOV, width_f);
    freeform_view_.proj = glm::perspective(glm::radians(DEFAULT_FOV), width_f / height_f, DEFAULT_NEAR, DEFAULT_FAR);

    // Resize the field of view and projection matrix based on new window size for static views
    for (auto& view : project_->views) {
        view.fov = calc_fov(static_cast<float>(view.intrinsic.at<double>(0, 0)), width_f);
        view.proj = calc_proj(view.intrinsic, width_f, height_f, DEFAULT_NEAR, DEFAULT_FAR);
    }
}


/* Private methods */

View Camera::freeform_view(glm::vec3 eye) {
    View freeform;
    freeform.eye = eye;
    freeform.at = DEFAULT_AT;
    freeform.up = DEFAULT_UP;
    freeform.fov = calc_fov(DEFAULT_FOV, VIEW_WIDTH);
    freeform.proj = glm::perspective(glm::radians(DEFAULT_FOV), VIEW_ASPECT, DEFAULT_NEAR, DEFAULT_FAR);
    return freeform;
}

void Camera::run_calibration() {
    auto cols = project_->chess_cols;
    auto rows = project_->chess_rows;

    std::vector<std::vector<cv::Point2f>> image_points;   // 2D points in image plane
    std::vector<std::vector<cv::Point3f>> object_points;  // 3D points in real world space
    std::vector<cv::Mat> images;                          // Images used for calibration

    // Prepare a single set of 3D object points for the chessboard pattern
    std::vector<cv::Point3f> obj_points;
    for (int i = 0; i < cols * rows; ++i) {
        obj_points.emplace_back(
            (i / rows) * (project_->square_size + CHESS_PADDING),
            (i % rows) * (project_->square_size + CHESS_PADDING),
            0.0f
        );
    }

    // Loop through each view to find chessboard corners
    for (auto& view : project_->views) {
        cv::Mat img = cv::imread(view.bg_path.string(), cv::IMREAD_GRAYSCALE);

        std::vector<cv::Point2f> corners;
        int find_flags = cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_NORMALIZE_IMAGE;

        // Try to find chessboard corners in the image
        if (cv::findChessboardCorners(img, cv::Size(rows, cols), corners, find_flags)) {
            // Refine corner locations
            cv::TermCriteria term_criteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 30, 0.1);
            cv::cornerSubPix(img, corners, cv::Size(11, 11), cv::Size(-1, -1), term_criteria);
            image_points.push_back(corners);
            object_points.push_back(obj_points);
            images.push_back(img);
        }
    }

    // Preview the detected chessboard corners and user confirmation
    for (size_t i = 0; i < images.size(); ++i) {
        cv::Mat preview;
        cv::cvtColor(images[i], preview, cv::COLOR_GRAY2BGR);
        cv::drawChessboardCorners(preview, cv::Size(rows, cols), image_points[i], true);

        // Draw instruction text at the top, centered, small
        std::string instruction = "Press any key to continue or ESC to abort";
        int font_face = cv::FONT_HERSHEY_SIMPLEX;
        double font_scale = 0.5;
        int thickness = 1;
        int baseline = 0;
        cv::Size text_size = cv::getTextSize(instruction, font_face, font_scale, thickness, &baseline);
        int x = (preview.cols - text_size.width) / 2;
        int y = text_size.height + 10; // 10 pixels from top
        cv::putText(preview, instruction, cv::Point(x, y), font_face, font_scale, cv::Scalar(0, 255, 255), thickness, cv::LINE_AA);

        std::string win_name = std::format("Calibration: View {}", i + 1);
        cv::imshow(win_name, preview);
        int key = cv::waitKey(0);
        cv::destroyWindow(win_name);

        if (key == 27) { // ESC
            throw std::runtime_error("Calibration aborted by user.");
        }
    }

    // Check if all views have valid chessboard detections
    if (images.size() < project_->views.size() * 0.75) {
        throw std::runtime_error("Calibration failed: not enough chessboards detected (need at least 75% of views).");
    }

    // Initialize camera matrix and distortion coefficients
    cv::Mat camera_matrix = cv::Mat::eye(3, 3, CV_64F);
    cv::Mat distortion_coeffs = cv::Mat::zeros(1, 5, CV_64F);
    std::vector<cv::Mat> rvecs, tvecs;

    // Run camera calibration
    cv::calibrateCamera(object_points, image_points, images[0].size(), camera_matrix, distortion_coeffs, rvecs, tvecs);

    // Save calibration results for each view
    for (size_t i = 0; i < images.size(); ++i) {
        project_->views[i].intrinsic = camera_matrix;
        project_->views[i].distortion = distortion_coeffs;
        project_->views[i].rvec = rvecs[i];
        project_->views[i].tvec = tvecs[i];
    }
}

bool Camera::read_calibration() {
    bool all_loaded = true;

    for (auto& view : project_->views) {
        if (view.cb_path.empty()) { 
            all_loaded = false;
            continue;
        }
        
        if (!std::filesystem::exists(view.cb_path)) { 
            all_loaded = false;
            continue;
        }

        cv::FileStorage file_storage(view.cb_path.string(), cv::FileStorage::READ);
        if (!file_storage.isOpened()) { 
            all_loaded = false;
            continue;
        }

        // Read calibration parameters from file
        file_storage["camera_matrix"] >> view.intrinsic;
        file_storage["dist_coeffs"] >> view.distortion;
        file_storage["rvec"] >> view.rvec;
        file_storage["tvec"] >> view.tvec;
    }

    return all_loaded;
}

void Camera::write_calibration() {
    for (auto& view : project_->views) {
        cv::FileStorage file_storage(view.cb_path.string(), cv::FileStorage::WRITE);
        file_storage << "camera_matrix" << view.intrinsic;
        file_storage << "dist_coeffs" << view.distortion;
        file_storage << "rvec" << view.rvec;
        file_storage << "tvec" << view.tvec;
        file_storage.release();
    }
}

void Camera::calibrate_view(View& view) const {
    float view_width = static_cast<float>(VIEW_WIDTH);
    float view_height = static_cast<float>(VIEW_HEIGHT);

    // Ensure matrices use double precision
    if (view.intrinsic.type() != CV_64F) { 
        view.intrinsic.convertTo(view.intrinsic, CV_64F);
    }
    if (view.distortion.type() != CV_64F) { 
        view.distortion.convertTo(view.distortion, CV_64F);
    }
    if (view.rvec.type() != CV_64F) { 
        view.rvec.convertTo(view.rvec, CV_64F);
    }
    if (view.tvec.type() != CV_64F) { 
        view.tvec.convertTo(view.tvec, CV_64F);
    }

    // Extract focal length, principal point, and calibration resolution
    view.focal_length = view.intrinsic(cv::Range(0,2), cv::Range(0,1)).clone();
    view.principal_point = view.intrinsic(cv::Range(0,2), cv::Range(2,3)).clone();

    // Compute the mask for the foreground image based on the background image
    view.mask = calc_mask(view.fg, view.bg);

    // Compute field of view
    view.fov = calc_fov(static_cast<float>(view.intrinsic.at<double>(0, 0)), view_width);

    // Compute projection matrix based on OpenCV intrinsics
    view.proj = calc_proj(view.intrinsic, view_width, view_height, DEFAULT_NEAR, DEFAULT_FAR);

    // Compute camera center in OpenCV world coordinates
    cv::Mat rotation, center;
    cv::Rodrigues(view.rvec, rotation);
    center = -rotation.t() * view.tvec;
    view.tvec_proj = -rotation * center;

    // Convert camera center and rotation to float for OpenGL compatibility
    center.convertTo(center, CV_32F);
    rotation.convertTo(rotation, CV_32F);

    // OpenGL: X=right, Y=up, Z=back; map (x,y,z) -> (x,z,-y)
    view.right = glm::vec3(rotation.at<float>(0,0), rotation.at<float>(0,2), -rotation.at<float>(0,1));
    view.upward = glm::vec3(-rotation.at<float>(1,0), -rotation.at<float>(1,2), rotation.at<float>(1,1));
    view.forward = glm::vec3(-rotation.at<float>(2,0), -rotation.at<float>(2,2), rotation.at<float>(2,1));

    // Position in OpenGL coordinates (float)
    view.eye = glm::vec3(center.at<float>(0), center.at<float>(2), -center.at<float>(1));
    view.at = view.eye - view.forward * LOOK_AT_DISTANCE;
    view.up = view.upward;

    // If position is (close to) zero, use a fallback state
    if (glm::length(view.eye) < EPSILON) {
        view.eye = DEFAULT_EYE;
    }
}

cv::Mat Camera::calc_mask(const cv::Mat& fg_img, const cv::Mat& bg_img) const {
    cv::Mat mask;
    cv::Mat fg_hsv, bg_hsv;
    cv::Mat diff_h, diff_s, diff_v;
    cv::Mat mask_h, mask_s, mask_v;
    std::vector<cv::Mat> fg_channels, bg_channels;

    // Convert images to HSV
    cv::cvtColor(fg_img, fg_hsv, cv::COLOR_BGR2HSV);
    cv::cvtColor(bg_img, bg_hsv, cv::COLOR_BGR2HSV);

    // Split HSV channels
    cv::split(fg_hsv, fg_channels);
    cv::split(bg_hsv, bg_channels);

    // Compute absolute differences for each channel
    cv::absdiff(fg_channels[0], bg_channels[0], diff_h);
    cv::absdiff(fg_channels[1], bg_channels[1], diff_s);
    cv::absdiff(fg_channels[2], bg_channels[2], diff_v);

    // Threshold each channel
    double maxval = static_cast<double>(std::numeric_limits<uint8_t>::max());
    cv::threshold(diff_h, mask_h, THRESHOLD_H, maxval, cv::THRESH_BINARY);
    cv::threshold(diff_s, mask_s, THRESHOLD_S, maxval, cv::THRESH_BINARY);
    cv::threshold(diff_v, mask_v, THRESHOLD_V, maxval, cv::THRESH_BINARY);

    // Combine masks: (H AND S) OR V
    cv::bitwise_and(mask_h, mask_s, mask);
    cv::bitwise_or(mask, mask_v, mask);

    // Morphological operations to clean up the mask
    cv::erode(mask, mask, cv::Mat());
    cv::dilate(mask, mask, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(5, 5)), cv::Point(-1, -1), 2);
    cv::erode(mask, mask, cv::Mat());

    return mask;
}

float Camera::calc_fov(float focal_length, float view_width) const {
    return glm::degrees(2.0f * std::atan(view_width / (2.0f * focal_length)));
}

glm::mat4 Camera::calc_proj(cv::Mat intrinsic, float width, float height, float near_plane, float far_plane) const {
    // If intrinsic matrix is invalid, fall back to default perspective
    if (intrinsic.empty() || intrinsic.rows < 3 || intrinsic.cols < 3) {
        return glm::perspective(glm::radians(DEFAULT_FOV), width / height, near_plane, far_plane);
    }

    // Extract intrinsic parameters
    double fx = intrinsic.at<double>(0, 0);
    double fy = intrinsic.at<double>(1, 1);
    double cx = intrinsic.at<double>(0, 2);
    double cy = intrinsic.at<double>(1, 2);

    // Guess calibration resolution by assuming principal point is near center
    const std::vector<glm::ivec2> common_res = {
        {640, 480}, {800, 600}, {1024, 768}, {1280, 720}, {1280, 960}, {1920, 1080}
    };
    
    double best_err = std::numeric_limits<double>::max();
    double calib_w = cx * 2.0;
    double calib_h = cy * 2.0;

    for (const auto& r : common_res) {
        if (cx < r.x && cy < r.y) {
            double err = std::abs(cx - r.x/2.0) / (r.x/2.0) + std::abs(cy - r.y/2.0) / (r.y/2.0);
            if (err < best_err) {
                best_err = err;
                calib_w = r.x;
                calib_h = r.y;
            }
        }
    }

    // Uniform scale to fit window, preserving aspect
    double scale = std::min(width / calib_w, height / calib_h);
    double fx_s = fx * scale, fy_s = fy * scale;
    double cx_s = cx * scale + (width - calib_w * scale) * 0.5;
    double cy_s = cy * scale + (height - calib_h * scale) * 0.5;

    // Build OpenGL-style projection matrix
    glm::mat4 perspective(0.0f);
    perspective[0][0] = 2.0f * static_cast<float>(fx_s) / width;
    perspective[1][1] = 2.0f * static_cast<float>(fy_s) / height;
    perspective[2][0] = 1.0f - 2.0f * static_cast<float>(cx_s) / width;
    perspective[2][1] = 2.0f * static_cast<float>(cy_s) / height - 1.0f;
    perspective[2][2] = -(far_plane + near_plane) / (far_plane - near_plane);
    perspective[2][3] = -1.0f;
    perspective[3][2] = -2.0f * far_plane * near_plane / (far_plane - near_plane);

    return perspective;
}
