.PHONY: clean print

CXX:=clang++
CXXFLAGS+=-g -O0 -std=c++17
LDFLAGS:=

CXX_SOURCES:=$(wildcard src/*.cpp)
CXX_OBJECTS:=$(patsubst src/%.cpp,build/%.o,$(CXX_SOURCES))


all: clean main

build/%.o: src/%.cpp
	$(CXX) $< $(CXXFLAGS) -c -o $@

main: $(CXX_OBJECTS)
	$(CXX) $< $(LDFLAGS) -o $@

print:
	echo $(CXX_SOURCES)
	echo $(CXX_OBJECTS)

clean:
	-rm $(CXX_OBJECTS)
