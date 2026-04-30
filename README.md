# LEXOR Interpreter (C++)
When cloning the repo just needs to type `make` in the terminal to compile the code, and `./lexor <source file>` to run it.
## Increment 2 Tasks (Draft)
- [ ] • Can recognize unary operator
- [ ] • Should be able to read input data (SCAN).Q
- [ ] • Should be able to perform arithmetic and logical operations.
        
        Details:
        
        Remaining work (Increment 2):
        
        Testing:
        - Use the sample program in the spec PDF for end-to-end validation; expected output:
        
        Notes:
        - If any token stream anomalies appear, report them and I will adjust the lexer.
        - The Checking of Boolean is done in the Lexer.cpp, not in the Token.h or SymbolTable.
                Since that would require more word and it wouldn't change much since we would
                still need to explicitly declare BOOL as String values.
        - `[]' Bracket recognition are also done in Lexer.cpp
