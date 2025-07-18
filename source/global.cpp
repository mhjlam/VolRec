#include "global.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>

#include <GL/glew.h>


std::string get_executable_dir() {
    char buffer[MAX_PATH];
    ::GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string path(buffer);
    size_t pos = path.find_last_of("\\/");
    return (std::string::npos == pos) ? "" : path.substr(0, pos);
}

std::string open_project_file_dialog() {
    char filename[MAX_PATH] = {0};
    OPENFILENAMEA ofn = {0};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "JSON Files\0*.json\0All Files\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (::GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
    return {};
}

// OpenGL error checking utility
std::string check_gl_error(const std::string& operation) {
    GLenum error = glGetError();

    if (error != GL_NO_ERROR) {
        switch (error) {
            case GL_INVALID_ENUM: 
                return "GL_INVALID_ENUM"; 
            case GL_INVALID_VALUE: 
                return "GL_INVALID_VALUE"; 
            case GL_INVALID_OPERATION: 
                return "GL_INVALID_OPERATION"; 
            case GL_OUT_OF_MEMORY: 
                return "GL_OUT_OF_MEMORY"; 
            default: 
                return "Unknown error " + error;
        }
    }

    return {};
}
