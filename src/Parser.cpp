#include "Parser.h"
#include "Lexer.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <deque>
#include <cctype>

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
            throw std::runtime_error("Multiple START SCRIPT statements found. Only one is allowed per script.");
        }
        if (check(TokenType::END_SCRIPT))
        {
            throw std::runtime_error("Multiple END SCRIPT statements found. Only one is allowed per script.");
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
    while (!check(TokenType::END_SCRIPT) &&
           !check(TokenType::END_IF) &&
           !check(TokenType::END_FOR) &&
           !check(TokenType::END_REPEAT) &&
           !check(TokenType::ELSE_IF) &&
           !check(TokenType::ELSE) &&
           !isAtEnd())
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
        parseIf();
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

    // Parse and print the entire concatenation expression
    // The expression can include & operators to concatenate values,
    // and $ tokens which represent newlines
    VarValue value = evaluateExpression();
    printValue(value);
}

// ---------------------------------------------------------------------------
// parseScan — SCAN: id
// ---------------------------------------------------------------------------
void Parser::parseScan()
{
    expect(TokenType::SCAN);
    expect(TokenType::COLON);

    // Collect one or more identifiers separated by commas
    std::vector<Token> ids;
    Token idToken = expect(TokenType::IDENTIFIER);
    ids.push_back(idToken);

    while (match(TokenType::COMMA))
    {
        Token nextId = expect(TokenType::IDENTIFIER);
        ids.push_back(nextId);
    }

    // Read scan input as a buffered stream so both formats work:
    //   4
    //   5
    //   6
    // and
    //   4,5,6
    //   4, 5, 6
    std::deque<std::string> bufferedInputs;

    auto trim = [](const std::string &text) -> std::string
    {
        size_t start = 0;
        while (start < text.size() && std::isspace(static_cast<unsigned char>(text[start])))
        {
            ++start;
        }

        size_t end = text.size();
        while (end > start && std::isspace(static_cast<unsigned char>(text[end - 1])))
        {
            --end;
        }

        return text.substr(start, end - start);
    };

    auto fillBuffer = [&]() -> bool
    {
        std::string line;

        while (bufferedInputs.empty() && std::getline(std::cin, line))
        {
            std::string trimmedLine = trim(line);
            if (trimmedLine.empty())
            {
                continue;
            }

            if (trimmedLine.find(',') != std::string::npos)
            {
                std::stringstream lineStream(trimmedLine);
                std::string part;

                while (std::getline(lineStream, part, ','))
                {
                    std::string token = trim(part);
                    if (token.empty())
                    {
                        throw std::runtime_error("Invalid SCAN input: empty value between commas");
                    }
                    bufferedInputs.push_back(token);
                }
            }
            else
            {
                std::stringstream lineStream(trimmedLine);
                std::string token;

                while (lineStream >> token)
                {
                    bufferedInputs.push_back(token);
                }
            }
        }

        return !bufferedInputs.empty();
    };

    // For each identifier, read one buffered value from stdin.
    for (const Token &t : ids)
    {
        if (!symbolTable_.exists(t.value))
        {
            std::ostringstream oss;
            oss << "Variable '" << t.value << "' is not declared";
            throw std::runtime_error(oss.str());
        }

        Symbol sym = symbolTable_.get(t.value);
        if (bufferedInputs.empty() && !fillBuffer())
        {
            std::ostringstream oss;
            oss << "Insufficient input for variable '" << t.value << "'";
            throw std::runtime_error(oss.str());
        }

        std::string inputToken = bufferedInputs.front();
        bufferedInputs.pop_front();

        try
        {
            VarValue value;

            // Strict parsing: ensure entire token is consumed and type matches
            if (sym.type == TokenType::TYPE_INT)
            {
                size_t idx = 0;
                long v = 0;
                try
                {
                    v = std::stol(inputToken, &idx);
                }
                catch (...)
                {
                    idx = std::string::npos;
                }
                if (idx != inputToken.size())
                {
                    std::ostringstream oss;
                    oss << "Invalid integer input for variable '" << t.value << "': '" << inputToken << "'";
                    throw std::runtime_error(oss.str());
                }
                value = static_cast<int>(v);
            }
            else if (sym.type == TokenType::TYPE_FLOAT)
            {
                size_t idx = 0;
                float v = 0.0f;
                try
                {
                    v = std::stof(inputToken, &idx);
                }
                catch (...)
                {
                    idx = std::string::npos;
                }
                if (idx != inputToken.size())
                {
                    std::ostringstream oss;
                    oss << "Invalid float input for variable '" << t.value << "': '" << inputToken << "'";
                    throw std::runtime_error(oss.str());
                }
                value = v;
            }
            else if (sym.type == TokenType::TYPE_CHAR)
            {
                if (inputToken.size() != 1)
                {
                    std::ostringstream oss;
                    oss << "Invalid char input for variable '" << t.value << "': '" << inputToken << "'";
                    throw std::runtime_error(oss.str());
                }
                value = inputToken[0];
            }
            else if (sym.type == TokenType::TYPE_BOOL)
            {
                // Accept TRUE/FALSE (case-insensitive) or 1/0
                std::string s = inputToken;
                for (auto &c : s)
                    c = static_cast<char>(std::toupper(c));
                if (s == "TRUE" || s == "1")
                {
                    value = true;
                }
                else if (s == "FALSE" || s == "0")
                {
                    value = false;
                }
                else
                {
                    std::ostringstream oss;
                    oss << "Invalid boolean input for variable '" << t.value << "': '" << inputToken << "'";
                    throw std::runtime_error(oss.str());
                }
            }
            else
            {
                throw std::runtime_error("Invalid variable type in SCAN");
            }

            symbolTable_.assign(t.value, value);
        }
        catch (const std::exception &e)
        {
            std::ostringstream oss;
            oss << "Error converting input for variable '" << t.value << "': " << e.what();
            throw std::runtime_error(oss.str());
        }
    }
}

