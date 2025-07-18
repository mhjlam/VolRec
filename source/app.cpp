#include "app.hpp"

#include <format>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <cxxopts.hpp>
#include <nlohmann/json.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "input.hpp"
#include "scene.hpp"
#include "camera.hpp"
#include "global.hpp"
#include "overlay.hpp"
#include "renderer.hpp"


/* Constructors */

App::App(int argc, char* argv[])
: window_(nullptr)
, app_context_{this}
, project_(std::make_shared<Project>())
{
    // Initialize GLFW and GLEW
    if (!initialize_window()) {
        throw std::runtime_error("Failed to initialize application window");
    }

    // Initialize project with default state
    project_ = std::make_shared<Project>();

    // Parse command line arguments
    parse_arguments(argc, argv);

    // Create all application components with default state
    scene_ = std::make_shared<Scene>();
    camera_ = std::make_shared<Camera>();
    renderer_ = std::make_shared<Renderer>(VIEW_WIDTH, VIEW_HEIGHT, scene_, camera_);
    overlay_ = std::make_shared<Overlay>(window_, scene_, renderer_, camera_, 
        [this](std::shared_ptr<Project> project) { load_project(project); },
        [this]() { unload_project(); }
    );
    input_ = std::make_unique<Input>(scene_, renderer_, camera_, overlay_);
    
    // Attempt to open project or initialize empty one otherwise
    if (project_->empty || !load_project(project_)) {
        unload_project();
    }

    // Show window only after all initialization is complete
    glfwShowWindow(window_);
}

App::~App() {
    glfwDestroyWindow(window_);
    glfwTerminate();
}


/* Public methods */

void App::run() {
    while (!glfwWindowShouldClose(window_)) {
        overlay_->new_frame();
        overlay_->render();
        renderer_->render();
        overlay_->end_frame();
        
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }
}

bool App::load_project(std::shared_ptr<Project> project) {
    if (!project || project->empty || project->file.empty()) {
        return false;
    }

    // Check if project file exists
    std::ifstream file_stream(project->file);
    if (!file_stream) {
        std::cerr << "Could not open project file: " << project->file << std::endl;
        return false;
    }

    // Parse JSON project file
    nlohmann::json json;
    file_stream >> json;

    // Set project_name_ from JSON if available, otherwise fallback to filename
    if (json.contains("project_name") && json["project_name"].is_string()) {
        project->name = json["project_name"].get<std::string>();
    }

    // Chessboard parameters
    if (json.contains("chessboard")) {
        const auto& cb = json["chessboard"];
        if (cb.contains("cols")) { project->chess_cols = cb["cols"].get<int>(); }
        if (cb.contains("rows")) { project->chess_rows = cb["rows"].get<int>(); }
        if (cb.contains("square")) { project->square_size = cb["square"].get<float>(); }
    }

    if (project->chess_cols < 3 || project->chess_rows < 3 
    ||  project->chess_cols > 20 || project->chess_rows > 20 
    ||  project->square_size < 5.0f || project->square_size > 100.0f) {
        std::cerr << "Invalid chessboard parameters." << std::endl;
        return false;
    }

    // Collect and check views
    if (!json.contains("views") || !json["views"].is_array() || json["views"].empty()) {
        std::cerr << "No views specified in project file." << std::endl;
        return false;
    }

    auto& json_views = json["views"];
    for (int i = 0; i < json_views.size(); ++i) {
        auto& json_view = json_views[i];

        // Check required fields
        if (!json_view.contains("background") || !json_view["background"].is_string()) {
            std::cerr << "Missing background image for view " << i + 1 << std::endl;
            return false;
        }
        if (!json_view.contains("foreground") || !json_view["foreground"].is_string()) {
            std::cerr << "Missing foreground image for view " << i + 1 << std::endl;
            return false;
        }

        View view;
        view.bg_path = (project->dir / json_view["background"].get<std::string>());
        view.bg = cv::imread(view.bg_path.string(), cv::IMREAD_UNCHANGED);
        view.fg_path = (project->dir / json_view["foreground"].get<std::string>());
        view.fg = cv::imread(view.fg_path.string(), cv::IMREAD_UNCHANGED);
        view.cb_path = (view.bg_path.parent_path() / std::format("cb{}.yml", i + 1));

        if (json_view.contains("camera") && json_view["camera"].is_string()) {
            view.cb_path = (project->dir / json_view["camera"].get<std::string>());
        }

        // Check if background and foreground images exist and are loadable
        if (!std::filesystem::exists(view.bg_path) || view.bg.empty()) {
            std::cerr << "Background image not found or not loadable: " << view.bg_path << std::endl;
            return false;
        }

        if (!std::filesystem::exists(view.fg_path) || view.fg.empty()) {
            std::cerr << "Foreground image not found or not loadable: " << view.fg_path << std::endl;
            return false;
        }

        // Check if calibration file exists and is loadable
        std::ifstream cb_file(view.cb_path);
        if (!std::filesystem::exists(view.cb_path) || !std::filesystem::is_regular_file(view.cb_path) || !cb_file.is_open()) {
            project->needs_calibration = true;
        }

        project->views.push_back(view);
    }

    // Initialize scene and renderer with the project
    camera_->load_project(project);
    scene_->load_project(project);
    renderer_->load_project(project);
    overlay_->load_project(project);

    // Store the initialized project state
    project->empty = false;
    project->initialized = true;
    project->needs_calibration = false;

    return true;
}

