#pragma once

#include <string>
#include <vector>
#include <filesystem>

#include "view.hpp"


constexpr const int CHESS_COLS = 7;
constexpr const int CHESS_ROWS = 7;
constexpr const float CHESS_SQUARE = 22.0f;  // mm
constexpr const float CHESS_PADDING = 100.0f;


/**
 * @brief Represents a camera calibration project.
 *
 * This struct holds all relevant data for a calibration project, including its state,
 * file paths, chessboard configuration, and associated views.
 *
 * @note The chessboard configuration uses default values defined by CHESS_COLS, CHESS_ROWS, and CHESS_SQUARE.
 *
 * Members:
 * - empty: Indicates whether the project is empty.
 * - initialized: Indicates whether the project has been initialized.
 * - needs_calibration: Indicates whether calibration is required.
 * - name: The name of the project.
 * - file: The full path to the project file.
 * - dir: The full path to the project directory.
 * - chess_cols: Number of columns in the chessboard.
 * - chess_rows: Number of rows in the chessboard.
 * - square_size: Size of a chessboard square in millimeters.
 * - views: Collection of views containing calibration data.
 */
struct Project {
    bool empty = true;                              // Whether the project is empty
    bool initialized = false;                       // Whether the project is initialized
    bool needs_calibration = false;                 // Whether calibration is needed

    std::string name;                               // Name of the project
    std::filesystem::path file;                     // Full path to the project file
    std::filesystem::path dir;                      // Full path to the project directory

    int chess_cols = CHESS_COLS;                    // Number of columns in the chessboard
    int chess_rows = CHESS_ROWS;                    // Number of rows in the chessboard
    float square_size = CHESS_SQUARE;               // Size of a square in mm

    std::vector<View> views;                        // Views with calibration data
};