// ---------------------------------------------------------------------------
// parseIf — IF/ELSE IF/ELSE conditional blocks
// ---------------------------------------------------------------------------
void Parser::parseIf()
{
    // Track whether any branch has already been taken so later branches are
    // skipped (evaluated but not executed).
    bool branchTaken = false;

    // ── initial IF ──────────────────────────────────────────────────────────
    expect(TokenType::IF);
    expect(TokenType::LPAREN);
    VarValue condition = evaluateExpression();
    expect(TokenType::RPAREN);

    if (!condition.isBool())
        throw std::runtime_error("IF condition must evaluate to a boolean expression");

    expect(TokenType::START_IF);

    if (condition.asBool())
    {
        branchTaken = true;
        parseStatements(); // execute the body
    }
    else
    {
        // Skip the body without executing
        while (!check(TokenType::END_IF) && !isAtEnd())
            advance();
    }

    expect(TokenType::END_IF);

    // ── zero or more ELSE IF branches ───────────────────────────────────────
    while (check(TokenType::ELSE_IF))
    {
        advance(); // consume ELSE IF
        expect(TokenType::LPAREN);
        VarValue elseIfCond = evaluateExpression();
        expect(TokenType::RPAREN);

        if (!elseIfCond.isBool())
            throw std::runtime_error("ELSE IF condition must evaluate to a boolean expression");

        expect(TokenType::START_IF);

        if (!branchTaken && elseIfCond.asBool())
        {
            branchTaken = true;
            parseStatements();
        }
        else
        {
            while (!check(TokenType::END_IF) && !isAtEnd())
                advance();
        }

        expect(TokenType::END_IF);
    }

    // ── optional ELSE branch ────────────────────────────────────────────────
    if (check(TokenType::ELSE))
    {
        advance(); // consume ELSE
        expect(TokenType::START_IF);

        if (!branchTaken)
        {
            parseStatements();
        }
        else
        {
            while (!check(TokenType::END_IF) && !isAtEnd())
                advance();
        }

        expect(TokenType::END_IF);
    }
}

// ---------------------------------------------------------------------------
// evaluateExpression — parse and evaluate an expression
// ---------------------------------------------------------------------------
VarValue Parser::evaluateExpression()
{
    return parseLogicalOr();
}

// ---------------------------------------------------------------------------
// parseLogicalOr — handle OR operator (lowest precedence)
// ---------------------------------------------------------------------------
VarValue Parser::parseLogicalOr()
{
    VarValue result = parseLogicalAnd();

    while (check(TokenType::OR))
    {
        advance(); // consume OR
        VarValue right = parseLogicalAnd();

        // Both operands must be boolean
        if (!result.isBool() || !right.isBool())
        {
            throw std::runtime_error("Logical OR requires boolean operands");
        }

        result = VarValue(result.asBool() || right.asBool());
    }

    return result;
}

// ---------------------------------------------------------------------------
// parseLogicalAnd — handle AND operator
// ---------------------------------------------------------------------------
VarValue Parser::parseLogicalAnd()
{
    VarValue result = parseComparison();

    while (check(TokenType::AND))
    {
        advance(); // consume AND
        VarValue right = parseComparison();

        // Both operands must be boolean
        if (!result.isBool() || !right.isBool())
        {
            throw std::runtime_error("Logical AND requires boolean operands");
        }

        result = VarValue(result.asBool() && right.asBool());
    }

    return result;
}

