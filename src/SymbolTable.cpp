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

    symbols_[name].value = val;
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
