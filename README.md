# LEXOR Interpreter

LEXOR is a strongly typed interpreter written in C++. It tokenizes a LEXOR source file, parses the program structure, and executes the script directly.

## Features

- Script wrapper enforcement with `SCRIPT AREA`, `START SCRIPT`, and `END SCRIPT`
- Variable declarations with `DECLARE` for `INT`, `FLOAT`, `CHAR`, and `BOOL`
- Assignment after declaration with runtime type checking
- Output with `PRINT`
- Input with `SCAN`
- Conditional execution with `IF`, `ELSE IF`, and `ELSE`
- Loop execution with `FOR` and `REPEAT WHEN`
- Arithmetic operators `+`, `-`, `*`, `/`, and `%`
- Comparison operators `>`, `<`, `>=`, `<=`, `==`, and `<>`
- Logical operators `AND`, `OR`, and `NOT`
- Unary `+` and unary `-`
- String concatenation with `&`
- Escape-style bracket literals such as `[#]`, `[[]`, and `[]]`
- Comments using `%%`

## Build

Compile the interpreter with:

```bash
make
```

Clean build artifacts with:

```bash
make clean
```

## Run

Execute a LEXOR program with:

```bash
./lexor <source_file>
```

Example:

```bash
./lexor test_simple.txt
```

If the program finishes successfully, the executable prints:

```text
Script executed successfully!
```

## Repository Layout

- [src/Lexer.cpp](src/Lexer.cpp): converts raw source text into tokens
- [src/Parser.cpp](src/Parser.cpp): parses and executes the language
- [src/SymbolTable.cpp](src/SymbolTable.cpp): stores declared variables and runtime values
- [src/main.cpp](src/main.cpp): loads the source file, tokenizes it, and starts execution
- [include/Token.h](include/Token.h): token definitions
- [include/Lexer.h](include/Lexer.h): lexer interface
- [include/Parser.h](include/Parser.h): parser interface
- [include/SymbolTable.h](include/SymbolTable.h): runtime value and symbol table definitions

## Language Overview

LEXOR uses uppercase reserved words. Variable names are case sensitive and must start with a letter or underscore, followed by letters, digits, or underscores.

### Program Structure

Every valid program must follow this outer structure:

```lexor
SCRIPT AREA
START SCRIPT
... declarations and statements ...
END SCRIPT
```

Declarations must appear immediately after `START SCRIPT`.

### Comments

Comments start with `%%` and continue until the end of the current line.

### Data Types

- `INT`: integer values
- `FLOAT`: decimal values
- `CHAR`: a single character
- `BOOL`: boolean values `TRUE` or `FALSE`

The runtime also supports string values internally for printing, concatenation, and bracket escapes.

### Literals

- Integer literals: `42`, `0`, `-7` through unary minus
- Float literals: `3.14`, `0.5`, `-2.5` through unary minus
- Character literals: `'a'`
- String literals: `"hello"`
- Boolean literals: `TRUE`, `FALSE`

The lexer also treats quoted `"TRUE"` and `"FALSE"` as boolean literals.

### Escape Brackets

The lexer recognizes bracketed single-character escapes and converts them into string literals. Examples:

- `[#]` becomes `#`
- `[[]` becomes the `[` character
- `[]]` becomes `]`

## Statements

### Declaration

Declare one or more variables of the same type:

```lexor
DECLARE INT x, y = 5, z
DECLARE FLOAT score = 98.5
DECLARE CHAR initial = 'A'
DECLARE BOOL ready = TRUE
```

Rules:

- Multiple variables can be declared in one statement using commas.
- Each variable may optionally be initialized at declaration time.
- Type mismatches are rejected at runtime.

### Assignment

```lexor
x = 10
score = score + 1.5
ready = FALSE
```

Assignments are only valid for variables that were declared earlier.

### Print

```lexor
PRINT: x & " " & y & $ & "done"
```

`PRINT` supports concatenation with `&` and newline insertion with `$`.

### Scan

```lexor
SCAN: x, y, z
```

`SCAN` reads input into declared variables in order.

