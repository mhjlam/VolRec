
#include "frustum.hpp"

#include <limits>

#include "global.hpp"
#include "render/mesh.hpp"
#include "render/shader.hpp"


/* Constructors */

Frustum::Frustum(const View& view) : Model(ModelType::MESH_BASED), vertices_{}, colors_{}
{
    generate_frustum_geometry(view);
}


/* Public methods */

void Frustum::initialize() {
    build_frustum_mesh();
    needs_rebuild_ = false;
}

void Frustum::set_frustum_color(const glm::vec4& color) {
    frustum_color_ = color;
    needs_rebuild_ = true;
}

void Frustum::update_frustum(const View& view) {
    generate_frustum_geometry(view);
    needs_rebuild_ = true;
}


/* Private methods */

void Frustum::generate_frustum_geometry(const View& view) {
    cv::Size bg_size = view.bg.size();
    int vw = VIEW_WIDTH;
    int vh = VIEW_HEIGHT;

    float width = static_cast<float>(bg_size.width);
    float height = static_cast<float>(bg_size.height);
    float scale = std::min(vw / width, vh / height);

    // Scaled focal lengths and principal point
    float fx_s = static_cast<float>(view.focal_length.at<double>(0, 0)) * scale;
    float fy_s = static_cast<float>(view.focal_length.at<double>(1, 0)) * scale;
    float cx_s = static_cast<float>(view.principal_point.at<double>(0, 0)) * scale + 0.5f * (vw - width * scale);
    float cy_s = static_cast<float>(view.principal_point.at<double>(1, 0)) * scale + 0.5f * (vh - height * scale);

    // Image plane distance (positive, in front of camera in OpenGL coordinates)
    float z = -0.5f * (fx_s + fy_s);

    // Image corners in camera space (OpenGL coordinates: Y+ up, Z- forward)
    glm::vec2 img_corners[4] = {
        {  -cx_s, cy_s   }, // top-left
        {vw-cx_s, cy_s   }, // top-right
        {vw-cx_s, cy_s-vh}, // bottom-right
        {  -cx_s, cy_s-vh}  // bottom-left
    };

    std::vector<glm::vec3> corners;
    for (const auto& c : img_corners) {
        glm::vec3 cam_pt(c.x, c.y, z);
        corners.push_back(view.eye + cam_pt.x * view.right + cam_pt.y * view.upward + cam_pt.z * view.forward);
    }

    // Prepare vertices and colors
    vertices_.clear();
    vertices_.reserve(16);
    colors_.clear();
    colors_.reserve(8);

    const glm::vec4 color_center{0.8f, 0.8f, 0.8f, 0.7f}, color_edge{0.5f, 0.5f, 0.5f, 0.7f};

    // Center to corners
    for (const auto& corner : corners) {
        colors_.push_back(color_center);
        vertices_.push_back(view.eye);
        vertices_.push_back(corner);
    }

    // Rectangle edges
    for (int i = 0; i < 4; ++i) {
        colors_.push_back(color_edge);
        vertices_.push_back(corners[i]);
        vertices_.push_back(corners[(i+1)%4]);
    }
}

void Frustum::build_frustum_mesh() {
    if (!needs_rebuild_ && !meshes().empty()) {
        return;
    }
    
    // Clear existing meshes
    clear_meshes();
    
    if (!vertices_.empty()) {
        // Create lines mesh from frustum vertices using the frustum color
        auto frustum_mesh = Mesh::create_lines(vertices_, frustum_color_);
        
        if (frustum_mesh) {
            frustum_mesh->upload_to_gpu();
            add_mesh(std::shared_ptr<Mesh>(frustum_mesh.release()));
        }
        
        // Set the model color to match frustum color
        set_color(frustum_color_);
    }
}
