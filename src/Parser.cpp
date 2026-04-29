#include "Parser.h"
#include "Lexer.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------
Parser::Parser(const std::vector<Token> &tokens)
    : tokens_(tokens), pos_(0)
{
}

// ---------------------------------------------------------------------------
// Token stream management
// ---------------------------------------------------------------------------
Token Parser::peek(int offset) const
{
    size_t index = pos_ + offset;
    if (index >= tokens_.size())
    {
        return tokens_.back(); // return END_OF_FILE
    }
    return tokens_[index];
}

Token Parser::advance()
{
    if (!isAtEnd())
    {
        return tokens_[pos_++];
    }
    return tokens_.back();
}

bool Parser::isAtEnd() const
{
    return pos_ >= tokens_.size() || peek().type == TokenType::END_OF_FILE;
}

Token Parser::expect(TokenType type)
{
    if (!check(type))
    {
        std::ostringstream oss;
        oss << "Expected " << tokenTypeToString(type)
            << " but got " << tokenTypeToString(peek().type);
        throw std::runtime_error(oss.str());
    }
    return advance();
}

bool Parser::match(TokenType type)
{
    if (check(type))
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::check(TokenType type) const
{
    return peek().type == type;
}

// ---------------------------------------------------------------------------
// Main parse entry point
// ---------------------------------------------------------------------------
void Parser::parse()
{
    try
    {
        parseScript();
        // If we get here, the entire program was valid
    }
    catch (const std::exception &e)
    {
        std::cerr << "Parse error: " << e.what() << std::endl;
        throw;
    }
}

// ---------------------------------------------------------------------------
// parseScript — validates top-level structure
// SCRIPT AREA START SCRIPT [statements] END SCRIPT
// ---------------------------------------------------------------------------
void Parser::parseScript()
{
    expect(TokenType::SCRIPT_AREA);
    expect(TokenType::START_SCRIPT);

    // Parse all statements
    parseStatements();

    expect(TokenType::END_SCRIPT);

    // Check for multiple START_SCRIPT or END_SCRIPT tokens
    if (!isAtEnd())
    {
        if (check(TokenType::START_SCRIPT))
        {
            throw std::runtime_error("Error: Multiple START SCRIPT statements found. Only one is allowed per script.");
        }
        if (check(TokenType::END_SCRIPT))
        {
            throw std::runtime_error("Error: Multiple END SCRIPT statements found. Only one is allowed per script.");
        }
        // If there are other tokens, also report an error
        if (!isAtEnd())
        {
            std::ostringstream oss;
            oss << "Error: Unexpected tokens after END SCRIPT: " << tokenTypeToString(peek().type);
            throw std::runtime_error(oss.str());
        }
    }
}

// ---------------------------------------------------------------------------
// parseStatements — parse all statements until END_SCRIPT
// ---------------------------------------------------------------------------
void Parser::parseStatements()
{
    while (!check(TokenType::END_SCRIPT) && !isAtEnd())
    {
        parseStatement();
    }
}

// ---------------------------------------------------------------------------
// parseStatement — dispatch to the appropriate statement parser
// ---------------------------------------------------------------------------
void Parser::parseStatement()
{
    if (check(TokenType::DECLARE))
    {
        parseDeclaration();
    }
    else if (check(TokenType::IDENTIFIER))
    {
        parseAssignment();
    }
    else if (check(TokenType::PRINT))
    {
        parsePrint();
    }
    else if (check(TokenType::SCAN))
    {
        parseScan();
    }
    else if (check(TokenType::IF))
    {
        throw std::runtime_error("IF statements not yet implemented");
    }
    else if (check(TokenType::FOR))
    {
        throw std::runtime_error("FOR loops not yet implemented");
    }
    else if (check(TokenType::REPEAT_WHEN))
    {
        throw std::runtime_error("REPEAT WHEN loops not yet implemented");
    }
    else
    {
        std::ostringstream oss;
        oss << "Unexpected token: " << tokenTypeToString(peek().type);
        throw std::runtime_error(oss.str());
    }
}

// ---------------------------------------------------------------------------
// parseDeclaration — DECLARE type id [= expr] [, id [= expr]]*
// ---------------------------------------------------------------------------
void Parser::parseDeclaration()
{
    expect(TokenType::DECLARE);

    // Expect a type token
    TokenType type;
    if (match(TokenType::TYPE_INT))
    {
        type = TokenType::TYPE_INT;
    }
    else if (match(TokenType::TYPE_FLOAT))
    {
        type = TokenType::TYPE_FLOAT;
    }
    else if (match(TokenType::TYPE_CHAR))
    {
        type = TokenType::TYPE_CHAR;
    }
    else if (match(TokenType::TYPE_BOOL))
    {
        type = TokenType::TYPE_BOOL;
    }
    else
    {
        throw std::runtime_error("Expected a type in DECLARE statement");
    }

    // Declare first identifier with optional initialization
    Token idToken = expect(TokenType::IDENTIFIER);
    symbolTable_.declare(idToken.value, type);

    // Check for optional initialization
    if (match(TokenType::ASSIGN))
    {
        VarValue value = evaluateExpression();
        symbolTable_.assign(idToken.value, value);
    }

    // Declare additional identifiers separated by commas
    while (match(TokenType::COMMA))
    {
        idToken = expect(TokenType::IDENTIFIER);
        symbolTable_.declare(idToken.value, type);

        // Check for optional initialization
        if (match(TokenType::ASSIGN))
        {
            VarValue value = evaluateExpression();
            symbolTable_.assign(idToken.value, value);
        }
    }
}

// ---------------------------------------------------------------------------
// parseAssignment — id = expression
// ---------------------------------------------------------------------------
void Parser::parseAssignment()
{
    Token idToken = expect(TokenType::IDENTIFIER);
    expect(TokenType::ASSIGN);

    VarValue value = evaluateExpression();

    if (!symbolTable_.exists(idToken.value))
    {
        std::ostringstream oss;
        oss << "Variable '" << idToken.value << "' is not declared";
        throw std::runtime_error(oss.str());
    }

    symbolTable_.assign(idToken.value, value);
}

// ---------------------------------------------------------------------------
// parsePrint — PRINT: expression
// Supports concatenation with & and newline with $
// ---------------------------------------------------------------------------
void Parser::parsePrint()
{
    expect(TokenType::PRINT);
    expect(TokenType::COLON);

    // Parse and print the first expression
    VarValue value = evaluateExpression();
    printValue(value);

    // Handle concatenation (&) and newline ($)
    while (check(TokenType::AMPERSAND) || check(TokenType::DOLLAR))
    {
        if (match(TokenType::AMPERSAND))
        {
            value = evaluateExpression();
            printValue(value);
        }
        else if (match(TokenType::DOLLAR))
        {
            std::cout << std::endl;
        }
    }
}

// ---------------------------------------------------------------------------
// parseScan — SCAN: id
// ---------------------------------------------------------------------------
void Parser::parseScan()
{
    expect(TokenType::SCAN);
    expect(TokenType::COLON);

    Token idToken = expect(TokenType::IDENTIFIER);

    if (!symbolTable_.exists(idToken.value))
    {
        std::ostringstream oss;
        oss << "Variable '" << idToken.value << "' is not declared";
        throw std::runtime_error(oss.str());
    }

    // For now, just read input and convert to appropriate type
    Symbol sym = symbolTable_.get(idToken.value);
    std::string input;
    std::getline(std::cin, input);

    try
    {
        VarValue value;
        switch (sym.type)
        {
        case TokenType::TYPE_INT:
            value = std::stoi(input);
            break;
        case TokenType::TYPE_FLOAT:
            value = std::stof(input);
            break;
        case TokenType::TYPE_CHAR:
            value = input.empty() ? '\0' : input[0];
            break;
        case TokenType::TYPE_BOOL:
            value = (input == "TRUE" || input == "true" || input == "1");
            break;
        default:
            throw std::runtime_error("Invalid variable type in SCAN");
        }
        symbolTable_.assign(idToken.value, value);
    }
    catch (const std::exception &e)
    {
        std::ostringstream oss;
        oss << "Error converting input for variable '" << idToken.value << "': " << e.what();
        throw std::runtime_error(oss.str());
    }
}

// ---------------------------------------------------------------------------
// evaluateExpression — parse and evaluate an expression
// ---------------------------------------------------------------------------
VarValue Parser::evaluateExpression()
{
    return parseAddition();
}

// ---------------------------------------------------------------------------
// parseAddition — handle + and - operators
// ---------------------------------------------------------------------------
VarValue Parser::parseAddition()
{
    VarValue result = parseMultiplication();

    while (check(TokenType::PLUS) || check(TokenType::MINUS))
    {
        TokenType op = advance().type;
        VarValue right = parseMultiplication();

        // Simple arithmetic: convert to int or float
        if (result.isInt() && right.isInt())
        {
            int lhs = result.asInt();
            int rhs = right.asInt();
            result = VarValue(op == TokenType::PLUS ? lhs + rhs : lhs - rhs);
        }
        else if ((result.isInt() || result.isFloat()) && (right.isInt() || right.isFloat()))
        {
            float lhs = result.isInt() ? static_cast<float>(result.asInt()) : result.asFloat();
            float rhs = right.isInt() ? static_cast<float>(right.asInt()) : right.asFloat();
            result = VarValue(op == TokenType::PLUS ? lhs + rhs : lhs - rhs);
        }
        else
        {
            throw std::runtime_error("Cannot perform arithmetic on non-numeric types");
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// parseMultiplication — handle * and / operators
// ---------------------------------------------------------------------------
VarValue Parser::parseMultiplication()
{
    VarValue result = parseConcatenation();

    while (check(TokenType::MULTIPLY) || check(TokenType::DIVIDE) || check(TokenType::MODULO))
    {
        TokenType op = advance().type;
        VarValue right = parseConcatenation();

        // Simple arithmetic: convert to int or float
        if (result.isInt() && right.isInt())
        {
            int lhs = result.asInt();
            int rhs = right.asInt();
            if (op == TokenType::MULTIPLY)
            {
                result = VarValue(lhs * rhs);
            }
            else if (op == TokenType::DIVIDE)
            {
                if (rhs == 0)
                    throw std::runtime_error("Division by zero");
                result = VarValue(lhs / rhs);
            }
            else // MODULO
            {
                if (rhs == 0)
                    throw std::runtime_error("Modulo by zero");
                result = VarValue(lhs % rhs);
            }
        }
        else if ((result.isInt() || result.isFloat()) && (right.isInt() || right.isFloat()))
        {
            float lhs = result.isInt() ? static_cast<float>(result.asInt()) : result.asFloat();
            float rhs = right.isInt() ? static_cast<float>(right.asInt()) : right.asFloat();
            if (op == TokenType::MULTIPLY)
            {
                result = VarValue(lhs * rhs);
            }
            else if (op == TokenType::DIVIDE)
            {
                if (rhs == 0.0f)
                    throw std::runtime_error("Division by zero");
                result = VarValue(lhs / rhs);
            }
            else // MODULO not supported for floats
            {
                throw std::runtime_error("Modulo operator not supported for floating-point numbers");
            }
        }
        else
        {
            throw std::runtime_error("Cannot perform arithmetic on non-numeric types");
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// parseConcatenation — handle & operator (string concatenation in LEXOR)
// ---------------------------------------------------------------------------
VarValue Parser::parseConcatenation()
{
    VarValue result = parsePrimaryValue();

    while (check(TokenType::AMPERSAND))
    {
        advance(); // consume &
        VarValue right = parsePrimaryValue();

        // Convert both to strings and concatenate
        std::ostringstream oss_left, oss_right;

        // Convert left value to string
        if (result.isInt())
        {
            oss_left << result.asInt();
        }
        else if (result.isFloat())
        {
            oss_left << result.asFloat();
        }
        else if (result.isChar())
        {
            oss_left << result.asChar();
        }
        else if (result.isBool())
        {
            oss_left << (result.asBool() ? "\"TRUE\"" : "\"FALSE\"");
        }
        else if (result.isString())
        {
            oss_left << result.asString();
        }

        // Convert right value to string
        if (right.isInt())
        {
            oss_right << right.asInt();
        }
        else if (right.isFloat())
        {
            oss_right << right.asFloat();
        }
        else if (right.isChar())
        {
            oss_right << right.asChar();
        }
        else if (right.isBool())
        {
            oss_right << (right.asBool() ? "\"TRUE\"" : "\"FALSE\"");
        }
        else if (right.isString())
        {
            oss_right << right.asString();
        }

        // Store concatenated result as a string
        std::string concat = oss_left.str() + oss_right.str();
        result = VarValue(concat);
    }

    return result;
}

// ---------------------------------------------------------------------------
// parsePrimaryValue — identifier, literal, or grouped expression
// ---------------------------------------------------------------------------
VarValue Parser::parsePrimaryValue()
{
    // Identifier (variable reference)
    if (check(TokenType::IDENTIFIER))
    {
        Token idToken = advance();
        if (!symbolTable_.exists(idToken.value))
        {
            std::ostringstream oss;
            oss << "Variable '" << idToken.value << "' is not declared";
            throw std::runtime_error(oss.str());
        }
        return symbolTable_.get(idToken.value).value;
    }

    // Integer literal
    if (check(TokenType::INT_LITERAL))
    {
        Token token = advance();
        return std::stoi(token.value);
    }

    // Float literal
    if (check(TokenType::FLOAT_LITERAL))
    {
        Token token = advance();
        return std::stof(token.value);
    }

    // Character literal
    if (check(TokenType::CHAR_LITERAL))
    {
        Token token = advance();
        return token.value.empty() ? '\0' : token.value[0];
    }

    // String literal (now properly stored as a string)
    if (check(TokenType::STRING_LITERAL))
    {
        Token token = advance();
        return VarValue(token.value);
    }

    // Boolean literal
    if (check(TokenType::BOOL_LITERAL))
    {
        Token token = advance();
        return token.value == "TRUE";
    }

    // Grouped expression (not implemented yet)
    if (check(TokenType::LPAREN))
    {
        advance();
        VarValue value = evaluateExpression();
        expect(TokenType::RPAREN);
        return value;
    }

    throw std::runtime_error("Expected an expression");
}

// ---------------------------------------------------------------------------
// Utility function to print a VarValue
// ---------------------------------------------------------------------------
void Parser::printValue(const VarValue &value)
{
    if (value.isInt())
    {
        std::cout << value.asInt();
    }
    else if (value.isFloat())
    {
        std::cout << value.asFloat();
    }
    else if (value.isChar())
    {
        std::cout << value.asChar();
    }
    else if (value.isBool())
    {
        std::cout << (value.asBool() ? "TRUE" : "FALSE");
    }
    else if (value.isString())
    {
        std::cout << value.asString();
    }
}

// ---------------------------------------------------------------------------
// tokenTypeToString — for error messages
// ---------------------------------------------------------------------------
std::string Parser::tokenTypeToString(TokenType type) const
{
    switch (type)
    {
    case TokenType::SCRIPT_AREA:
        return "SCRIPT AREA";
    case TokenType::START_SCRIPT:
        return "START SCRIPT";
    case TokenType::END_SCRIPT:
        return "END SCRIPT";
    case TokenType::DECLARE:
        return "DECLARE";
    case TokenType::TYPE_INT:
        return "INT";
    case TokenType::TYPE_FLOAT:
        return "FLOAT";
    case TokenType::TYPE_CHAR:
        return "CHAR";
    case TokenType::TYPE_BOOL:
        return "BOOL";
    case TokenType::PRINT:
        return "PRINT";
    case TokenType::SCAN:
        return "SCAN";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::INT_LITERAL:
        return "INT_LITERAL";
    case TokenType::FLOAT_LITERAL:
        return "FLOAT_LITERAL";
    case TokenType::CHAR_LITERAL:
        return "CHAR_LITERAL";
    case TokenType::STRING_LITERAL:
        return "STRING_LITERAL";
    case TokenType::BOOL_LITERAL:
        return "BOOL_LITERAL";
    case TokenType::ASSIGN:
        return "ASSIGN";
    case TokenType::COLON:
        return "COLON";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::AMPERSAND:
        return "AMPERSAND";
    case TokenType::DOLLAR:
        return "DOLLAR";
    case TokenType::LPAREN:
        return "LPAREN";
    case TokenType::RPAREN:
        return "RPAREN";
    case TokenType::END_OF_FILE:
        return "END_OF_FILE";
    default:
        return "UNKNOWN";
    }
}
