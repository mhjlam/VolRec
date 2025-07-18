#pragma once

#include <array>
#include <memory>
#include <filesystem>

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

#include "global.hpp"
#include "project.hpp"


class Input;
class Scene;
class Camera;
class Overlay;
class Renderer;
class Calibrator;
struct GLFWwindow;


/**
 * @class App
 * @brief Main application class. Manages window, input, and core components.
 */
class App {
public: // Constructors
    /**
     * @brief Construct a new App object.
     * @param argc Argument count.
     * @param argv Argument values.
     */
    App(int argc, char* argv[]);

    /** @brief Destructor. Cleans up resources. */
    ~App();

public: // Methods
    /** @brief Run the main application loop. */
    void run();

    /**
     * @brief Load a project and initialize core components.
     * @param project Shared pointer to the project to load.
     * @return True if successful.
     */
    bool load_project(std::shared_ptr<Project> project);

    /** @brief Unload the current project and reset state. */
    void unload_project();

private: // Methods
    /**
     * @brief Parse command line arguments.
     * @param argc Argument count.
     * @param argv Argument values.
     */
    void parse_arguments(int argc, char** argv);

    /** @brief Initialize the application window. */
    bool initialize_window();

private: // Variables
    GLFWwindow* window_;
    AppContext app_context_;

    std::shared_ptr<Project> project_;
    
    std::shared_ptr<Scene> scene_;
    std::shared_ptr<Camera> camera_;
    std::shared_ptr<Overlay> overlay_;
    std::shared_ptr<Renderer> renderer_;

    std::unique_ptr<Input> input_;
};
