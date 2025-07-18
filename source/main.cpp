#include "app.hpp"

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif


int main(int argc, char* argv[]) {
    try {
        App app(argc, argv);
        app.run();
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
#ifdef _WIN32
        MessageBoxA(nullptr, e.what(), "Error", MB_OK | MB_ICONERROR);
#endif
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception occurred." << std::endl;
#ifdef _WIN32
        MessageBoxA(nullptr, "Unknown exception occurred.", "Error", MB_OK | MB_ICONERROR);
#endif
        return 1;
    }
}
