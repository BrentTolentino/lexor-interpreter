CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude

all: lexor

lexor: src/main.o
	$(CXX) $(CXXFLAGS) -o lexor src/main.o

src/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o src/main.o

clean:
	rm -f src/*.o lexor
