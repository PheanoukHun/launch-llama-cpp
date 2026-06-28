# Makefile for launch-llama

CXX = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -O2 -I/usr/include/yaml-cpp

# Find source files
SRCS := $(wildcard src/*.cpp)
OBJS := $(SRCS:.cpp=.o)

TARGET = build/launch-llama

all: $(TARGET)

$(TARGET): $(OBJS) | build
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) -lnotcurses -lyaml-cpp -lstdc++

build:
	mkdir -p build

clean:
	rm -rf $(OBJS) $(TARGET)

.PHONY: all clean
