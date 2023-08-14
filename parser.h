#pragma once
#include "peglib.h"
namespace parser {
    struct Parser {
    private:
        // Single atoms need to be a part of BINARYEXPR to use the precedence and associativity capabilities of peglib
        const char *grammar = R"(
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
        )";
    public:
        peg::parser parser;
        Parser();
        // Parse a string into the supplied AST pointer
        // Modifies ast in place
        void parseExpr(std::shared_ptr<peg::Ast> &ast, const char* expr) const;
    };
} // namespace parser