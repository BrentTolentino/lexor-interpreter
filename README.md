# LEXOR Interpreter (C++)
When cloning the repo just needs to type `make` in the terminal to compile the code, and `./lexor` to run it.
## Increment 1 Tasks
- [x] Recognize basic structure (`SCRIPT AREA`, `START SCRIPT`, `END SCRIPT`)
- [x] Recognize comments (`%%`)
- [x] Recognize reserved words
- [x] Able to concatenate (`&`)
- [x] Declare variables (`INT`, `CHAR`, `BOOL`, `FLOAT`) (this is already recognized at the lexer level, the tokens exist and are correctly typed.)
- [x] Assign data to a variable (`=`) (this is already recognized at the lexer level, the tokens exist and are correctly typed.)
- [x] Able to display (`PRINT:`)



Add completed lexer on feature/lexer-tokenizer with tokens defined in include/Token.h.
Document how to obtain token stream (Lexer::tokenize()) and remaining tasks for Increment 1:
        
        feat(lexer): implement lexer/tokenizer and provide integration notes for parser tasks
        
        Completed lexer implementation on branch feature/lexer-tokenizer and added usage details for downstream parser work.
        
        Details:
        - Tokens are fully defined in include/Token.h — do not modify, simply #include.
        - Lexer API: create with Lexer lexer(source); then call auto tokens = lexer.tokenize(); to get a clean token stream.
        - Lexer recognizes token-level constructs including DECLARE, TYPE tokens (INT/CHAR/BOOL/FLOAT), ASSIGN, PRINT, COLON, AMPERSAND, DOLLAR, and literal tokens (INT/FLOAT/CHAR/STRING/BOOL).
        
        Remaining work (Increment 1):
        1. Parser.cpp / Parser.h — traverse token stream and validate script structure (SCRIPT AREA → START SCRIPT → declarations → statements → END SCRIPT).
        2. Variable declarations — on DECLARE + type + IDENTIFIER, add to symbol table.
        3. Assignments — on IDENTIFIER + ASSIGN + value, update symbol table.
        4. PRINT execution — on PRINT + COLON, evaluate expression, support & for concatenation and $ for newline, printing to stdout.
        
        Testing:
        - Use the sample program in the spec PDF for end-to-end validation; expected output:
          4TRUE5
          c#last
        
        Notes:
        - If any token stream anomalies appear, report them and I will adjust the lexer.
