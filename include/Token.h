#ifndef TOKEN_H
#define TOKEN_H

#include <string>

// ---------------------------------------------------------------------------
// TokenType — every token kind the LEXOR language can produce.
// Grouped by category for readability.
// ---------------------------------------------------------------------------
enum class TokenType {
    // --- Program structure keywords ---
    SCRIPT_AREA,        // "SCRIPT AREA"
    START_SCRIPT,       // "START SCRIPT"
    END_SCRIPT,         // "END SCRIPT"

    // --- Declaration keyword ---
    DECLARE,            // "DECLARE"

    // --- Data type keywords ---
    TYPE_INT,           // "INT"
    TYPE_CHAR,          // "CHAR"
    TYPE_BOOL,          // "BOOL"
    TYPE_FLOAT,         // "FLOAT"

    // --- I/O keywords ---
    PRINT,              // "PRINT"
    SCAN,               // "SCAN"

    // --- Control flow keywords ---
    IF,                 // "IF"
    ELSE,               // "ELSE"
    ELSE_IF,            // "ELSE IF"
    START_IF,           // "START IF"
    END_IF,             // "END IF"
    FOR,                // "FOR"
    START_FOR,          // "START FOR"
    END_FOR,            // "END FOR"
    REPEAT_WHEN,        // "REPEAT WHEN"
    START_REPEAT,       // "START REPEAT"
    END_REPEAT,         // "END REPEAT"

    // --- Boolean literals (LEXOR uses "TRUE"/"FALSE") ---
    BOOL_LITERAL,       // "TRUE" or "FALSE"

    // --- Logical operators ---
    AND,                // "AND"
    OR,                 // "OR"
    NOT,                // "NOT"

    // --- Identifiers and value literals ---
    IDENTIFIER,         // variable names, e.g. x, a_1, myVar
    INT_LITERAL,        // e.g. 42, -7
    FLOAT_LITERAL,      // e.g. 3.14
    CHAR_LITERAL,       // e.g. 'c'
    STRING_LITERAL,     // e.g. "hello"

    // --- Arithmetic operators ---
    PLUS,               // +
    MINUS,              // -
    MULTIPLY,           // *
    DIVIDE,             // /
    MODULO,             // %

    // --- Comparison operators ---
    GREATER,            // >
    LESS,               // 
    GREATER_EQ,         // >=
    LESS_EQ,            // <=
    EQUAL,              // ==
    NOT_EQUAL,          // <>

    // --- Assignment ---
    ASSIGN,             // =

    // --- LEXOR-specific symbols ---
    AMPERSAND,          // & (concatenator)
    DOLLAR,             // $ (newline/carriage return in PRINT)
    COLON,              // : (used after PRINT and SCAN)
    COMMA,              // , (separator in SCAN and DECLARE)
    LPAREN,             // (
    RPAREN,             // )
    LBRACKET,           // [
    RBRACKET,           // ]

    // --- Special ---
    NEWLINE,            // end of a logical line
    END_OF_FILE,        // marks end of input
    UNKNOWN             // anything the lexer cannot classify
};

// ---------------------------------------------------------------------------
// Token — a single unit produced by the Lexer.
// ---------------------------------------------------------------------------
struct Token {
    TokenType   type;
    std::string value;  // the raw text slice from the source
    int         line;   // 1-based line number (for error messages)

    Token(TokenType type, std::string value, int line)
        : type(type), value(std::move(value)), line(line) {}
};

#endif // TOKEN_H