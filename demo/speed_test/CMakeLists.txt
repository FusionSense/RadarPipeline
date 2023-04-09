cmake_minimum_required(VERSION 3.2)
project( speed_test )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

# Define the variable
set(CONFIG "A" CACHE STRING "Which configuration the executables are compiled for.")
set(IP "127.0.0.1" CACHE STRING "Server IP address.")

# Pass the variable to the source code
add_definitions(-DCONFIG="${CONFIG}")
add_definitions(-DIP="${IP}")

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
