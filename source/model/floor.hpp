#pragma once

#include <glm/glm.hpp>

#include "model.hpp"


/**
 * @class Floor
 * @brief Represents a ground floor grid for scene rendering.
 *
 * Supports configurable size, divisions, and color. Inherits from Model and implements required rendering interface.
 */
class Floor : public Model {
public: // Constructors
    /**
     * @brief Construct a Floor object.
     */
    Floor() : Model(ModelType::MESH_BASED) {}

    /** @brief Destructor. Cleans up resources. */
    ~Floor() = default;

public: // Methods
    /** @brief Initialize the floor model. */
    void initialize() override;

    /**
     * @brief Set the size of the floor grid.
     * @param size New size value.
     */
    void set_size(float size) { size_ = size; needs_rebuild_ = true; }

    /**
     * @brief Set the number of divisions in the floor grid.
     * @param divisions New divisions value.
     */
    void set_divisions(int divisions) { divisions_ = divisions; needs_rebuild_ = true; }

    /**
     * @brief Set the color of the floor grid.
     * @param color New color value.
     */
    void set_floor_color(const glm::vec4& color) { floor_color_ = color; }

public: // Getters
    /** @brief Get the size of the floor grid. */
    float size() const { return size_; }
    
    /** @brief Get the number of divisions in the floor grid. */
    int divisions() const { return divisions_; }

    /** @brief Get the color of the floor grid. */
    const glm::vec4& floor_color() const { return floor_color_; }

private: // Methods
    /** @brief Build the geometry for the floor grid. */
    void build_floor_geometry();

private: // Variables
    bool needs_rebuild_{true};

    float size_{20.0f};
    int divisions_{20};
    glm::vec4 floor_color_{0.7f, 0.7f, 0.7f, 1.0f};
};
