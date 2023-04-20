make_minimum_required(VERSION 3.10)

project(RadarPipeline)

# Set C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)

# Find required libraries
find_package(OpenCV REQUIRED)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(demo)
add_subdirectory(test)

