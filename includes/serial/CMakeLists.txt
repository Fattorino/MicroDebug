cmake_minimum_required(VERSION 3.4...3.28)
set(CMAKE_CXX_STANDARD 17)

# CREATE PROJECT
project(Serial)

# SET SOURCE FILES FOR PROJECT
file(GLOB_RECURSE _HDRS "include/*.h")
file(GLOB_RECURSE _SRCS "src/*.cc")

# CREATE LIBRARY FROM SOURCE_FILES
add_library(${PROJECT_NAME} ${_SRCS} ${_HDRS})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} setupapi)
endif()

# ADD TO INCLUDE DIRECTORIES
target_include_directories(Serial PUBLIC include)
