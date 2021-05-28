cmake_minimum_required(VERSION 3.19)
project(imgui_font_atlas_generator)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(glfw)
find_package(OpenGL REQUIRED)

include_directories(glfw/include)
include_directories(imgui)
include_directories(imgui/backends)
include_directories(${OPENGL_INCLUDE_DIRS})

add_definitions(-O1 -g)

add_executable(imgui_font_atlas_generator
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_demo.cpp
        imgui/backends/imgui_impl_glfw.cpp
        imgui/backends/imgui_impl_opengl2.cpp
        font_atlas.hpp
        main.cpp)

target_link_libraries(imgui_font_atlas_generator
        ${OPENGL_LIBRARIES}
        glfw
        ${GLFW_LIBRARIES})