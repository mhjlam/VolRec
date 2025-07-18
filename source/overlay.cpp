#include "overlay.hpp"

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "scene.hpp"
#include "camera.hpp"
#include "global.hpp"
#include "renderer.hpp"


static constexpr float MENU_WINDOW_WIDTH = 300.0f;
static constexpr ImVec2 CAMERA_BUTTON_SIZE = ImVec2(30, 30);


/* Constructors */

Overlay::Overlay(GLFWwindow* window, 
                 std::shared_ptr<Scene> scene, 
                 std::shared_ptr<Renderer> renderer, 
                 std::shared_ptr<Camera> camera,
                 std::function<void(std::shared_ptr<Project>)> project_load_cb, 
                 std::function<void()> project_close_cb)
: window_(window)
, project_(std::make_shared<Project>())
, scene_(scene)
, camera_(camera)
, renderer_(renderer)
, project_open_callback_(std::move(project_load_cb))
, project_close_callback_(std::move(project_close_cb))
{
    toggle_items_ = {
        { "Box",      &show_box_,      [this]{ if (renderer_) { renderer_->toggle_box(); } } },
        { "Frame",    &show_frame_,    [this]{ if (renderer_) { renderer_->toggle_frame(); } } },
        { "Floor",    &show_floor_,    [this]{ if (renderer_) { renderer_->toggle_floor(); } } },
        { "Checkers", &show_checkers_, [this]{ if (renderer_) { renderer_->toggle_checkers(); } } },
        { "Frustums", &show_frustums_, [this]{ if (renderer_) { renderer_->toggle_frustums(); } } },
        { "Volume",   &show_volume_,   [this]{ if (renderer_) { renderer_->toggle_volume(); } } }
    };

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    
    // Adjust style for better visibility
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 5.0f;
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    // Setup Platform/Renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(window_, true)) {
        throw std::runtime_error("Failed to initialize ImGui GLFW backend!");
    }

    // Use OpenGL 4.5 core profile
    const char* glsl_version = "#version 450 core";
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        ImGui_ImplGlfw_Shutdown();
        throw std::runtime_error("Failed to initialize ImGui OpenGL3 backend!");
    }

    std::cout << "ImGui overlay initialized successfully" << std::endl;
}

Overlay::~Overlay() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


/* Public methods */

void Overlay::load_project(std::shared_ptr<Project> project) {
    project_ = project;
    reset_ui_state(true);
    if (renderer_ && !renderer_->get_show_volume()) {
        renderer_->toggle_volume();
    }
    sync_with_renderer();
}

void Overlay::unload_project() {
    project_ = std::make_shared<Project>();
    reset_ui_state(false);
    if (renderer_ && renderer_->get_show_volume()) {
        renderer_->toggle_volume();
    }
    error_popup_open_ = false;
    error_popup_message_.clear();
    sync_with_renderer();
}

void Overlay::set_current_camera_view(int view_index) {
    // Accept -1 (freeform) or valid static view indices
    if (view_index >= 0 || view_index < project_->views.size()) { 
        active_camera_view_ = view_index;
    }
}

void Overlay::new_frame() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Overlay::render() {
    // Show error popup if needed
    if (error_popup_open_) {
        ImGui::OpenPopup("Error");
    }
    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", error_popup_message_.c_str());
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            error_popup_open_ = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    render_menu();

    // Deferred project actions (must be called after ImGui frame)
    if (pending_project_load_ && project_open_callback_) {
        pending_project_load_ = false;

        auto project = std::make_shared<Project>();
        project->file = std::filesystem::path(pending_project_path_);
        project->dir = project->file.parent_path();
        project->name = project->file.stem().string();
        project->empty = false;

        std::string path = std::move(pending_project_path_);
        pending_project_path_.clear();
        project_open_callback_(project);
        return;
    }

    if (pending_project_close_ && project_close_callback_) {
        pending_project_close_ = false;
        project_close_callback_();
        return;
    }
}

