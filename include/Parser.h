#ifndef PARSER_H
#define PARSER_H

#include "Token.h"
#include "SymbolTable.h"
#include <vector>
#include <string>
#include <memory>

// Forward declarations
class Lexer;
struct Token;

// ---------------------------------------------------------------------------
// Parser — builds an Abstract Syntax Tree (AST) from a token stream
//         and simulates execution of LEXOR programs
//
// Usage:
//   Lexer lexer(sourceCode);
//   auto tokens = lexer.tokenize();
//   Parser parser(tokens);
//   parser.parse();  // validates and executes the program
// ---------------------------------------------------------------------------
class Parser
{
public:
    // Initialize parser with a token stream.
    explicit Parser(const std::vector<Token> &tokens);

    // Main entry point — parse and validate the entire program structure.
    // Throws std::runtime_error on parse errors.
    void parse();

private:
    // --- Token stream management ---
    std::vector<Token> tokens_;
    size_t pos_;              // current token position
    SymbolTable symbolTable_; // stores variables during execution

    // --- Helper methods ---
    Token peek(int offset = 0) const; // look ahead at token
    Token advance();                  // consume and return current token
    bool isAtEnd() const;
    Token expect(TokenType type);     // consume token of expected type, throw if mismatch
    bool match(TokenType type);       // check if current token matches type
    bool check(TokenType type) const; // peek without consuming

    // --- Parsing methods (recursive descent) ---
    void parseScript();      // main script structure
    void parseStatements();  // parse all statements until END_SCRIPT
    void parseStatement();   // parse a single statement
    void parseDeclaration(); // DECLARE type id [, id]*
    void parseAssignment();  // id = expression
    void parsePrint();       // PRINT: expression [& | $]*
    void parseScan();        // SCAN: id

    // --- Expression evaluation ---
    VarValue evaluateExpression();  // parse and evaluate an expression
    VarValue parseAddition();       // handle + and - operators
    VarValue parseMultiplication(); // handle *, /, and % operators
    VarValue parseConcatenation();  // handle & (concatenation)
    VarValue parsePrimaryValue();   // identifier, literal, or grouped expression

    // --- Utility ---
    std::string tokenTypeToString(TokenType type) const;
    void printValue(const VarValue &value); // print a VarValue to stdout
};

#endif
