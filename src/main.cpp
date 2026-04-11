#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./lexor <source_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    Lexer lexer(buffer.str());
    std::vector<Token> tokens = lexer.tokenize();

    for (const Token& t : tokens) {
        std::cout << "[Line " << t.line << "] "
                  << "Type: " << static_cast<int>(t.type)
                  << "  Value: \"" << t.value << "\"" << std::endl;
    }

    return 0;
}