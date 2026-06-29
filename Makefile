CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2
LDFLAGS  = -lnotcurses -lnotcurses-core -lyaml-cpp

SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)

TARGET = build/launch-llama

all: $(TARGET)

$(TARGET): $(OBJS) | build
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

build:
	mkdir -p build

test: all
	$(CXX) $(CXXFLAGS) -o build/integration_test tests/integration_test.cpp
	./build/integration_test

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean test
