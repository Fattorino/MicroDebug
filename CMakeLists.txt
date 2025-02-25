cmake_minimum_required(VERSION 3.4...3.28)
project(MicroDebug)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(includes/glfw-3.4)
add_subdirectory(includes/imgui-1.91.8-docking)
add_subdirectory(includes/implot)
add_subdirectory(includes/serial)
add_subdirectory(includes/nativefiledialog-extended-1.2.1)
add_subdirectory(includes/bluetooth-serial-port)

add_executable(${PROJECT_NAME} main.cpp
        MicroDebug.cpp
        MicroDebug.h
        ConsoleWindow.cpp
        ConsoleWindow.h
        DataStream.cpp
        DataStream.h
        PlotWindow.cpp
        PlotWindow.h)

target_link_libraries(${PROJECT_NAME}
        ImGui
        ImPlot
        Serial
        nfd
        bluetoothserialport
)

if (WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif (UNIX)
    target_link_libraries(${PROJECT_NAME} GL)
endif()

# Somehow I have to add this because its target_include_directories() function doesn't work
#include_directories(includes/nativefiledialog/src/include)
