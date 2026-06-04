#include "Lexer.h"
#include <stdexcept>
#include <cctype>
#include <sstream>

// ---------------------------------------------------------------------------
// Static keyword table
// Single-word reserved words only. Multi-word are handled in
// tryMatchMultiwordKeyword().
// ---------------------------------------------------------------------------
const std::unordered_map<std::string, TokenType> Lexer::KEYWORDS = {
    {"DECLARE", TokenType::DECLARE},
    {"PRINT", TokenType::PRINT},
    {"SCAN", TokenType::SCAN},
    {"INT", TokenType::TYPE_INT},
    {"CHAR", TokenType::TYPE_CHAR},
    {"BOOL", TokenType::TYPE_BOOL},
    {"FLOAT", TokenType::TYPE_FLOAT},
    {"IF", TokenType::IF},
    {"ELSE", TokenType::ELSE},
    {"FOR", TokenType::FOR},
    {"AND", TokenType::AND},
    {"OR", TokenType::OR},
    {"NOT", TokenType::NOT},
    {"TRUE", TokenType::BOOL_LITERAL},
    {"FALSE", TokenType::BOOL_LITERAL},
};

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------
Lexer::Lexer(const std::string &source)
    : source_(source), pos_(0), line_(1) {}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
bool Lexer::isAtEnd() const
{
    return pos_ >= source_.size();
}

char Lexer::peek(int offset) const
{
    size_t index = pos_ + static_cast<size_t>(offset);
    if (index >= source_.size())
        return '\0';
    return source_[index];
}

char Lexer::advance()
{
    return source_[pos_++];
}

void Lexer::skipWhitespace()
{
    while (!isAtEnd() && (peek() == ' ' || peek() == '\t' || peek() == '\r'))
    {
        advance();
    }
}

void Lexer::skipComment()
{
    // Called after detecting %% — consume everything until newline
    while (!isAtEnd() && peek() != '\n')
    {
        advance();
    }
}

// ---------------------------------------------------------------------------
// Multi-word keyword matching
//
// Called when we have already read `firstWord` and want to check whether the
// next non-space word forms a known two-word keyword.
// If it matches, pos_ is advanced past the second word and outType is set.
// ---------------------------------------------------------------------------
bool Lexer::tryMatchMultiwordKeyword(const std::string &firstWord,
                                     TokenType &outType)
{
    // Save position so we can backtrack if no match
    size_t savedPos = pos_;

    // Skip spaces/tabs between the two words (no newlines — that would be a
    // different statement)
    while (!isAtEnd() && (peek() == ' ' || peek() == '\t'))
    {
        advance();
    }

    // Read the next word
    std::string secondWord;
    while (!isAtEnd() && (std::isalpha(peek()) || peek() == '_'))
    {
        secondWord += advance();
    }

    // Check known two-word combinations
    std::string combined = firstWord + " " + secondWord;

    if (combined == "SCRIPT AREA")
    {
        outType = TokenType::SCRIPT_AREA;
        return true;
    }
    if (combined == "START SCRIPT")
    {
        outType = TokenType::START_SCRIPT;
        return true;
    }
    if (combined == "END SCRIPT")
    {
        outType = TokenType::END_SCRIPT;
        return true;
    }
    if (combined == "START IF")
    {
        outType = TokenType::START_IF;
        return true;
    }
    if (combined == "END IF")
    {
        outType = TokenType::END_IF;
        return true;
    }
    if (combined == "START FOR")
    {
        outType = TokenType::START_FOR;
        return true;
    }
    if (combined == "END FOR")
    {
        outType = TokenType::END_FOR;
        return true;
    }
    if (combined == "START REPEAT")
    {
        outType = TokenType::START_REPEAT;
        return true;
    }
    if (combined == "END REPEAT")
    {
        outType = TokenType::END_REPEAT;
        return true;
    }
    if (combined == "REPEAT WHEN")
    {
        outType = TokenType::REPEAT_WHEN;
        return true;
    }
    if (combined == "ELSE IF")
    {
        outType = TokenType::ELSE_IF;
        return true;
    }

    // No match — backtrack
    pos_ = savedPos;
    return false;
}

// ---------------------------------------------------------------------------
// Keyword lookup (single-word fallback)
// ---------------------------------------------------------------------------
TokenType Lexer::lookupKeyword(const std::string &word) const
{
    auto it = KEYWORDS.find(word);
    if (it != KEYWORDS.end())
        return it->second;
    return TokenType::IDENTIFIER;
}

// ---------------------------------------------------------------------------
// Literal scanners
// ---------------------------------------------------------------------------
Token Lexer::scanString()
{
    // Opening " already consumed by caller
    int startLine = line_;
    std::string value;
    while (!isAtEnd() && peek() != '"')
    {
        if (peek() == '\n')
        {
            throw std::runtime_error("Unterminated string literal at line " + std::to_string(startLine));
        }
        value += advance();
    }
    if (isAtEnd())
    {
        throw std::runtime_error("Unterminated string literal at line " + std::to_string(startLine));
    }
    advance(); // closing "
    // --- ADDED FIX: Check if the string literal is actually a BOOL literal ---
    if (value == "TRUE" || value == "FALSE")
    {
        return Token(TokenType::BOOL_LITERAL, value, startLine);
    }
    return Token(TokenType::STRING_LITERAL, value, startLine);
}

Token Lexer::scanChar()
{
    // Opening ' already consumed by caller
    int startLine = line_;
    if (isAtEnd() || peek() == '\n')
    {
        throw std::runtime_error("Unterminated char literal at line " + std::to_string(startLine));
    }
    char ch = advance();
    if (isAtEnd() || peek() != '\'')
    {
        throw std::runtime_error("CHAR literal must be exactly one character, line " + std::to_string(startLine));
    }
    advance(); // closing '
    return Token(TokenType::CHAR_LITERAL, std::string(1, ch), startLine);
}

