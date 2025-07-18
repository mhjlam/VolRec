cmake_minimum_required(VERSION 3.20)
project(VolRec)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable CMake integration with vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Find packages
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glew CONFIG REQUIRED)
find_package(OpenCV CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(OpenMP)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(cxxopts CONFIG REQUIRED)

# Set the source files
set(SOURCE_FILES 
    source/app.cpp
    source/camera.cpp
    source/global.cpp
    source/input.cpp
    source/main.cpp
    source/overlay.cpp
    source/renderer.cpp
    source/scene.cpp

    # Render files
    source/render/buffer.cpp
    source/render/framebuffer.cpp
    source/render/index_buffer.cpp
    source/render/mesh.cpp
    source/render/shader.cpp
    source/render/texture.cpp
    source/render/vertex_array.cpp
    source/render/vertex_buffer.cpp

    # Model files
    source/model/box.cpp
    source/model/checkers.cpp
    source/model/floor.cpp
    source/model/frame.cpp
    source/model/frustum.cpp
    source/model/model.cpp
    source/model/volume.cpp

    # Resource file for application icon
    source/VolRec.rc
)

# Set executable name
add_executable(VolRec ${SOURCE_FILES})

# Add include directories
target_include_directories(VolRec PRIVATE 
    source/
    source/model/
    source/render/
)

# Link libraries
target_link_libraries(VolRec PRIVATE 
    glfw 
    GLEW::GLEW 
    opengl32 
    glm::glm 
    ${OpenCV_LIBS} 
    imgui::imgui 
    nlohmann_json::nlohmann_json
    cxxopts::cxxopts
)

# Link OpenMP if found (cross-platform)
if(OpenMP_CXX_FOUND)
    target_link_libraries(VolRec PRIVATE OpenMP::OpenMP_CXX)
endif()

set_target_properties(VolRec PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_BINARY_DIR}/RelWithDebInfo"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_BINARY_DIR}/MinSizeRel"
)

# Copy frames to the output target directory after build
add_custom_command(TARGET VolRec POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/example/" "$<TARGET_FILE_DIR:VolRec>/example/"
)

# Copy shaders to the output target directory after build
add_custom_command(TARGET VolRec POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders/" "$<TARGET_FILE_DIR:VolRec>/shaders/"
)

add_custom_command(TARGET VolRec POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_SOURCE_DIR}/VolRec.png" "$<TARGET_FILE_DIR:VolRec>/VolRec.png"
)