// ---------------------------------------------------------------------------
// parseComparison — handle ==, !=, <, >, <=, >= operators
// ---------------------------------------------------------------------------
VarValue Parser::parseComparison()
{
    VarValue result = parseAddition();

    while (check(TokenType::EQUAL) || check(TokenType::NOT_EQUAL) ||
           check(TokenType::LESS) || check(TokenType::GREATER) ||
           check(TokenType::LESS_EQ) || check(TokenType::GREATER_EQ))
    {
        TokenType op = advance().type;
        VarValue right = parseAddition();

        bool comparison_result = false;

        // Handle numeric comparisons
        if ((result.isInt() || result.isFloat()) && (right.isInt() || right.isFloat()))
        {
            float lhs = result.isInt() ? static_cast<float>(result.asInt()) : result.asFloat();
            float rhs = right.isInt() ? static_cast<float>(right.asInt()) : right.asFloat();

            switch (op)
            {
            case TokenType::EQUAL:
                comparison_result = (lhs == rhs);
                break;
            case TokenType::NOT_EQUAL:
                comparison_result = (lhs != rhs);
                break;
            case TokenType::LESS:
                comparison_result = (lhs < rhs);
                break;
            case TokenType::GREATER:
                comparison_result = (lhs > rhs);
                break;
            case TokenType::LESS_EQ:
                comparison_result = (lhs <= rhs);
                break;
            case TokenType::GREATER_EQ:
                comparison_result = (lhs >= rhs);
                break;
            default:
                break;
            }
        }
        // Handle string comparisons
        else if (result.isString() && right.isString())
        {
            std::string lhs = result.asString();
            std::string rhs = right.asString();

            switch (op)
            {
            case TokenType::EQUAL:
                comparison_result = (lhs == rhs);
                break;
            case TokenType::NOT_EQUAL:
                comparison_result = (lhs != rhs);
                break;
            default:
                throw std::runtime_error("Comparison operators <, >, <=, >= not supported for strings");
            }
        }
        // Handle boolean comparisons
        else if (result.isBool() && right.isBool())
        {
            bool lhs = result.asBool();
            bool rhs = right.asBool();

            switch (op)
            {
            case TokenType::EQUAL:
                comparison_result = (lhs == rhs);
                break;
            case TokenType::NOT_EQUAL:
                comparison_result = (lhs != rhs);
                break;
            default:
                throw std::runtime_error("Only == and != operators supported for booleans");
            }
        }
        else
        {
            throw std::runtime_error("Invalid operand types for comparison operator");
        }

        result = VarValue(comparison_result);
    }

    return result;
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
    VarValue result = parseUnary();

    while (check(TokenType::AMPERSAND))
    {
        advance(); // consume &
        VarValue right = parseUnary();

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

//
// parseUnary - handle unary operators: +, -, and NOT
//
VarValue Parser::parseUnary()
{
    if (check(TokenType::NOT))
    {
        advance();                     // consume NOT
        VarValue right = parseUnary(); // Recursively call for chained unary operators

        if (!right.isBool())
        {
            throw std::runtime_error("NOT operator requires a boolean operand");
        }

        return VarValue(!right.asBool());
    }

    if (check(TokenType::PLUS) || check(TokenType::MINUS))
    {
        TokenType operatorType = advance().type; // Consume the unary operator

        VarValue right = parseUnary(); // Recursively call parseUnary for chained unary operators

        if (operatorType == TokenType::MINUS)
        {
            if (right.isInt())
            {
                return VarValue(-right.asInt());
            }
            else if (right.isFloat())
            {
                return VarValue(-right.asFloat());
            }
            else
            {
                throw std::runtime_error("Cannot apply unary minus to non-numeric type.");
            }
        }
        // Unary PLUS simply returns the value as is.
        // The Lexer already handles numeric literals with implicit positive sign.
        else if (operatorType == TokenType::PLUS)
        {
            if (right.isInt() || right.isFloat())
            {
                return right; // No change for unary plus on numbers
            }
            else
            {
                throw std::runtime_error("Cannot apply unary plus to non-numeric type.");
            }
        }
    }
    // If no unary operator, parse the next higher precedence (primary value)
    return parsePrimaryValue();
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

    // Dollar sign ($) represents a newline/carriage return in PRINT context
    if (check(TokenType::DOLLAR))
    {
        advance();
        return VarValue(std::string("\n"));
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
