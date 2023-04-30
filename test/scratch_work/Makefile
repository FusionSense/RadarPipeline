CXX = g++
CXXFLAGS = -Wall
LDFLAGS = -lfftw3f -lm

SRC = scratch_work.cpp
EXE = chicken

all: $(EXE)

$(EXE): $(SRC)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

optimized: CXXFLAGS += -O3
optimized: clean $(EXE)

debug: CXXFLAGS += -g
debug: clean $(EXE)

clean:
	rm -f $(EXE)
