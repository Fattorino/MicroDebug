cmake_minimum_required(VERSION 3.4...3.28)
project(MicroDebug)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp
        MicroDebug.cpp
        MicroDebug.h
        ConsoleWindow.cpp
        ConsoleWindow.h
        DataStream.cpp
        DataStream.h
        PlotWindow.cpp
        PlotWindow.h)

add_subdirectory(includes/glfw-3.4)
add_subdirectory(includes/imgui-1.91.8-docking)
add_subdirectory(includes/implot)
add_subdirectory(includes/serial)
add_subdirectory(includes/bluetooth-serial-port)

target_link_libraries(${PROJECT_NAME}
        ImGui
        ImPlot
        Serial
        bluetoothserialport
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} GL)
endif()
