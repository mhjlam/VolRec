#pragma once

#include <glm/glm.hpp>

#include "model.hpp"


class Box : public Model {
public: // Constructors
    /** @brief Construct a new Box object. */
    Box() : Model(ModelType::MESH_BASED) {}
    
    /** @brief Destructor. Cleans up resources. */
    ~Box() = default;

public: // Methods
    /** @brief Initialize the Box. */
    void initialize() override;
    
    // Box-specific configuration
    /**
     * @brief Set the size of the Box.
     * @param size New size for the Box.
     */
    void set_size(const glm::vec3& size);
    
    /**
     * @brief Set the position of the Box.
     * @param position New position for the Box.
     */
    void set_position(const glm::vec3& position);
    
    /**
     * @brief Set the wireframe mode for the Box.
     * @param wireframe True to enable wireframe mode, false otherwise.
     */
    void set_wireframe(bool wireframe);
    
    /**
     * @brief Set the color of the Box.
     * @param color New color for the Box.
     */
    void set_box_color(const glm::vec4& color);
    
public: // Getters
    /**
     * @brief Get the size of the Box.
     * @return Reference to the size of the Box.
     */
    const glm::vec3& size() const { return size_; }
    
    /**
     * @brief Get the position of the Box.
     * @return Reference to the position of the Box.
     */
    const glm::vec3& position() const { return position_; }
    
    /**
     * @brief Check if the Box is in wireframe mode.
     * @return True if wireframe mode is enabled, false otherwise.
     */
    bool is_wireframe() const { return wireframe_; }
    
    /**
     * @brief Get the color of the Box.
     * @return Reference to the color of the Box.
     */
    const glm::vec4& box_color() const { return box_color_; }

private: // Methods
    /** @brief Build the geometry for the Box. */
    void build_box_geometry();
    
private: // Variables
    bool wireframe_{true};
    bool needs_rebuild_{true};

    glm::vec3 size_{2.0f, 2.0f, 2.0f};
    glm::vec3 position_{0.0f, 0.0f, 0.0f};
    glm::vec4 box_color_{1.0f, 1.0f, 0.0f, 1.0f}; // Yellow
};
