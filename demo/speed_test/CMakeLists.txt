cmake_minimum_required(VERSION 3.2)
project( SpeedTest )

# Set variables
set( SOURCES test.cpp )

# Add definitions
# code here

# Include directories

# Create executable
add_executable(test ${SOURCES})

# Link libraries
target_link_libraries(test PRIVATE Demo)