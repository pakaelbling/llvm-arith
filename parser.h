#pragma once
#include <peglib.h>
#include <llvm/IR/IRBuilder.h>

namespace parser {
    class Parser {
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
        peg::parser parser = peg::parser();
         Parser(){
            parser.load_grammar(grammar);
            parser.enable_packrat_parsing();
            parser.enable_ast();
        };

         void parseExpr(std::shared_ptr<peg::Ast> &ast, const char* expr) const {
             parser.parse(expr, ast);
         }
    };

} // parser
