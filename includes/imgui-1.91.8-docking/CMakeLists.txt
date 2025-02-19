cmake_minimum_required(VERSION 3.4...3.28)
project(ImGuiLibrary)

set(CMAKE_CXX_STANDARD 17)

add_library(ImGui STATIC
        imgui.cpp
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        backends/imgui_impl_glfw.cpp
        backends/imgui_impl_opengl3.cpp
        misc/cpp/imgui_stdlib.cpp
)

target_link_libraries(ImGui glfw)

target_include_directories(ImGui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/backends
        ${CMAKE_CURRENT_SOURCE_DIR}/misc/cpp
)
