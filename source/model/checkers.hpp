#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

#include "model.hpp"


/**
 * @class Checkers
 * @brief Represents the checkerboard model for visualization on the floor.
 */
class Checkers : public Model {
public: // Constructors
    /**
     * @brief Construct a checkers board with given number of inner corners and square size (in mm).
     * @param rows Number of rows in the checkerboard.
     * @param cols Number of columns in the checkerboard.
     * @param square_size Size of each square in millimeters.
     */
    Checkers(int rows, int cols, float square_size)
    : Model(ModelType::MESH_BASED)
    , rows_(rows)
    , cols_(cols)
    , square_size_(square_size) {}

    /** @brief Destructor. Cleans up resources. */
    ~Checkers() override = default;

public: // Methods
    /** @brief Initialize the checkers model. */
    void initialize() override;

    /**
     * @brief Update the checkers model (no-op).
     * @param delta_time Time since last update.
     */
    void update(float delta_time) override {}

    /** @brief Force VAO and attribute layout to be correct for all meshes. */
    void force_upload_all_meshes();

public: // Getters
    /** @brief Get the number of rows in the checkerboard. */
    int rows() const { return rows_; }
    
    /** @brief Get the number of columns in the checkerboard. */
    int cols() const { return cols_; }
    
    /** @brief Get the size of each square in millimeters. */
    float square_size() const { return square_size_; }

private: // Variables
    int rows_ = 0;
    int cols_ = 0;
    float square_size_ = 0.0f;
};
