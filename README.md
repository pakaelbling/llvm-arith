# Arith Comp
This repo contains a compiler (and, if you wish, executor) for basic arithmetic expressions on 32-bit integers.
It uses:
- [cpp-peglib](https://github.com/yhirose/cpp-peglib) for parsing
- [llvm](https://llvm.org/) for IR generation, object code generation, and execution, and
- [gtest](https://github.com/google/googletest) for unit testing

## Features
This project supports:
- Object code generation
- LLVM IR generation (.ll file)
- Abstract Syntax Tree printing
- File reading

## Language Definition
The Parsing Expression Grammar for the language is as follows:
```
EXPRESSION  <- UNARYEXPR / BINARYEXPR
BINARYEXPR  <- ATOM (BINOP ATOM)* {
                        precedence
                            L - +
                            L / *
                            R ^
                       }
UNARYEXPR   <- ATOM UNOP
ATOM        <- '(' EXPRESSION ')' / LITERAL
BINOP       <- < [-+/*^] >
UNOP        <- "++" / "--"
LITERAL     <- < '-'? [0-9]+ >
%whitespace <- [ \t\r\n]*
```
Here are some valid example expressions:
- 1
  - Evaluates to: 1
- 1 + 2
  - Evaluates to: 3
- 5 / 2
  - Evaluates to: 2
- 1 + 2 * 3
  - Evaluates to: 1 + (2 * 3) = 7
- 2 ^ 3
  - Evaluates to: 8
- 2 ^ 3 ^ 3
  - Evaluates to 2 ^ (3 ^ 3) = 134217728
- 3 + (2++) ^ 2
  - Evaluates to 3 + (3 ^ 2) = 12
## Running It
This project uses `cmake`, so after installing LLVM and whatnot you should be able to build by running:

```shell
cmake --build ./<build_dir> --target ArithComp -j 14
```

To build and run tests, instead run 
```shell
cmake --build ./<build_dir> --target runTest -j 14
./<build_dir>/runTest 
```

Then, after building, run the project with: 

```./<build_dir>/ArithComp [<options>] "<expr>"```

It also supports the following command line flags:
- `-a`: Prints abstract syntax tree to stdout
- `-i`: Prints generated IR to stdout
- `-f <filename>`: sets filename as the source for the compiler
  - Note that if this option is used an expression supplied to the executable
    via command line will be ignored
- `-o`: generates object code at `<build_dir>/objectCodeOut.o`
- `-l`: generates LLVM IR at `<build_dir>/llOut.ll`