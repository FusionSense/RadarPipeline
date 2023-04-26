cmake_minimum_required(VERSION 3.5)
project( RadarPipeline )

# Add the include directory to the include path
include_directories(${CMAKE_SOURCE_DIR}/src/rpl)

# Add the subdirectories
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(demo)