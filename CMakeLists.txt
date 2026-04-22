cmake_minimum_required(VERSION 3.10)
project(GPU_Physics_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(SOURCES
    src/main.cpp
    src/game.cpp
    src/Physics.cpp
    src/renderer2d.cpp
    src/shader_util.cpp
    src/UI.cpp
    src/glad.c
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_sdl3.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# Create executable
add_executable(app ${SOURCES})

# Add PROJECT_ROOT definition for shader paths
target_compile_definitions(app PRIVATE
    PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/"
)

# Include directories
target_include_directories(app PRIVATE
    /usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

# Library directories
target_link_directories(app PRIVATE
    /usr/local/lib
)

# Link libraries
target_link_libraries(app PRIVATE
    SDL3
    GL
)
