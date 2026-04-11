#ifndef LEXER_H
#define LEXER_H

#include "Token.h"
#include <string>
#include <vector>
#include <unordered_map>

// ---------------------------------------------------------------------------
// Lexer — turns raw LEXOR source text into a flat list of Tokens.
//
// Usage:
//   Lexer lexer(sourceCode);
//   std::vector<Token> tokens = lexer.tokenize();
// ---------------------------------------------------------------------------
class Lexer {
public:
    explicit Lexer(const std::string& source);

    // Main entry point — returns all tokens including END_OF_FILE.
    std::vector<Token> tokenize();

private:
    std::string source_;   // full source text
    size_t      pos_;      // current read position
    int         line_;     // current line number (1-based)

    // --- Low-level helpers ---
    char    peek(int offset = 0) const;   // look ahead without consuming
    char    advance();                     // consume and return current char
    bool    isAtEnd() const;
    void    skipWhitespace();             // spaces and tabs only (not newlines)
    void    skipComment();                // consumes rest of line after %%

    // --- Token scanners ---
    Token   scanString();        // "..."
    Token   scanChar();          // '.'
    Token   scanNumber();        // INT or FLOAT literal
    Token   scanIdentifierOrKeyword();  // identifiers and all reserved words

    // --- Keyword lookup ---
    // Checks if a word is a reserved keyword and returns its TokenType.
    // Returns TokenType::IDENTIFIER if it is not a keyword.
    TokenType lookupKeyword(const std::string& word) const;

    // --- Multi-word keyword helper ---
    // Some LEXOR keywords span two words (e.g. "SCRIPT AREA", "START SCRIPT").
    // This tries to match them starting at pos_ and returns true on success,
    // also consuming the extra word and whitespace.
    bool tryMatchMultiwordKeyword(const std::string& firstWord,
                                  TokenType& outType);

    // Static keyword table built once at construction.
    static const std::unordered_map<std::string, TokenType> KEYWORDS;
};

#endif // LEXER_H