# Compiler and flags
CXX = g++
# CXXFLAGS are compiler flags for the C++ compiler
# -Wall: turn on all warnings
# -Wextra: turn on even more warnings
# -pedantic: enforce stricter C++ rules
# -std=c++11: use the C++11 standard
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
# LDFLAGS are linker flags
# -I../../src/ include header files from the source directory
# -lfftw3f: link with the FFTW3 librar (f is for floats instead of the default double library)
# -lm: link with the math library
LDFLAGS = -I../../src/ -lfftw3f -lm `pkg-config --cflags --libs opencv4`

# Files and directories
# SRC is the name of the C++ source file
SRC = test.cpp
# EXE is the name of the output binary executable file
EXE = test



.PHONY: all clean debug profile optimized


# Default target
# This is the deault target, so running `make` without any arguments will build this target.
all: $(EXE)

# Rule to build exectuable
# This rule specifies how to build the executable.
# $(EXE) depends of $(SRC), so if $(SRC) has changed since the last build, this rule will be executed.
# $< is the first dependency (in this case, $(SRC)), and $@ is the target (in this case, $(EXE)).
$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Clear target
# This target removes the executable binary file.
clean:
	rm -f $(OBJS) $(EXE)

# Debug target
# This target specifies how to build a debug version of the executable
debug: CXXFLAGS += -g
debug: $(EXE)

profile: CXXFLAGS +=-g -pg
profile: $(EXE)

# Optimized target
# This target specifies how to build and optimized of the exectuable.
# It depends on the `clean` target, so it will always start with a clean slate
# It also appends the `-O3` optimization flag to the `CXXFLAGS` variable, which tells the compiler to optimize the code.
optimized:
	$(MAKE) clean
	CXXFLAGS+='-O3' $(MAKE) $(EXE)