void App::unload_project() {
    project_ = std::make_shared<Project>();

    scene_->unload_project();
    renderer_->unload_project();
    overlay_->unload_project();
    camera_->unload_project();
}


/* Private methods */

void App::parse_arguments(int argc, char** argv) {
    cxxopts::Options options("VolRec", "Volumetric Reconstruction");
    options.add_options()
        ("project", "Project file", cxxopts::value<std::string>())
        ("f,force-calibration", "Force camera calibration")
        ("h,help", "Print usage");
    
    // Tell cxxopts that the first positional argument is "project"
    options.parse_positional({"project"});

    // Parse command line arguments
    auto args = options.parse(argc, argv);

    if (args.count("help")) {
        std::cout << options.help() << std::endl;
        std::exit(0);
    }

    if (args.count("project")) {
        project_->file = std::filesystem::absolute(args["project"].as<std::string>());
        if (std::filesystem::exists(project_->file)) {
            project_->dir = project_->file.parent_path();
            project_->name = project_->file.stem().string();
            project_->empty = false;

            if (args.count("force-calibration")) {
                project_->needs_calibration = args["force-calibration"].as<bool>();
            }
        }
    }
}

bool App::initialize_window() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return false;
    }

    // Set OpenGL context hints for modern rendering
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    window_ = glfwCreateWindow(VIEW_WIDTH, VIEW_HEIGHT, "VolRec", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return false;
    }

    // Set window icon using OpenCV and GLFW
    cv::Mat icon_img = cv::imread("VolRec.png", cv::IMREAD_UNCHANGED);
    if (!icon_img.empty() && icon_img.channels() == 4) {
        // Convert BGRA to RGBA
        cv::cvtColor(icon_img, icon_img, cv::COLOR_BGRA2RGBA);
        GLFWimage icon;
        icon.width = icon_img.cols;
        icon.height = icon_img.rows;
        icon.pixels = icon_img.ptr<unsigned char>();
        glfwSetWindowIcon(window_, 1, &icon);
    }
    else {
        std::cerr << "Failed to load VolRec.png for window icon." << std::endl;
    }

    // Set the application context
    glfwMakeContextCurrent(window_);
    glfwSetWindowUserPointer(window_, &app_context_);

    // Initialize GLEW after making OpenGL context current
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW!" << std::endl;
        glfwTerminate();
        return false;
    }

    // Set keyboard and mouse callbacks
    glfwSetKeyCallback(window_, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(w));
        if (ctx && ctx->app && ctx->app->input_) { ctx->app->input_->on_key(w, key, scancode, action, mods); }
    });

    glfwSetCursorPosCallback(window_, [](GLFWwindow* w, double xpos, double ypos) {
        AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(w));
        if (ctx && ctx->app && ctx->app->input_) { ctx->app->input_->on_cursor_pos(w, xpos, ypos); }
    });

    glfwSetMouseButtonCallback(window_, [](GLFWwindow* w, int button, int action, int mods) {
        AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(w));
        if (ctx && ctx->app && ctx->app->input_) { ctx->app->input_->on_mouse_button(w, button, action, mods); }
    });

    // Set resize callback
    glfwSetFramebufferSizeCallback(window_, [](GLFWwindow* w, int width, int height) {
        AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(w));
        if (ctx && ctx->app && ctx->app->input_) { ctx->app->renderer_->resize(width, height); }
    });

    // Center the window before showing it
    if (GLFWmonitor* primary = glfwGetPrimaryMonitor()) {
        if (const GLFWvidmode* mode = glfwGetVideoMode(primary)) {
            glfwSetWindowPos(window_, (mode->width - VIEW_WIDTH) / 2, (mode->height - VIEW_HEIGHT) / 2);
        }
    }
    return true;
}
