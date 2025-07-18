#pragma once

#include <limits>
#include <string>
#include <numbers>


constexpr const int VIEW_WIDTH = 1280;
constexpr const int VIEW_HEIGHT = 720;
constexpr const float VIEW_ASPECT = static_cast<float>(VIEW_WIDTH) / static_cast<float>(VIEW_HEIGHT);

constexpr const int VOLUME_VOXEL_SIZE = 40;
constexpr const int VOLUME_BOX_LENGTH = 800;

// Mathematical constants
constexpr const float EPSILON = std::numeric_limits<float>::epsilon();
constexpr const float HALF_PI = std::numbers::pi_v<float> / 2.0f;


class App;


/**
 * @struct AppContext
 * @brief Holds a pointer to the main application instance.
 */
struct AppContext {
    App* app = nullptr; ///< Pointer to the main application instance
};

/**
 * @brief Get the directory of the current executable.
 * @return Path to the executable directory.
 */
std::string get_executable_dir();

/**
 * @brief Open a file dialog to select a project file.
 * @return Path to the selected project file.
 */
std::string open_project_file_dialog();

/**
 * @brief Check for OpenGL errors after an operation.
 * @param operation Description of the operation.
 * @return Error string if any error occurred.
 */
std::string check_gl_error(const std::string& operation);
