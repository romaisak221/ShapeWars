cmake_minimum_required(VERSION 3.10)
project(ShapeWars)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
file(GLOB SRC_FILES src/*.cpp)

# Include headers
include_directories(include)

# Find SFML
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

add_executable(ShapeWars ${SRC_FILES})

target_link_libraries(ShapeWars sfml-graphics sfml-window sfml-system sfml-audio)
