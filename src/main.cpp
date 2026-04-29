#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: ./lexor <source_file>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open file " << argv[1] << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    try
    {
        // Tokenize the source code
        Lexer lexer(buffer.str());
        std::vector<Token> tokens = lexer.tokenize();

        // Filter out NEWLINE tokens
        std::vector<Token> filteredTokens;
        for (const Token &t : tokens)
        {
            if (t.type != TokenType::NEWLINE)
            {
                filteredTokens.push_back(t);
            }
        }

        // Parse and execute the program
        Parser parser(filteredTokens);
        parser.parse();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "Script executed successfully!" << std::endl;
    return 0;
}