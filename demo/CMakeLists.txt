cmake_minimum_required( VERSION 3.2 )
project( Demo )

# Set C++ standard
set( CMAKE_CXX_STANDARD 14 )

# Include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../"
    "${CMAKE_CURRENT_SOURCE_DIR}/../"
    "${CMAKE_CURRENT_SOURCE_DIR}/../network"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src"
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/rpl")

# Include libraries
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Add subdirectory for demo
add_subdirectory( speed_test )