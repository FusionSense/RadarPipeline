# Source Code

There are a few working demonsrations that can be run:

1. `speed_test`: test the transmit speed between a node and the server
2. `single_node`: displays range doppler map on node and transmits to the server
3. `multi_node`: ...ß

## Building Demos
In order to build the executables for this project, you must use [CMake](https://cmake.org/). If you are using MacOS, [Homebrew](https://brew.sh/) is the best way to achieve this. CMake is already installed on each node. CMake allows for... using a `CMakeLists.txt` file. To build the `server` and `client` executables, run the follow commands from the correct demo directory:
```
mkdir build
cd build
cmake ../
make
```