# VolRec

Volumetric Reconstruction and Visualization

## Overview

`VolRec` is a modern C++ application for volumetric scene reconstruction and visualization, featuring a modular architecture and a fully shader-based OpenGL 4.5 rendering pipeline. It is designed for camera calibration, 3D visualization, and interactive exploration of volumetric data. The application supports reconstructing 3D volumes from multiple camera views and calibration data, enabling users to visualize and analyze volumetric representations derived from 2D images.

## Features

- **OpenGL 4.5 Pipeline**: Modern OpenGL rendering architecture using buffers, textures, and shaders.
- **Volumetric Reconstruction**: Reconstruct 3D volumes from multiple camera views and calibration data, allowing visualization of volumetric data generated from 2D images.
- **Meshes and Volumes**: Supports both mesh-based (box, floor, axes, frustums, checkerboard) and volumetric (point cloud and solid voxels) visualization.
- **Project Loading**: Load calibration projects containing camera views, chessboard settings, and image resources. Projects files in JSON format can be opened and closed via the UI.
- **Interactive User Interface**: ImGui-based overlay for project loading, camera switching, and visualization toggles.

## User Guide

### Controls

**Keyboard:**

- **1-4**: Switch between camera views.
- **A**: Toggle world axes display.
- **B**: Toggle bounding box display.
- **C**: Toggle camera frustums display.
- **F**: Toggle floor grid display.
- **V**: Toggle volume rendering mode (point cloud ↔ solid voxels).
- **ESC**: Quit application.

**Mouse:**

- **Left button**: Rotate camera view around world origin.
- **Right button**: Zoom camera view in and out.

### Running the Application

1. **Build**:

   ```bash
   cmake --build build --config Debug
   build/Debug/VolRec.exe [--project <project.json>]
   ```

2. **Launching**:
   - When first launched, `VolRec` starts with an empty project by default. No calibration or scene data is loaded until you open or create a project.
   - You can load a project at startup by passing the `--project <project.json>` argument, or use the UI to open a project file after launch.

3. **Project files**:
   - Project files are in JSON format and contain calibration settings, chessboard configuration, and references to image resources.
   - Example project file:

     ```json
     {
       "name": "Example Project",
       "chess_cols": 7,
       "chess_rows": 7,
       "square_size": 22.0,
       "views": [
        { "bg_path": "bg1.jpg", "fg_path": "fg1.png", "cb_path": "cb1.yml" },
        { "bg_path": "bg2.jpg", "fg_path": "fg2.png", "cb_path": "cb2.yml" },
        { "bg_path": "bg3.jpg", "fg_path": "fg3.png", "cb_path": "cb3.yml" },
        { "bg_path": "bg4.jpg", "fg_path": "fg4.png", "cb_path": "cb4.yml" },
       ]
     }
     ```

   - Each view entry specifies the background, foreground, and chessboard calibration data for a camera. At least 4 are needed, but more views are allowed.

4. **Program arguments**:

   - `--project <file>`: Load the specified project file at startup (can also be given as the first positional argument).
   - `-f, --force-calibration`: Force camera calibration on project load.
   - `-h, --help`: Print usage information and exit.

## Architecture

- `App`: Main application class, manages window, input, and core components.
- `Scene`: Manages all 3D models (Box, Floor, Frame, Frustum, Volume, Checkers) and their relationships.
- `Model`: Abstract base class for all renderable objects, supporting both mesh-based and volume-based models.
- `Renderer`: Handles all OpenGL calls, manages shaders, framebuffers, and rendering state. Supports toggling of scene elements and volume render modes.
- `Camera`: Manages camera state, calibration, and view switching. Supports both static and interactive camera modes.
- `Overlay`: ImGui-based UI for project management, camera selection, and visualization toggles.
- `Input`: Handles keyboard and mouse events, including passthrough for critical shortcuts even when UI is focused.
- `View`: Data structure for per-view camera/image calibration and render data.
- `Project`: Data structure for `VolRec` project, including chessboard configuration and references to `View`s.
- `Shader`: OpenGL wrapper for a shader program.
- `Buffer`: OpenGL wrapper for a buffer (index, vertex, etc).
- `Mesh`: OpenGL wrapper for a renderable polygon mesh.
- `Texture`: OpenGL wrapper for 2D texture.

### Volumetric Reconstruction

Volumetric reconstruction is implemented using a flexible, GPU-accelerated architecture:

- **Volume**: 3D volumetric data is represented as a collection of voxels. Besides volumetric reconstruction from calibration data, procedurally generated shapes (e.g., spheres, cubes, planes) are also supported.
- **Voxel**: Voxels are stored in efficient data structures and uploaded to the GPU for rendering. Each voxel can store color and occupancy information.
- **Camera**: When a project is loaded, the volume is reconstructed from multiple camera views and calibration data, supporting 3D scene reconstruction from 2D images.
- **Shaders**: The renderer supports two main visualization modes implemented using shaders:
  - **Point Cloud**: Voxels are rendered as a point cloud for a lightweight, sparse visualization.
  - **Solid Voxels**: Voxels are rendered as cubes for a solid, blocky appearance. This uses instanced rendering for performance.

### Program Execution

1. **Initialization**: App initializes required OpenGL functionality and all core components. Scene and Renderer are set up with a default (empty) or project-loaded state.
2. **Project Loading**: Loads calibration data, chessboard settings, and images. Scene creates and configures all models accordingly.
3. **Calibration**: Loads images and camera intrinsics to determine the locations of the cameras and volume in the scene.
4. **Rendering Loop**: Renderer draws all enabled scene elements using modern OpenGL. Volume can be rendered as points or cubes.
5. **User Interaction**: Input is handled for navigation, toggling, and camera switching. Overlay provides UI for project and visualization control.

## Directories

- `build/` — CMake build output.
- `example/` — Example projects (including json file, images, calibration data).
- `source/` — Main C++ source code.
- `shaders/` — GLSL shader programs for all rendering modes.

## Libraries

- [GLFW 3.4](https://www.glfw.org/)
- [GLEW 2.2](http://glew.sourceforge.net/)
- [GLM 1.0](https://github.com/g-truc/glm)
- [OpenCV 4.11](https://opencv.org/)
- [OpenMP 2.0](https://www.openmp.org/)
- [ImGui 1.91](https://github.com/ocornut/imgui)
- [nlohmann-json 3.11](https://github.com/nlohmann/json)
- [cxxopts 3.3](https://github.com/jarro2783/cxxopts)

## License

This software is licensed under the [CC BY-NC-SA](https://creativecommons.org/licenses/by-nc-sa/4.0/) license.
