#include "SymbolTable.h"
#include <sstream>

// ---------------------------------------------------------------------------
// SymbolTable::declare
// ---------------------------------------------------------------------------
void SymbolTable::declare(const std::string &name, TokenType type)
{
    if (symbols_.find(name) != symbols_.end())
    {
        std::ostringstream oss;
        oss << "Variable '" << name << "' is already declared.";
        throw std::runtime_error(oss.str());
    }

    // Initialize with a default value based on type
    VarValue defaultValue;
    switch (type)
    {
    case TokenType::TYPE_INT:
        defaultValue = 0;
        break;
    case TokenType::TYPE_FLOAT:
        defaultValue = 0.0f;
        break;
    case TokenType::TYPE_CHAR:
        defaultValue = '\0';
        break;
    case TokenType::TYPE_BOOL:
        defaultValue = false;
        break;
    default:
        throw std::runtime_error("Invalid type for variable declaration.");
    }

    symbols_[name] = Symbol{type, defaultValue};
}

// ---------------------------------------------------------------------------
// SymbolTable::assign
// ---------------------------------------------------------------------------
void SymbolTable::assign(const std::string &name, const VarValue &val)
{
    if (symbols_.find(name) == symbols_.end())
    {
        std::ostringstream oss;
        oss << "Variable '" << name << "' is not declared.";
        throw std::runtime_error(oss.str());
    }

    // Type check: ensure value matches declared type
    Symbol &sym = symbols_[name];
    bool typeMatch = false;

    switch (sym.type)
    {
    case TokenType::TYPE_INT:
        if (!val.isInt())
        {
            std::ostringstream oss;
            oss << "Type mismatch for variable '" << name << "': expected INT but got ";
            if (val.isFloat())
                oss << "FLOAT";
            else if (val.isChar())
                oss << "CHAR";
            else if (val.isBool())
                oss << "BOOL";
            else if (val.isString())
                oss << "STRING";
            throw std::runtime_error(oss.str());
        }
        typeMatch = true;
        break;
    case TokenType::TYPE_FLOAT:
        if (!val.isFloat())
        {
            std::ostringstream oss;
            oss << "Type mismatch for variable '" << name << "': expected FLOAT but got ";
            if (val.isInt())
                oss << "INT";
            else if (val.isChar())
                oss << "CHAR";
            else if (val.isBool())
                oss << "BOOL";
            else if (val.isString())
                oss << "STRING";
            throw std::runtime_error(oss.str());
        }
        typeMatch = true;
        break;
    case TokenType::TYPE_CHAR:
        if (!val.isChar())
        {
            std::ostringstream oss;
            oss << "Type mismatch for variable '" << name << "': expected CHAR but got ";
            if (val.isInt())
                oss << "INT";
            else if (val.isFloat())
                oss << "FLOAT";
            else if (val.isBool())
                oss << "BOOL";
            else if (val.isString())
                oss << "STRING";
            throw std::runtime_error(oss.str());
        }
        typeMatch = true;
        break;
    case TokenType::TYPE_BOOL:
        if (!val.isBool())
        {
            std::ostringstream oss;
            oss << "Type mismatch for variable '" << name << "': expected BOOL but got ";
            if (val.isInt())
                oss << "INT";
            else if (val.isFloat())
                oss << "FLOAT";
            else if (val.isChar())
                oss << "CHAR";
            else if (val.isString())
                oss << "STRING";
            throw std::runtime_error(oss.str());
        }
        typeMatch = true;
        break;
    default:
        throw std::runtime_error("Invalid variable type");
    }

    if (typeMatch)
    {
        sym.value = val;
    }
}

// ---------------------------------------------------------------------------
// SymbolTable::get
// ---------------------------------------------------------------------------
Symbol SymbolTable::get(const std::string &name) const
{
    auto it = symbols_.find(name);
    if (it == symbols_.end())
    {
        std::ostringstream oss;
        oss << "Variable '" << name << "' is not declared.";
        throw std::runtime_error(oss.str());
    }

    return it->second;
}

// ---------------------------------------------------------------------------
// SymbolTable::exists
// ---------------------------------------------------------------------------
bool SymbolTable::exists(const std::string &name) const
{
    return symbols_.find(name) != symbols_.end();
}

// ---------------------------------------------------------------------------
// SymbolTable::clear
// ---------------------------------------------------------------------------
void SymbolTable::clear()
{
    symbols_.clear();
}
