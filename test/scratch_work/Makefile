# Compiler and flags
CXX=g++
# CXXFLAGS are compiler flags for the C++ compiler
# -Wall: turn on all warnings
# -Wextra: turn on even more warnings
# -pedantic: enforce stricter C++ rules
# -std=c++11: use the C++11 standard
CXXFLAGS=-Wall -Wextra -pedantic -std=c++11
# LDFLAGS are linker flags
# -lfftw3f: link with the FFTW3 library (f is for floats instead of the default double library)
# -lm: link with the math library (for the -lm flag)
LDFLAGS=-lfftw3f -lm

# Files and directories
# SRC is the name of the C++ source file
SRC=scratch_work.cpp
# EXE is the name of the output binary executable file
EXE=chicken

# Default target
# This is the default target, so running `make` without any arguments will build this target.
all: $(EXE)

# Rule to build executable
# This rule specifies how to build the executable.
# $(EXE) depends on $(SRC), so if $(SRC) has changed since the last build, this rule will be executed.
# $< is the first dependency (in this case, $(SRC)), and $@ is the target (in this case, $(EXE)).
$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $< -o $@ $(LDFLAGS)

# Clean target
# This target removes the executable binary file.
clean:
	rm -f $(EXE)

# Debug target
# This target specifies how to build a debug version of the executable.
debug: CXXFLAGS += -g
debug: $(EXE)

# Optimized target
# This target specifies how to build an optimized version of the executable.
# It depends on the `clean` target, so it will always start with a clean slate.
# It also appends the `-O3` optimization flag to the `CXXFLAGS` variable, which tells the compiler to optimize the code.
optimized:
	$(MAKE) clean
	CXXFLAGS+='-O3' $(MAKE) $(EXE)
