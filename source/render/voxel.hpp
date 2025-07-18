#pragma once

#include <glm/glm.hpp>

/**
 * @struct Voxel
 * @brief Represents a single voxel for volumetric rendering.
 *
 * Contains position, color, density, and active state.
 */
struct Voxel
{
    glm::vec3 position;   /**< Voxel position */
    glm::vec4 color;      /**< Voxel color */
    float density;        /**< Density value for volume rendering */
    bool active;          /**< Whether this voxel should be rendered */

    /**
     * @brief Default constructor. Initializes to default values.
     */
    Voxel()
    : position(0.0f)
    , color(1.0f)
    , density(1.0f)
    , active(false) {}

    /**
     * @brief Construct with position, color, density, and active state.
     * @param pos Voxel position.
     * @param col Voxel color.
     * @param dens Density value.
     * @param act Active state.
     */
    Voxel(const glm::vec3& pos, const glm::vec4& col = glm::vec4(1.0f), float dens = 1.0f, bool act = true)
    : position(pos)
    , color(col)
    , density(dens)
    , active(act) {}
};
