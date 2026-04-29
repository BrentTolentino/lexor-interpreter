CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -Iinclude

all: lexor

lexor: src/main.o src/Lexer.o src/SymbolTable.o src/Parser.o
	$(CXX) $(CXXFLAGS) -o lexor src/main.o src/Lexer.o src/SymbolTable.o src/Parser.o

src/main.o: src/main.cpp include/Parser.h include/Lexer.h
	$(CXX) $(CXXFLAGS) -c src/main.cpp -o src/main.o

src/Lexer.o: src/Lexer.cpp include/Lexer.h include/Token.h
	$(CXX) $(CXXFLAGS) -c src/Lexer.cpp -o src/Lexer.o

src/SymbolTable.o: src/SymbolTable.cpp include/SymbolTable.h include/Token.h
	$(CXX) $(CXXFLAGS) -c src/SymbolTable.cpp -o src/SymbolTable.o

src/Parser.o: src/Parser.cpp include/Parser.h include/SymbolTable.h include/Token.h
	$(CXX) $(CXXFLAGS) -c src/Parser.cpp -o src/Parser.o

clean:
	rm -f src/*.o lexor