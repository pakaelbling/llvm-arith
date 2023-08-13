#pragma once
#include <peglib.h>
#include <llvm/IR/IRBuilder.h>

namespace parser {
    class Parser {
    private:
        const char *grammar = R"(
        EXPRESSION  <- ATOM (OPERATOR ATOM)* {
                         precedence
                           L - +
                           L / *
                       }
        ATOM        <- '(' EXPRESSION ')' / NUMBER
        NUMBER      <- LITERAL
        OPERATOR    <- < [-+/*] >
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
