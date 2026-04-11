# LEXOR Interpreter (C++)
When cloning the repo just needs to type `make` in the terminal to compile the code, and `./lexor` to run it.
## Increment 1 Tasks
- [x] Recognize basic structure (`SCRIPT AREA`, `START SCRIPT`, `END SCRIPT`)
- [x] Recognize comments (`%%`)
- [x] Recognize reserved words
- [x] Able to concatenate (`&`)
- [ ] Declare variables (`INT`, `CHAR`, `BOOL`, `FLOAT`) (this is already recognized at the lexer level, the tokens exist and are correctly typed.)
- [ ] Assign data to a variable (`=`) (this is already recognized at the lexer level, the tokens exist and are correctly typed.)
- [ ] Able to display (`PRINT:`)



Add completed lexer on feature/lexer-tokenizer with tokens defined in include/Token.h.
Document how to obtain token stream (Lexer::tokenize()) and remaining tasks for Increment 1:

    parser structure, declarations, assignments, PRINT execution
    key token types and expected sample output for end-to-end test.
