CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
INCLUDES = -I../include
LIBS = -L../build -lswarmcog_core -lpthread

demo: basic_demo.cpp
basic_demo.cpp $(CXX) $(CXXFLAGS) $(INCLUDES) -o demo basic_demo.cpp $(LIBS)

clean:
basic_demo.cpp rm -f demo

.PHONY: clean
