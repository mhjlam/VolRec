#pragma once

#include <array>

#include <glm/glm.hpp>

#include "model.hpp"


/**
 * @class Frame
 * @brief Represents a world axes frame for scene visualization.
 *
 * Supports configurable axis length, colors, and position. Inherits from Model and implements required rendering interface.
 */
class Frame : public Model {
public: // Constructors
    /** @brief Construct a Frame object. */
    Frame() : Model(ModelType::MESH_BASED) {}

    /** @brief Destructor. Cleans up resources. */
    ~Frame() = default;

public: // Methods
    /** @brief Initialize the frame model. */
    void initialize() override;

    /**
     * @brief Set the length of the axes.
     * @param length New axis length.
     */
    void set_axis_length(float length) { axis_length_ = length; needs_rebuild_ = true; }

    /**
     * @brief Set the colors for the axes.
     * @param colors Array of colors for X, Y, Z axes.
     */
    void set_axis_colors(const std::array<glm::vec4, 3>& colors) { axis_colors_ = colors; needs_rebuild_ = true; }

    /**
     * @brief Set the position of the frame.
     * @param position New position vector.
     */
    void set_position(const glm::vec3& position);

public: // Getters
    /** @brief Get the length of the axes. */
    float axis_length() const { return axis_length_; }

    /** @brief Get the colors of the axes. */
    const std::array<glm::vec4, 3>& axis_colors() const { return axis_colors_; }

    /** @brief Get the position of the frame. */
    const glm::vec3& position() const { return position_; }

private: // Methods
    /** @brief Build the geometry for the frame axes. */
    void build_frame_geometry();

private: // Variables
    bool needs_rebuild_{true};

    float axis_length_{1.0f};
    glm::vec3 position_{0.0f, 0.0f, 0.0f};
    
    std::array<glm::vec4, 3> axis_colors_{{
        {1.0f, 0.0f, 0.0f, 1.0f}, // X - Red
        {0.0f, 1.0f, 0.0f, 1.0f}, // Y - Green  
        {0.0f, 0.0f, 1.0f, 1.0f}  // Z - Blue
    }};
};