void Overlay::end_frame() {
    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool Overlay::wants_capture_mouse() const {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

bool Overlay::wants_capture_keyboard() const {
    ImGuiIO& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

void Overlay::sync_with_renderer() {
    if (!renderer_ || !scene_) { return; }

    // Sync UI state with renderer flags
    show_box_ = renderer_->get_show_box();
    show_floor_ = renderer_->get_show_floor();
    show_frame_ = renderer_->get_show_frame();
    show_volume_ = renderer_->get_show_volume();
    show_frustums_ = renderer_->get_show_frustums();
    show_checkers_ = renderer_->get_show_checkers();
    show_background_ = renderer_->get_show_background();
}

void Overlay::show_error_popup(const std::string& message) {
    set_error_popup(message);
}


/* Private methods */

void Overlay::render_menu() {
    render_main_menu_bar();
    render_control_window();
}

void Overlay::render_main_menu_bar() {
    if (!ImGui::BeginMainMenuBar()) {
        return;
    }

    // Get menu bar dimensions
    float menu_bar_width = ImGui::GetWindowWidth();
    float help_width = ImGui::CalcTextSize("(?)").x + ImGui::GetStyle().ItemSpacing.x * 2.0f;

    // Render menu sections
    render_file_menu();
    render_project_name(menu_bar_width);
    render_help_tooltip(menu_bar_width, help_width);

    ImGui::EndMainMenuBar();
}

void Overlay::render_file_menu() {
    if (ImGui::BeginMenu("File")) {
        // Open Project
        if (ImGui::MenuItem("Open...")) {
            std::string selected_file = open_project_file_dialog();
            if (!selected_file.empty()) 
            {
                pending_project_load_ = true;
                pending_project_path_ = selected_file;
            }
        }

        // Close Project (disabled if no project loaded)
        BeginDisabledIf(!project_->initialized);
        if (ImGui::MenuItem("Close")) {
            pending_project_close_ = true;
        }
        EndDisabledIf(!project_->initialized);

        ImGui::EndMenu();
    }
}

void Overlay::render_project_name(float menu_bar_width) {
    // Display the project name centered in the menu bar
    std::string project_string = project_->name.empty() ? "No Project Loaded" : project_->name;

    // Calculate width of the project name and center position
    float project_name_width = ImGui::CalcTextSize(project_string.c_str()).x;
    float center_x = (menu_bar_width - project_name_width) * 0.5f;

    // Move cursor to center, but not before the File menu
    float file_menu_end_x = ImGui::GetCursorPosX();
    if (center_x > file_menu_end_x) {
        ImGui::SetCursorPosX(center_x);
    }

    ImGui::TextUnformatted(project_string.c_str());
}

void Overlay::render_help_tooltip(float menu_bar_width, float help_width) {
    ImGui::SameLine(menu_bar_width - help_width);
    ImGui::TextDisabled("(?)");
    
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Mouse Controls:");
        ImGui::BulletText("LMB + drag: Rotate camera");
        ImGui::BulletText("RMB + drag: Zoom in/out");
        ImGui::Separator();
        ImGui::Text("Keyboard Shortcuts:");
        ImGui::BulletText("1-4: Switch views");
        ImGui::BulletText("A: Toggle world axes");
        ImGui::BulletText("B: Toggle volume box");
        ImGui::BulletText("F: Toggle floor grid");
        ImGui::BulletText("C: Toggle camera frustums");
        ImGui::BulletText("V: Toggle volume visibility");
        ImGui::BulletText("G: Toggle background window");
        ImGui::BulletText("H: Toggle foreground window");
        ImGui::EndTooltip();
    }
}

void Overlay::render_control_window() {
    // Save current style and set window rounding to 0 for the control window
    ImGuiStyle& style = ImGui::GetStyle();
    float prev_window_rounding = style.WindowRounding;
    style.WindowRounding = 0.0f;

    // Position the control window just below the menu bar
    float menu_bar_height = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(ImVec2(0, menu_bar_height), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(MENU_WINDOW_WIDTH, 0), ImGuiCond_Always);

    // Window flags: no collapse, no title bar, no resize
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;

    if (ImGui::Begin("###Menu", nullptr, window_flags)) {
        // Render sections in the control window
        render_camera_selection();
        render_background_toggle();
        render_volume_render_mode();
        render_scene_models();
    }
    ImGui::End();

    // Restore previous window rounding style
    style.WindowRounding = prev_window_rounding;
}

void Overlay::render_camera_selection() {
    ImGui::Text("Static Camera View:");

    int num_views = static_cast<int>(project_->views.size());
    for (int i = 0; i < num_views; ++i) {
        // Place buttons on the same line except for the first one
        if (i > 0) {
            ImGui::SameLine();
        }

        bool is_current = (i == active_camera_view_);

        // Highlight the active camera view button
        if (is_current) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        }

        // Disable button if project is not initialized
        BeginDisabledIf(!project_->initialized);

        // Button label is the view index + 1 (1-based)
        std::string label = std::to_string(i + 1);
        if (ImGui::Button(label.c_str(), CAMERA_BUTTON_SIZE)) {
            active_camera_view_ = i;
            if (camera_) {
                camera_->set_view(i);
            }
        }

        EndDisabledIf(!project_->initialized);

        if (is_current) {
            ImGui::PopStyleColor();
        }
    }
    ImGui::Separator();
}

void Overlay::render_background_toggle() {
    // Background overlay toggle, only enabled in static camera views
    bool in_static_view = camera_ && camera_->in_static_view();
    bool can_toggle_background = project_->initialized && in_static_view;

    BeginDisabledIf(!can_toggle_background);

    bool prev_background = show_background_;
    if (ImGui::Checkbox("Background", &show_background_)) {
        if (renderer_ && show_background_ != prev_background) {
            renderer_->toggle_background();
        }
    }

    EndDisabledIf(!can_toggle_background);

    if (!can_toggle_background) {
        ImGui::TextDisabled("(Only available in static camera views)");
    }

    ImGui::Separator();
}

void Overlay::render_volume_render_mode() {
    ImGui::Text("Volume Render Mode:");

    // Radio buttons for selecting volume render mode
    int current_mode = volume_render_mode_;
    const char* modes[] = { "Points", "Voxels" };

    for (int i = 0; i < 2; ++i) {
        if (i > 0) ImGui::SameLine();
        if (ImGui::RadioButton(modes[i], current_mode == i)) {
            if (volume_render_mode_ != i) {
                volume_render_mode_ = i;
                if (renderer_) {
                    renderer_->toggle_volume_render_mode();
                }
            }
        }
    }

    ImGui::Separator();
}

void Overlay::render_scene_models() {
    ImGui::Text("Scene Models:");

    // Iterate over each toggleable scene item and render a checkbox
    for (const auto& item : toggle_items_) {
        bool previous_state = *(item.state);

        // Render checkbox for the item
        if (ImGui::Checkbox(item.label, item.state)) {
            // If the state changed, call the associated toggle function
            if (*(item.state) != previous_state) {
                item.toggle_func();
            }
        }
    }
}

void Overlay::reset_ui_state(bool project_loaded) {
    show_box_ = true;
    show_frame_ = true;
    show_floor_ = true;
    show_volume_ = project_loaded;
    show_checkers_ = true;
    show_frustums_ = true;
    show_background_ = false;
    
    active_camera_view_ = DEFAULT_CAMERA_VIEW;
    volume_render_mode_ = DEFAULT_VOLUME_RENDER_MODE;
}

void Overlay::set_error_popup(const std::string& message) {
    error_popup_message_ = message;
    error_popup_open_ = true;
}
