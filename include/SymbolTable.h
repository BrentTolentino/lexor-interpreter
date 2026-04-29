#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <string>
#include <unordered_map>
#include <stdexcept>
#include "Token.h"

// ---------------------------------------------------------------------------
// VarValue — C++11-compatible value holder for LEXOR runtime values
// Stores one of: int, float, char, bool, string
// ---------------------------------------------------------------------------
class VarValue
{
public:
    enum Type
    {
        INT,
        FLOAT,
        CHAR,
        BOOL,
        STRING
    };

    // Constructors
    VarValue() : type_(INT), int_value(0), string_value("") {}

    VarValue(int val) : type_(INT), int_value(val), string_value("") {}
    VarValue(float val) : type_(FLOAT), float_value(val), string_value("") {}
    VarValue(char val) : type_(CHAR), char_value(val), string_value("") {}
    VarValue(bool val) : type_(BOOL), bool_value(val), string_value("") {}
    VarValue(const std::string &val) : type_(STRING), int_value(0), string_value(val) {}

    // Getters with type check
    int asInt() const
    {
        if (type_ != INT)
            throw std::runtime_error("Value is not an int");
        return int_value;
    }

    float asFloat() const
    {
        if (type_ != FLOAT)
            throw std::runtime_error("Value is not a float");
        return float_value;
    }

    char asChar() const
    {
        if (type_ != CHAR)
            throw std::runtime_error("Value is not a char");
        return char_value;
    }

    bool asBool() const
    {
        if (type_ != BOOL)
            throw std::runtime_error("Value is not a bool");
        return bool_value;
    }

    std::string asString() const
    {
        if (type_ != STRING)
            throw std::runtime_error("Value is not a string");
        return string_value;
    }

    Type getType() const { return type_; }

    // Type check methods
    bool isInt() const { return type_ == INT; }
    bool isFloat() const { return type_ == FLOAT; }
    bool isChar() const { return type_ == CHAR; }
    bool isBool() const { return type_ == BOOL; }
    bool isString() const { return type_ == STRING; }

private:
    Type type_;
    int int_value;
    float float_value;
    char char_value;
    bool bool_value;
    std::string string_value;
};

// ---------------------------------------------------------------------------
// Symbol — represents a declared variable with its type and current value
// ---------------------------------------------------------------------------
struct Symbol
{
    TokenType type; // TYPE_INT, TYPE_FLOAT, TYPE_CHAR, TYPE_BOOL
    VarValue value; // the actual runtime value
};

// ---------------------------------------------------------------------------
// SymbolTable — manages variable declarations and assignments
//
// Usage:
//   SymbolTable table;
//   table.declare("x", TokenType::TYPE_INT);
//   table.assign("x", 42);
//   if (table.exists("x")) { ... }
// ---------------------------------------------------------------------------
class SymbolTable
{
public:
    // Declare a new variable with a type.
    // Throws std::runtime_error if the name is already declared.
    void declare(const std::string &name, TokenType type);

    // Assign a value to an existing variable.
    // Throws std::runtime_error if the name does not exist.
    void assign(const std::string &name, const VarValue &val);

    // Retrieve a symbol (type and value).
    // Throws std::runtime_error if the name does not exist.
    Symbol get(const std::string &name) const;

    // Check if a variable has been declared.
    bool exists(const std::string &name) const;

    // Clear all symbols (useful for testing or resets).
    void clear();

private:
    std::unordered_map<std::string, Symbol> symbols_;
};

#endif
