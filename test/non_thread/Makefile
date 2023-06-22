CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic
LDFLAGS = -lfftw3f -pthread -lm `pkg-config --cflags --libs opencv4`

SRCS = test.cpp
OBJS = $(SRCS:.cpp=.o)
EXEC = test

.PHONY: all clean

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -I../../src/ -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I../../src/ -c $< -o $@ $(LDFLAGS)

clean:
	rm -f $(OBJS) $(EXEC)