Supported input formats:

- newline-separated values:

```text
4
5
6
```

- comma-separated values:

```text
4,5,6
```

- comma-separated values with spaces:

```text
4, 5, 6
```

Input parsing is strict:

- `INT` values must be valid integers
- `FLOAT` values must be valid floating-point values
- `CHAR` values must contain exactly one character
- `BOOL` values accept `TRUE`, `FALSE`, `1`, and `0` case-insensitively

### Conditionals

```lexor
IF (x < y)
START IF
PRINT: "x is smaller"
END IF
ELSE IF (x == y)
START IF
PRINT: "equal"
END IF
ELSE
START IF
PRINT: "x is larger"
END IF
```

Conditional blocks use `START IF` and `END IF` for every branch body.

### FOR Loops

```lexor
FOR (i = 0, i < 10, i = i + 1)
START FOR
PRINT: i & " "
END FOR
```

The loop header contains initialization, condition, and update parts separated by commas.

### REPEAT WHEN Loops

```lexor
REPEAT WHEN (running)
START REPEAT
PRINT: "looping"
END REPEAT
```

The loop body is executed while the condition remains true.

## Expression Support

Expressions are evaluated using recursive descent parsing.

### Operator Precedence

From lowest to highest precedence, the current parser evaluates:

1. `OR`
2. `AND`
3. Comparisons: `==`, `<>`, `<`, `>`, `<=`, `>=`
4. `+`, `-`
5. `*`, `/`, `%`
6. `&`
7. Unary `+`, unary `-`, `NOT`
8. Primary values: identifiers, literals, and parenthesized expressions

### Arithmetic

- Integer arithmetic is supported directly.
- Mixed `INT` and `FLOAT` expressions promote to float.
- Division by zero is rejected.
- Modulo is supported for integers only.

### Comparison

- Numeric values can be compared with all comparison operators.
- Strings support `==` and `<>`.
- Booleans support `==` and `<>`.

### Concatenation

The `&` operator converts values to text and concatenates them.

### Unary Operators

- Unary `+` preserves numeric values
- Unary `-` negates numeric values
- `NOT` negates boolean values

## Output Rules

- `INT` prints as a decimal integer
- `FLOAT` prints as a decimal float
- `CHAR` prints as a single character
- `BOOL` prints as `TRUE` or `FALSE`
- `STRING` prints verbatim

## Validation and Errors

The interpreter reports runtime and parse errors for:

- missing or repeated script markers
- undeclared variables
- duplicate declarations
- type mismatches
- malformed literals
- invalid `SCAN` input
- invalid loop or conditional conditions
- division by zero
- unterminated strings or escape sequences

## Tests and Examples

The repository includes several sample programs that can be run directly:

- [test_simple.txt](test_simple.txt): basic script structure and printing
- [test_arith.txt](test_arith.txt): arithmetic expressions and precedence
- [test_unary_ops.txt](test_unary_ops.txt): unary `+` and `-`
- [test_scan.txt](test_scan.txt): `SCAN` with comma-separated and line-separated input
- [test_conditionals.txt](test_conditionals.txt): conditional branching
- [test_logical.txt](test_logical.txt): logical expressions
- [test_repeat](test_repeat): repeat-loop behavior
- [test_nested_repeat](test_nested_repeat): nested repeat loops
- [test_nested_loops](test_nested_loops): nested loop behavior
- [tc1.txt](tc1.txt) and [tc2.txt](tc2.txt): additional sample programs

Example run:

```bash
./lexor test_scan.txt
```

## Implementation Notes

- The lexer handles reserved words, multi-word keywords, literals, comments, and escape brackets.
- `main.cpp` removes newline tokens before parsing, so statement structure is driven by tokens rather than physical line breaks.
- Boolean handling is implemented in the lexer and parser path.
- Logical operator behavior is implemented, but this area is still under active refinement, so complex boolean expressions should be verified against [test_logical.txt](test_logical.txt).

## Development Tip

If you are changing the language surface, update the sample programs alongside the interpreter so the README, tests, and runtime behavior stay aligned.
