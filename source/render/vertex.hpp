#pragma once

#include <glm/glm.hpp>

/**
 * @struct Vertex
 * @brief Represents a single vertex with position, normal, texture coordinates, and color.
 */
struct Vertex
{
    glm::vec3 position;   /**< Vertex position */
    glm::vec3 normal;     /**< Vertex normal */
    glm::vec2 tex_coords; /**< Texture coordinates */
    glm::vec4 color;      /**< Vertex color */

    /**
     * @brief Default constructor. Initializes to default values.
     */
    Vertex()
    : position(0.0f)
    , normal(0.0f, 1.0f, 0.0f)
    , tex_coords(0.0f)
    , color(1.0f) {}

    /**
     * @brief Construct with position.
     * @param pos Vertex position.
     */
    Vertex(const glm::vec3& pos)
    : position(pos)
    , normal(0.0f, 1.0f, 0.0f)
    , tex_coords(0.0f)
    , color(1.0f) {}

    /**
     * @brief Construct with position and normal.
     * @param pos Vertex position.
     * @param norm Vertex normal.
     */
    Vertex(const glm::vec3& pos, const glm::vec3& norm)
    : position(pos)
    , normal(norm)
    , tex_coords(0.0f)
    , color(1.0f) {}

    /**
     * @brief Construct with position, normal, and texture coordinates.
     * @param pos Vertex position.
     * @param norm Vertex normal.
     * @param tex Texture coordinates.
     */
    Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex)
    : position(pos)
    , normal(norm)
    , tex_coords(tex)
    , color(1.0f) {}

    /**
     * @brief Construct with position, normal, texture coordinates, and color.
     * @param pos Vertex position.
     * @param norm Vertex normal.
     * @param tex Texture coordinates.
     * @param col Vertex color.
     */
    Vertex(const glm::vec3& pos, const glm::vec3& norm, const glm::vec2& tex, const glm::vec4& col)
    : position(pos)
    , normal(norm)
    , tex_coords(tex)
    , color(col) {}
};
