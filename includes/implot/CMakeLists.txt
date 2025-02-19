cmake_minimum_required(VERSION 3.4...3.28)
project(ImPlotLibrary)

set(CMAKE_CXX_STANDARD 17)

add_library(ImPlot STATIC
        implot.cpp
        implot_items.cpp
        implot_demo.cpp
        implot_internal.h
        implot.h
)

target_link_libraries(ImPlot ImGui)

target_include_directories(ImPlot PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
