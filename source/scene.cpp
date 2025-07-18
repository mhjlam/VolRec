#include "scene.hpp"

#include <limits>
#include <fstream>
#include <iostream>

#include <omp.h>
#include <opencv2/opencv.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "model/box.hpp"
#include "model/floor.hpp"
#include "model/frame.hpp"
#include "model/volume.hpp"
#include "model/frustum.hpp"
#include "model/checkers.hpp"


/* Public methods */

void Scene::load_project(std::shared_ptr<Project> project) {
    create_box();
    create_floor();
    create_frame();
    create_checkers(project->chess_rows, project->chess_cols, project->square_size);
    create_frustums(project->views);
    create_volume(project->views);
}

void Scene::unload_project() {
    // Reset all models
    box_.reset();
    floor_.reset();
    frame_.reset();
    checkers_.reset();
    frustums_.clear();
    volume_.reset();

    // Create empty default models
    create_box();
    create_floor();
    create_frame();
    create_checkers(CHESS_ROWS, CHESS_COLS, CHESS_SQUARE);
    create_empty_volume();
}


/* Private methods */

void Scene::create_box() {
    box_ = std::make_shared<Box>();

    // Configure box based on global constants
    glm::vec3 box_size = glm::vec3(VOLUME_BOX_LENGTH * 2, VOLUME_BOX_LENGTH, VOLUME_BOX_LENGTH * 2);
    glm::vec3 box_position = glm::vec3(0.0f, VOLUME_BOX_LENGTH * 0.5f, 0.0f); // Center it vertically

    box_->set_size(box_size);
    box_->set_position(box_position);
    box_->set_wireframe(true); // Volume box should be wireframe
    box_->set_box_color(glm::vec4(0.9f, 0.9f, 0.9f, 0.5f));

    box_->initialize();
}

void Scene::create_floor() {
    floor_ = std::make_shared<Floor>();

    // Configure floor based on global constants
    int floor_size = VOLUME_BOX_LENGTH * 2;
    int half_floor_edge = VOLUME_BOX_LENGTH / 2;;

    floor_->set_size(static_cast<float>(floor_size));
    floor_->set_divisions(floor_size / half_floor_edge);
    floor_->set_floor_color(glm::vec4(0.9f, 0.9f, 0.9f, 0.5f));
    floor_->initialize();
}

void Scene::create_frame() {
    std::array<glm::vec4, 3> axis_colors{{
        {1.0f, 0.0f, 0.0f, 1.0f}, // X = Red
        {0.0f, 1.0f, 0.0f, 1.0f}, // Y = Green
        {0.0f, 0.0f, 1.0f, 1.0f}  // Z = Blue
    }};

    frame_ = std::make_shared<Frame>();
    frame_->set_axis_length(500.0f);
    frame_->set_axis_colors(axis_colors);
    frame_->initialize();
}

void Scene::create_checkers(int cols, int rows, float square_size) {
    // Use provided or default parameters
    if (cols <= 0) { cols = CHESS_COLS; }
    if (rows <= 0) { rows = CHESS_ROWS; }
    if (square_size <= 0.0f) { square_size = CHESS_SQUARE; }

    checkers_ = std::make_shared<Checkers>(cols, rows, square_size);
    checkers_->set_color(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    checkers_->initialize();
    checkers_->force_upload_all_meshes();
}

void Scene::create_frustums(const std::vector<View>& views) {
    frustums_.clear();
    frustums_.reserve(views.size());

    for (const auto &view : views) {
        auto frustum = std::make_shared<Frustum>(view);
        frustum->set_frustum_color(glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
        frustum->initialize();
        frustums_.push_back(frustum);
    }
}

void Scene::create_empty_volume() {
    const int num_x = (VOLUME_BOX_LENGTH * 2) / VOLUME_VOXEL_SIZE;
    const int num_y = (VOLUME_BOX_LENGTH * 2) / VOLUME_VOXEL_SIZE;
    const int num_z = VOLUME_BOX_LENGTH / VOLUME_VOXEL_SIZE;

    volume_ = std::make_shared<Volume>(num_x, num_y, num_z, static_cast<float>(VOLUME_VOXEL_SIZE));

    const int total_voxels = num_x * num_y * num_z;
    #pragma omp parallel for
    for (int idx = 0; idx < total_voxels; ++idx) {
        int xi = idx / (num_y * num_z);
        int yi = (idx / num_z) % num_y;
        int zi = idx % num_z;
        volume_->set_voxel_active(xi, yi, zi, true);
        volume_->set_voxel_color(xi, yi, zi, glm::vec4(0.8f, 0.3f, 0.2f, 0.9f));
    }

    volume_->initialize();
}

void Scene::create_volume(const std::vector<View>& views) {
    auto voxel_visible = [](const View& view, const cv::Point3f& vox_pos) {
        // Check if mask is empty
        if (view.mask.empty()) {
            return false;
        }

        // Project the 3D point into the image plane using OpenCV
        std::vector<cv::Point2f> img_pts;
        std::vector<cv::Point3f> obj_pts = {vox_pos};
        cv::projectPoints(obj_pts, view.rvec, view.tvec_proj, view.intrinsic, view.distortion, img_pts);

        // Check if the projected point is within the image bounds and on the mask
        if (img_pts.empty()) {
            return false;
        }

        const auto& pt = img_pts[0];
        int px = cvRound(pt.x);
        int py = cvRound(pt.y);

        if (px < 0 || px >= view.mask.cols || py < 0 || py >= view.mask.rows) {
            return false;
        }
        return view.mask.at<uint8_t>(py, px) == std::numeric_limits<uint8_t>::max(); // 255
    };

    const int num_x = (VOLUME_BOX_LENGTH * 2) / VOLUME_VOXEL_SIZE;
    const int num_y = (VOLUME_BOX_LENGTH * 2) / VOLUME_VOXEL_SIZE;
    const int num_z = VOLUME_BOX_LENGTH / VOLUME_VOXEL_SIZE;
    volume_ = std::make_shared<Volume>(num_x, num_y, num_z, static_cast<float>(VOLUME_VOXEL_SIZE));

    std::vector<std::vector<cv::Point3i>> thread_voxels(omp_get_max_threads());

#pragma omp parallel
    {
        auto& local_voxels = thread_voxels[omp_get_thread_num()];
        
#pragma omp for schedule(dynamic, 2) nowait
        for (int idx = 0; idx < num_x * num_y * num_z; ++idx) {
            int xi = idx / (num_y * num_z);
            int yi = (idx / num_z) % num_y;
            int zi = idx % num_z;
            
            // Calculate voxel position in OpenCV coordinates
            int x = -VOLUME_BOX_LENGTH + xi * VOLUME_VOXEL_SIZE;
            int y = -VOLUME_BOX_LENGTH + yi * VOLUME_VOXEL_SIZE;
            int z = zi * VOLUME_VOXEL_SIZE;

            bool all_visible = std::ranges::all_of(views, [&](const View& view) {
                return voxel_visible(view, {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
            });

            if (all_visible) {
                local_voxels.emplace_back(x, z, -y);
                volume_->set_voxel_active(xi, yi, zi, true);
                volume_->set_voxel_color(xi, yi, zi, glm::vec4(0.8f, 0.3f, 0.2f, 0.9f));
            }
        }
    }

    volume_->initialize();
}
