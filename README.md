# LEXOR Interpreter (C++)
When cloning the repo just needs to type `make` in the terminal to compile the code, and `./lexor <source file>` to run it.
## Increment 2 Tasks
- [x] • Can recognize unary operator
- [ ] • Should be able to read input data (SCAN).Q
- [ ] • Should be able to perform arithmetic and logical operations.
        
        Details:
        - Changes for the Unary operator were done in the Parser.h and Parser.cpp. 
        
        Remaining work (Increment 2):
        - **Scanner implementation**: Implement logic to read runtime input into variables.
        - **Arithmetic & Logical Operators**: Complete the implementation of all operators in the precedence hierarchy.
        - **AST Refinement**: (If applicable) Introduce specific nodes for Unary and Logical expressions to decouple parsing from evaluation.


        Testing:
        - To test is use './lexor tc1.txt' or './lexor test_unary_ops.txt'. (Verifies unary `+` and `-` precedence and numeric negation).
        
        Notes:
        - If any token stream anomalies appear, report them and I will adjust the lexer.
        - The Checking of Boolean is done in the Lexer.cpp, not in the Token.h or SymbolTable.
                Since that would require more word and it wouldn't change much since we would
                still need to explicitly declare BOOL as String values.
        - `[]' Bracket recognition are also done in Lexer.cpp.
        
        Suggestions:
        - include/Parser.h and src/Parser.cpp: To update the grammar logic (e.g., the expression parsing rules) to handle unary operators in the precedence hierarchy.
        - include/AST.h (or your equivalent AST node structure): To add a new node type (e.g., UnaryExpressionNode) that can store the operator and its operand.
        - src/Evaluator.cpp (or your equivalent Interpreter/Evaluator class): To implement the execution logic for the new unary operator (e.g., performing the negation or logical NOT).