Token Lexer::scanNumber()
{
    int startLine = line_;
    std::string value;
    bool isFloat = false;

    while (!isAtEnd() && std::isdigit(peek()))
    {
        value += advance();
    }

    if (!isAtEnd() && peek() == '.' && std::isdigit(peek(1)))
    {
        isFloat = true;
        value += advance(); // consume '.'
        while (!isAtEnd() && std::isdigit(peek()))
        {
            value += advance();
        }
    }

    TokenType type = isFloat ? TokenType::FLOAT_LITERAL : TokenType::INT_LITERAL;
    return Token(type, value, startLine);
}

Token Lexer::scanIdentifierOrKeyword()
{
    int startLine = line_;
    std::string word;

    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_'))
    {
        word += advance();
    }

    // Try multi-word keywords first (e.g. "START SCRIPT")
    TokenType multiType;
    if (tryMatchMultiwordKeyword(word, multiType))
    {
        return Token(multiType, word, startLine); // value is just the first word;
                                                  // parser uses the type, not the value
    }

    // Fall back to single-word keyword or identifier
    TokenType type = lookupKeyword(word);
    return Token(type, word, startLine);
}

// ---------------------------------------------------------------------------
// tokenize() — main loop
// ---------------------------------------------------------------------------
std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (!isAtEnd())
    {
        skipWhitespace();

        if (isAtEnd())
            break;

        char c = peek();

        // --- Newline ---
        if (c == '\n')
        {
            tokens.emplace_back(TokenType::NEWLINE, "\\n", line_);
            advance();
            line_++;
            continue;
        }

        // --- Comment detection: %% ---
        if (c == '%' && peek(1) == '%')
        {
            skipComment();
            continue;
        }

        // --- String literal ---
        if (c == '"')
        {
            advance(); // consume opening "
            tokens.push_back(scanString());
            continue;
        }

        // --- Char literal ---
        if (c == '\'')
        {
            advance(); // consume opening '
            tokens.push_back(scanChar());
            continue;
        }

        // --- Number ---
        if (std::isdigit(c))
        {
            tokens.push_back(scanNumber());
            continue;
        }

        // --- Identifier or keyword (starts with letter or underscore) ---
        if (std::isalpha(c) || c == '_')
        {
            tokens.push_back(scanIdentifierOrKeyword());
            continue;
        }

        // --- Operators and symbols ---
        advance(); // consume the character we're about to classify

        switch (c)
        {
        case '+':
            tokens.emplace_back(TokenType::PLUS, "+", line_);
            break;
        case '-':
            tokens.emplace_back(TokenType::MINUS, "-", line_);
            break;
        case '*':
            tokens.emplace_back(TokenType::MULTIPLY, "*", line_);
            break;
        case '/':
            tokens.emplace_back(TokenType::DIVIDE, "/", line_);
            break;

        // Single % (not %%) — modulo operator
        case '%':
            tokens.emplace_back(TokenType::MODULO, "%", line_);
            break;

        case '&':
            tokens.emplace_back(TokenType::AMPERSAND, "&", line_);
            break;
        case '$':
            tokens.emplace_back(TokenType::DOLLAR, "$", line_);
            break;
        case ':':
            tokens.emplace_back(TokenType::COLON, ":", line_);
            break;
        case ',':
            tokens.emplace_back(TokenType::COMMA, ",", line_);
            break;
        case '(':
            tokens.emplace_back(TokenType::LPAREN, "(", line_);
            break;
        case ')':
            tokens.emplace_back(TokenType::RPAREN, ")", line_);
            break;
        case '[':
        {
            if (isAtEnd())
            {
                throw std::runtime_error("Unterminated escape sequence at line " + std::to_string(line_));
            }

            // Read the single character that is being escaped
            char escapedChar = advance();

            // Ensure it is immediately followed by the closing bracket
            if (isAtEnd() || peek() != ']')
            {
                throw std::runtime_error("Invalid escape sequence or missing ']' at line " + std::to_string(line_));
            }
            advance(); // Consume the closing ']'

            // Emit it as a standard STRING_LITERAL so the Parser can easily print/concatenate it
            tokens.emplace_back(TokenType::STRING_LITERAL, std::string(1, escapedChar), line_);
            break;
        }
        case ']':
            tokens.emplace_back(TokenType::RBRACKET, "]", line_);
            break;
        case '>':
            if (!isAtEnd() && peek() == '=')
            {
                advance();
                tokens.emplace_back(TokenType::GREATER_EQ, ">=", line_);
            }
            else
            {
                tokens.emplace_back(TokenType::GREATER, ">", line_);
            }
            break;

        case '<':
            if (!isAtEnd() && peek() == '=')
            {
                advance();
                tokens.emplace_back(TokenType::LESS_EQ, "<=", line_);
            }
            else if (!isAtEnd() && peek() == '>')
            {
                advance();
                tokens.emplace_back(TokenType::NOT_EQUAL, "<>", line_);
            }
            else
            {
                tokens.emplace_back(TokenType::LESS, "<", line_);
            }
            break;

        case '=':
            if (!isAtEnd() && peek() == '=')
            {
                advance();
                tokens.emplace_back(TokenType::EQUAL, "==", line_);
            }
            else
            {
                tokens.emplace_back(TokenType::ASSIGN, "=", line_);
            }
            break;

        default:
            tokens.emplace_back(TokenType::UNKNOWN, std::string(1, c), line_);
            break;
        }
    }

    tokens.emplace_back(TokenType::END_OF_FILE, "", line_);
    return tokens;
}