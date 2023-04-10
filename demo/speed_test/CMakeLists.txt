cmake_minimum_required(VERSION 3.2)
project( speed_test )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Include directories
include_directories("${CMAKE_SOURCE_DIR}/../"
    "${CMAKE_SOURCE_DIR}/../../"
    "${CMAKE_SOURCE_DIR}/../../network"
    "${CMAKE_SOURCE_DIR}/../../src"
    "${CMAKE_SOURCE_DIR}/../../src/rpl")

# Define the variable
set(CONFIG "A" CACHE STRING "Which configuration the executables are compiled for.")
set(IP "127.0.0.1" CACHE STRING "Server IP address.")
set(SERVER_PARAMS "network/server.hpp" CACHE STRING "Server parameters.")
set(CLIENT_PARAMS "network/node_${CONFIG}.hpp" CACHE STRING "Client parameters.")

# Pass the variable to the source code
add_definitions(-DCONFIG="${CONFIG}")
add_definitions(-DIP="${IP}")
add_definitions(-DSERVER_PARAMS="${SERVER_PARAMS}")
add_definitions(-DCLIENT_PARAMS="${CLIENT_PARAMS}")

# Check for correct input
if(((CONFIG STREQUAL "A") OR (CONFIG STREQUAL "B") OR (CONFIG STREQUAL "C")))
    message(STATUS "Configuration:\tspeed_test for node ${CONFIG}")
    message(STATUS "Server IP Address:\t${IP}")
else()
    message(FATAL_ERROR "You must enter a correct node (A,B,C).")
endif()

# Link libraries
add_executable( server server.cpp )
add_executable( client client.cpp )
target_link_libraries( server ${OpenCV_LIBS} )
target_link_libraries( client ${OpenCV_LIBS} )
