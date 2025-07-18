#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

#include "view.hpp"
#include "model.hpp"

/**
 * @class Frustum
 * @brief Represents a camera frustum for rendering camera views.
 */
class Frustum : public Model {
public: // Constructors
    /**
     * @brief Construct a frustum using a View reference.
     * @param view Camera view parameters.
     */
    Frustum(const View& view);

    /** @brief Destructor. Cleans up resources. */
    ~Frustum() = default;

public: // Methods
    /** @brief Initialize the frustum model. */
    void initialize() override;

    /**
     * @brief Set the color of the frustum.
     * @param color New color for the frustum.
     */
    void set_frustum_color(const glm::vec4& color);

    /**
     * @brief Update frustum geometry (useful for dynamic cameras).
     * @param view Camera view parameters.
     */
    void update_frustum(const View& view);

public: // Getters
    /** @brief Get the frustum vertices for line rendering. */
    const std::vector<glm::vec3>& vertices() const { return vertices_; }

    /** @brief Get the colors for each frustum line. */
    const std::vector<glm::vec4>& colors() const { return colors_; }

    /** @brief Get the color of the frustum. */
    const glm::vec4& frustum_color() const { return frustum_color_; }

private: // Methods
    /**
     * @brief Generate frustum geometry from a View.
     * @param view Camera view parameters.
     */
    void generate_frustum_geometry(const View& view);

    /** @brief Build the frustum mesh for rendering. */
    void build_frustum_mesh();
    
private: // Variables
    bool needs_rebuild_{true};
    std::vector<glm::vec3> vertices_;                   // Flat list of vertices for line rendering
    std::vector<glm::vec4> colors_;                     // Colors for each line
    glm::vec4 frustum_color_{0.8f, 0.8f, 0.8f, 1.0f};   // Light gray
};
