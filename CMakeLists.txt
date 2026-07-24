cmake_minimum_required(VERSION 3.16)
project(SudokuSolver VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Find SFML ─────────────────────────────────────────────────────────────────
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# ── Sources ───────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.cpp
    src/SudokuGrid.cpp
    src/Solver.cpp
    src/GUIApp.cpp
)

add_executable(sudoku ${SOURCES})

target_include_directories(sudoku PRIVATE src)

target_link_libraries(sudoku
    sfml-graphics
    sfml-window
    sfml-system
)

# ── Copy assets next to the binary ────────────────────────────────────────────
add_custom_command(TARGET sudoku POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:sudoku>/assets
    COMMENT "Copying assets..."
)

# ── Compiler warnings ─────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(sudoku PRIVATE /W4)
else()
    target_compile_options(sudoku PRIVATE -Wall -Wextra -Wpedantic)
endif()
